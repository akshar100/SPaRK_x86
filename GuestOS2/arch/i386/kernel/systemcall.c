#include <rtl_conf.h>
#include <rtl_sched.h>
#include <arch/linkage.h>
#include <rtl_time.h>

#include <arch/circqueue.h>

int kbd_data_k;

/*****************************************************************************/
asmlinkage void sys_print_k(char *str)
{
	spark_print(str);
}
/*****************************************************************************/
asmlinkage void sys_printLong_k(unsigned long number)
{
        spark_printLong(number);
}
/*****************************************************************************/
// right now not using temp, may be used in later stages
// system call that call hypercall to register the interrupt 
// and also passes the address of a kernel variable
// right now we are using a single variable but it can also be a pointer
// of a structure a fifo or queue or just anything
asmlinkage void sys_reg_intrpt_k(int intr, unsigned int  temp)
{
	spark_register_interrupt(intr, &kbd_data_k);
}

/*****************************************************************************/
// sytem call that will copy the keyboard data from GOS kernel area to usre area
asmlinkage void  sys_get_kbd_data_k(int  *temp)
{
	*temp = kbd_data_k; 
}

/*****************************************************************************/
//PCI_DRIVER
asmlinkage long sys_pci1710_digital_in_k(char *buffer, unsigned long size)
{
	return spark_pci1710_digital_in(buffer,size);
}

/*****************************************************************************/
asmlinkage long sys_pci1710_digital_out_k(char *buffer, unsigned long size)
{
	return spark_pci1710_digital_out(buffer,size);
}

/*****************************************************************************/
asmlinkage long sys_pci1710_analog_in_k(char *buffer, unsigned long size)
{
	return spark_pci1710_analog_in(buffer,size);
}

/*****************************************************************************/
asmlinkage long sys_pci1710_analog_out_k(char *buffer, unsigned long size)
{
	return spark_pci1710_analog_out(buffer,size);
}

/*****************************************************************************/
// System Call to register the messgage queue for IPC
asmlinkage long  sys_registerMQ_k(unsigned long ulChannelID, unsigned long ulMemSize)
{
	return spark_registerMQ(ulChannelID, ulMemSize);
}

/*****************************************************************************/
// System Call to send the data to messgage queue IPC
asmlinkage long  sys_sendMQ_k(unsigned long ulChannelID, char *msg, unsigned long ulDataSize)
{
	return spark_send(ulChannelID, msg, ulDataSize);
}

/*****************************************************************************/
// System Call to recieve data from message queue queue for IPC
asmlinkage long sys_receiveMQ_k(unsigned long ulChannelID, char *msg, unsigned long ulDataSize)
{
	return spark_receive(ulChannelID, msg, ulDataSize);
}

/*****************************************************************************/
// System call to get the current time
asmlinkage long sys_getTime_k(unsigned long *ptrTime)
{
	hrtime_t t;
	t = gethrtime();
	*ptrTime = (unsigned long)t;
	// spark_printLong((unsigned long)*ptrTime);
	return 0;
}
/*****************************************************************************/
// System call to get task information
struct task_information
{
    hrtime_t period;
};

asmlinkage long sys_getTaskInfo_k(unsigned long ptrTaskInfo)
{
    int i = 0;

    struct rtl_thread_struct *current_task = (LOCAL_SCHED->rtl_tasks);
    while(current_task != NULL)
    {
        /* Period is specified in ns. Report is ms */
        (((struct task_information *)ptrTaskInfo) + i)->period = current_task->period;
        i++;
        current_task = current_task->next;
    }

    return 0;
}
/*****************************************************************************/
#define SOCKET_CALL_CHANNEL1                0x01020201
#define SOCKET_TCP_RECV_CHANNEL1            0x02010102
#define SOCKET_TCP_SEND_CHANNEL1            0x52515152
#define CALL_CHANNEL1_SIZE                  512
#define TCP_RECV_CHANNEL1_SIZE              512 
#define TCP_SEND_CHANNEL1_SIZE              512 

#define ERROR_NO_PORT_LISTENING             0xEFEF

unsigned int port_index = 0;

// System call to initialize the socket interface
asmlinkage long sys_sock_init_k()
{
    //spark_print("sys_sock_init\n\n");
    spark_registerMQ(SOCKET_CALL_CHANNEL1, CALL_CHANNEL1_SIZE);
    spark_registerMQ(SOCKET_TCP_RECV_CHANNEL1, TCP_RECV_CHANNEL1_SIZE);
    return 0;
}
/*****************************************************************************/
// System call to create a server 

struct port_information
{
    unsigned long sock_id;
    unsigned short queued_packets;
    struct circ_queue_monitor port_buffer_mon;
    unsigned int length[APP_QUEUE_SIZE];
};
struct port_information active_ports[MAX_OPEN_PORTS];

