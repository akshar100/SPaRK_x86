/* This is the monitoring task */
/* It executes status commands recvd from Control Room and 
 * sends the status to Control room. The communication is 
 * over Ethernet via communication partition
 */

#include <rtl_conf.h>
#include <time.h>
#include <pthread.h>
#include "systemcall_lib.c"
//Add here any other header files required by task.

int stack_task2[1024];

void *task2(void)
{
    //sys_print("Guest 3: Task2\n");
    //Give period of task in milli sec
    //pthread_make_periodic_np (32000);
    pthread_make_periodic_np (32);

    while(1)
    {
        //Add operations to be performed by task in its one execution
        //here
        //Make task wait till next period
        pthread_wait_np ();
        //sys_print("Guest 3: Task2\n");
    }
    return 0;
}
