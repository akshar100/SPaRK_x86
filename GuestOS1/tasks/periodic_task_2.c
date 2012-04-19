#include <rtl_conf.h>
#include <time.h>
#include <pthread.h>
#include <itc.h>
#include <uip/demo.h>
#include "systemcall_lib.c"

#define  APP_DATA_SIZE 200
int stack_task2[1024];

//Add here any other header files required by task.
void *task2(void)
{
    int i;
    unsigned long callid;
    unsigned long gos_id;
    unsigned long task_switch_time;
    unsigned long parameter = 0x0;
    unsigned long port_gosid[2];
    unsigned long gosid_ip_port_sockid[4];
    unsigned long gosid_ip_size_sockid[4];
    unsigned long tcp_send_buffer[APP_DATA_SIZE];
    unsigned long reading[2];

    sys_print("GuestOS1:Task2\n");
    sys_registerMQ(SOCKET_CALL_CHANNEL1, CALL_CHANNEL1_SIZE);
    sys_registerMQ(SOCKET_CALL_REPLY_CHANNEL1, CALL_REPLY_CHANNEL1_SIZE);
    sys_registerMQ(SOCKET_TCP_RECV_CHANNEL1, TCP_RECV_CHANNEL1_SIZE);
    sys_registerMQ(SOCKET_CALL_CHANNEL2, CALL_CHANNEL2_SIZE);
    sys_registerMQ(SOCKET_CALL_REPLY_CHANNEL2, CALL_REPLY_CHANNEL2_SIZE);
    sys_registerMQ(SOCKET_TCP_RECV_CHANNEL2, TCP_RECV_CHANNEL2_SIZE);

    //Give period of task in milli sec
    //pthread_make_periodic_np (16000);
    pthread_make_periodic_np (16);
    while(1)
    {
        //Make task wait till next period
        pthread_wait_np ();
        //sys_print("Guest 1: Task2\n");

        callid = 0;
        sys_receiveMQ(SOCKET_CALL_CHANNEL1, &callid, sizeof(callid));
        switch(callid)
        {
            case 0x1111:
                sys_receiveMQ(SOCKET_CALL_CHANNEL1, port_gosid, sizeof(port_gosid));
                sys_uip_listen(port_gosid[1], port_gosid[0]);
                break;
            case 0x2222:
                sys_receiveMQ(SOCKET_CALL_CHANNEL1, gosid_ip_port_sockid, sizeof(gosid_ip_port_sockid));
                sys_uip_connect(gosid_ip_port_sockid[1], gosid_ip_port_sockid[2], gosid_ip_port_sockid[0], gosid_ip_port_sockid[3]);
                break;
            case 0x3333:
                sys_receiveMQ(SOCKET_CALL_CHANNEL1, gosid_ip_size_sockid, sizeof(gosid_ip_size_sockid));
                if(gosid_ip_size_sockid[2] > 0)
                {
                    sys_receiveMQ(SOCKET_CALL_CHANNEL1, tcp_send_buffer, gosid_ip_size_sockid[2]);
                    sys_uip_tcp_send(gosid_ip_size_sockid[3], gosid_ip_size_sockid[0], tcp_send_buffer, gosid_ip_size_sockid[2]);
                }
                break;
            case 0:
                break;
            default :
                sys_print("\tUnidentified socket call by partition 2\n");
                sys_printLong(callid);
                break;
        }
        callid = 0;
        sys_receiveMQ(SOCKET_CALL_CHANNEL2, &callid, sizeof(callid));
        switch(callid)
        {
            case 0x1111:
                sys_receiveMQ(SOCKET_CALL_CHANNEL2, port_gosid, sizeof(port_gosid));
                sys_uip_listen(port_gosid[1], port_gosid[0]);
                break;
            case 0x2222:
                sys_receiveMQ(SOCKET_CALL_CHANNEL2, gosid_ip_port_sockid, sizeof(gosid_ip_port_sockid));
                sys_uip_connect(gosid_ip_port_sockid[1], gosid_ip_port_sockid[2], gosid_ip_port_sockid[0], gosid_ip_port_sockid[3]);
                break;
            case 0:
                break;
            default :
                sys_print("\tUnidentified socket call by partition 3\n");
                sys_printLong(callid);
                break;
        }
    }
    return 0;
}