asmlinkage long sys_sock_listen_k(unsigned long port)
{
    unsigned long ipc_listen_call[3];
    //spark_print("sys_sock_listen\n\n");
    if((port & 0xFFFF0000) || (port == 0) || port_index == MAX_OPEN_PORTS)
        return 0;
    else
    {
        ipc_listen_call[0] = 0x1111; //call id for listen
        ipc_listen_call[1] = 0x1;    //GOD id for GOS2 
        ipc_listen_call[2] = port;   //port on which to listen
        active_ports[port_index].sock_id = port;
        //spark_printLong(active_ports[port_index].sock_id);
        init_cq_monitor(&(active_ports[port_index].port_buffer_mon));
        port_index++;
        spark_send(SOCKET_CALL_CHANNEL1, ipc_listen_call, sizeof(ipc_listen_call));
        return port;
    }
}
/*****************************************************************************/
// System call to recv tcp data
unsigned char tcp_recv_data[APP_DATA_SIZE];
asmlinkage long sys_sock_tcp_recv_k(unsigned long sockid, unsigned char *buffer, unsigned long size)
{
    int message_size = 0;
    int index;
    int count;
    unsigned long dport = 0x0 ;
    unsigned long dsockid = 0xFFFF0000 ;
    unsigned long dlength;
    unsigned char *data_to_process;
    int ret;
    unsigned long buffer_bytes = 0;
    unsigned long flag_copied_buffer = 0;

    //spark_print("\nsys_sock_tcp_recv_k\n");
    for(index=0; index<MAX_OPEN_PORTS; index++)
       if(active_ports[index].sock_id == sockid) 
          break; 
    /* No one listening on this port. Nothing to return */
    if(index==MAX_OPEN_PORTS)
        return ERROR_NO_PORT_LISTENING;

    /* else : see what u can get from comm partition 
            : see what u have enqueued in the buffer for this port 
       index now holds the pointer to the port-information of the port
    */
    //spark_print("Query SockID\t");
    //spark_printLong(sockid);
    //spark_print("Listening Port\t");
    //spark_printLong(active_ports[index].sock_id);
    /* IPC message comes with 16 byte header:callid, port, length, sockid */
    message_size = sys_receiveMQ_k(SOCKET_TCP_RECV_CHANNEL1, tcp_recv_data, size + 16);
    //spark_print("\n message_size");
    //spark_printLong(message_size);
    if(message_size != 0)
    {
        if(message_size != (size + 16))
            message_size = sys_receiveMQ_k(SOCKET_TCP_RECV_CHANNEL1, tcp_recv_data, message_size);
        data_to_process = tcp_recv_data;
        while(message_size != 0)
        {
            //spark_print("\nIPC message_size");
            //spark_printLong(message_size);
            dport = *(unsigned long *)(data_to_process + 4);
            dsockid = *(unsigned long *)(data_to_process + 12);
            //spark_print("Destination Port\t");
            //spark_printLong(dport);
            //spark_print("Destination Socket id\t");
            //spark_printLong(dsockid);
            //spark_print("Queried Port_Sockid\t");
            //spark_printLong(sockid);

            dlength = *(unsigned long *)(data_to_process + 8);
            //spark_print("Destination Length\t");
            //spark_printLong(dlength);

            if((dport == sockid) || (dsockid == sockid))
            {
               ////spark_print("TCP DATA FOR GIVEN PORT : YES : NEW\n");
               memcpy(buffer, &(data_to_process[16]), dlength);
#if 0
               spark_print("\nData\n");
               for(count=0; count<dlength; count++)
                   spark_printLong(buffer[count]);
#endif
               flag_copied_buffer = 1;
               buffer_bytes = dlength;
            }
            else
            {
                /* Store in buffer for appropriate buffer */
                ////spark_print("TCP DATA FOR GIVEN PORT : NO\n");
                ////spark_print("TCP DATA FOR SOME OTHER PORT : YES\n");
                /* If u have some old packets in buffer give them to app layer */
                if(!isempty(&(active_ports[index].port_buffer_mon)))
                { 
                    ////spark_print("OLD PACKET FOUND FOR THIS PORT : YES\n");
                    ret = removeq(&(active_ports[index].port_buffer_mon), buffer);
                    size = active_ports[index].length[ret];
                    active_ports[index].length[ret] = 0;
                    flag_copied_buffer = 1;
#if 0
                    spark_print("\nData\n");
                    for(count=0; count<size; count++)
                        spark_printLong(buffer[count]);
#endif
                }
                else  
                { 
                    ////spark_print("OLD PACKET FOUND FOR THIS PORT : NO\n");
                    size = 0;
                }
                /* enqueue the packet in the buffer of target port */
                for(index=0; index<MAX_OPEN_PORTS; index++)
                    if((active_ports[index].sock_id == dport) ||
                        (active_ports[index].sock_id == dsockid))
                        break;
                //spark_print("Enqueuing packet for \t");
                //spark_printLong(active_ports[index].sock_id);
                /* No one listening on this port. Discard the packet */
                if(index==MAX_OPEN_PORTS)
                {
                    //spark_print("Packet for client port may be\t");
                    //spark_printLong(dport);
                    spark_print("\n*********Discarding as of now********\n");
                    buffer_bytes = flag_copied_buffer?buffer_bytes:0;
                }
                else
                {
                    ret = insertq(&(active_ports[index].port_buffer_mon), 
                                  &(data_to_process[16]), dlength); 
                    //spark_print("index =\t");
                    //spark_printLong(index);
                    //spark_print("ret =\t");
                    //spark_printLong(ret);
                    active_ports[index].length[ret] = dlength;
                    //spark_print("active_ports[index].length[ret] =\t");
                    //spark_printLong(active_ports[index].length[ret]);
                    buffer_bytes = flag_copied_buffer?size:0;
                }
            }
            data_to_process += (16 + dlength);
            message_size -= (16 + dlength);
        }
    }
    else
    {
        //spark_print("index =\t");
        //spark_printLong(index);
        /* If u have some old packets in buffer give them to app layer */
        if(!isempty(&(active_ports[index].port_buffer_mon)))
        {
            ////spark_print("TCP DATA FOR GIVEN PORT : YES : OLD\n");
            ret = removeq(&(active_ports[index].port_buffer_mon), buffer);
            size = active_ports[index].length[ret];
#if 0
            spark_print("\nData\n");
            for(count=0; count<size; count++)
                spark_printLong(buffer[count]);
#endif
            //spark_print("ret =\t");
            //spark_printLong(ret);
            //spark_print("active_ports[index].length[ret] =\t");
            //spark_printLong(active_ports[index].length[ret]);
            active_ports[index].length[ret] = 0;
            return size;
        }
        else
        {
            ////spark_print("TCP DATA FOR GIVEN PORT : NO\n");
            return 0;
        }
    }
    return buffer_bytes;
}

