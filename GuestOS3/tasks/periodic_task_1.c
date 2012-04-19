#include <rtl_conf.h>
#include <time.h>
#include <pthread.h>
#include "systemcall_lib.c"
//Add here any other header files required by task.

int stack_task1[1024];

void *task1(void)
{
    //sys_print("Guest 3: Task1\n");
    //Give period of task in milisec
    //pthread_make_periodic_np (16000);
    pthread_make_periodic_np (16);
    sys_sock_init();
    
    while(1)
    {
    	//Make task wait till next period
        pthread_wait_np ();
        //sys_print("Guest 3: Task1\n");
        //Add additional operations to be performed by task in its one execution here
    
    }
    return 0;
}
