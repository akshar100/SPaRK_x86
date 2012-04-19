#include <rtl_conf.h>
#include <time.h>
#include <pthread.h>
#include "systemcall_lib.c"
//Add here any other header files required by task.

int stack_task2[1024];

void *task2(void)
{
    //sys_print("Guest 2: Task2\n");
    //Give period of task in milli sec
    //pthread_make_periodic_np (16000);
    pthread_make_periodic_np (16);
    while(1)
    {
    	//Make task wait till next period
        pthread_wait_np ();
    	//sys_print("Guest 2: Task2\n");
        //Add additional operations to be performed by task in its one execution here
    }
    return 0;
}