/*****************************************************************************/
// System call to send tcp data
unsigned char tcp_send_data[APP_DATA_SIZE + 20];
asmlinkage long sys_sock_tcp_send_k(unsigned long sockid, unsigned char *buffer, unsigned long size)
{
    int index;

    //spark_print("\nsys_sock_tcp_send_k\n");
    for(index=0; index<MAX_OPEN_PORTS; index++)
    {
       //spark_print("\nactive_ports[index].sock_id = \t");
       //spark_printLong(active_ports[index].sock_id);
       //spark_print("\nsock_id = \t");
       //spark_printLong(sockid);
       if(active_ports[index].sock_id == sockid) 
          break; 
    }
    /* No one listening on this port. Nothing to return */
    if(index==MAX_OPEN_PORTS)
        return ERROR_NO_PORT_LISTENING;
    *(((unsigned long *)tcp_send_data) + 0) = 0x3333; //call id for tcp_send 
    *(((unsigned long *)tcp_send_data) + 1) = 0x1;    //GOD id for GOS2 
    *(((unsigned long *)tcp_send_data) + 2) = 0x0;    //ip addr to connect to
    *(((unsigned long *)tcp_send_data) + 3) = size;   //size of buffer
    *(((unsigned long *)tcp_send_data) + 4) = sockid; //socket id
    for(index=0; index<size; index++)
       (tcp_send_data[20+index]) = *(buffer + index);
    //spark_print("\nIPC to COmm PArt\n");
    spark_send(SOCKET_CALL_CHANNEL1, tcp_send_data, 20+size);
 
    return size;
}
/*****************************************************************************/
unsigned long spark_sockid = 0x00010000;
// System call to connect to a listening endpoint 
asmlinkage long sys_sock_connect_k(unsigned long pip_addr, unsigned long port)
{
    int i;
    int ret = 0;
    //spark_print("\nsys_sock_connect_k\n");
    //spark_print("pip_addr");
    //spark_printLong(*(unsigned long *)pip_addr); 
    //spark_print("port");
    //spark_printLong(port); 
    unsigned long ipc_connect_call[5];
    //spark_print("sys_sock_listen\n\n");
    if((port & 0xFFFF0000) || (port == 0) || (port_index == MAX_OPEN_PORTS))
        return 0;
    if(*((unsigned long *)pip_addr) == 0x00000000)
        return 0;
    else
    {
        ipc_connect_call[0] = 0x2222; //call id for connect
        ipc_connect_call[1] = 0x1;    //GOD id for GOS2 
        ipc_connect_call[2] = *((unsigned long *)pip_addr);   //ip addr to connect to
        ipc_connect_call[3] = port;   //port to connect to
        ipc_connect_call[4] = spark_sockid;   //socket id to pair up with the lport
        active_ports[port_index].sock_id = spark_sockid;
        ret = spark_sockid;
        //spark_printLong(active_ports[port_index].sock_id);
        init_cq_monitor(&(active_ports[port_index].port_buffer_mon));
        port_index++;
        spark_sockid++;
        spark_send(SOCKET_CALL_CHANNEL1, ipc_connect_call, sizeof(ipc_connect_call));
        return (ret);
    }
}
/*****************************************************************************/
