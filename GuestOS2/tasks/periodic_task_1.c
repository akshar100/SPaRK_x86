#include <rtl_conf.h>
#include <time.h>
#include <pthread.h>
#include "systemcall_lib.c"
//Add here any other header files required by task.

#define DATA_SIZE  100

int stack_task1[1024];

void *task1(void)
{
    unsigned char gos2_tcp_in_buffer[DATA_SIZE];
    unsigned long port1 = 5678;
    unsigned long sockid1 = 0;
    unsigned long size;
    unsigned long ret; 
    unsigned long count_packet1 = 0; 
    //sys_print("Guest 2: Task1\n");
    //Give period of task in milisec
    //pthread_make_periodic_np (16000);
    pthread_make_periodic_np (16);
    sys_sock_init();
    sockid1 = sys_sock_listen(port1);
    while(1)
    {
    	//Make task wait till next period
        pthread_wait_np ();
    	//sys_print("Guest 2: Task1\n");

    	size = DATA_SIZE;
        ret = sys_sock_tcp_recv(sockid1, gos2_tcp_in_buffer, size);
        if(ret > 0)
        {
            if(ret == 0xEFEF)
            { 
                 sys_print("--NOT Listening on PORT 1 : 5678\n");
            }    
            else
            {
                 //sys_print("Task 1 of GOS 2 recvd tcp data on PORT 1: YES\n");
                 sys_print("\t\t\t\t\tGOS 1 : PORT : 5678\n");
                 sys_print("\t\t\t\t\tGOS 1 : RECV : Length = ");
                 sys_printLong(ret);
                 count_packet1++;
                 sys_print("\t\t\t\t\tGOS 1 : RECV : Data   = ");
                 sys_printLong(*(unsigned long *)gos2_tcp_in_buffer);
                 (*(unsigned long *)gos2_tcp_in_buffer) += 1;
                 sys_print("\t\t\t\t\tGOS 1 : SEND : Data   = ");
                 sys_printLong(*(unsigned long *)gos2_tcp_in_buffer);
                 ret = sys_sock_tcp_send(sockid1, gos2_tcp_in_buffer, ret);
            }
        }
        else
        {
            //sys_print("Task 1 of GOS 2 : TCP data on PORT : 5678 : NO\n");
        }
        //Add additional operations to be performed by task in its one execution here
    }
    return 0;
}
