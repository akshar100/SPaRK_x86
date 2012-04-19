#include <rtl_conf.h>
#include <rtl_sched.h>
#include <unistd.h>
//Add here any other header files if required.

#define NUMBER_OF_TASKS  2 //n refers to number of tasks

//Define Task control blocks for all the tasks
pthread_t tcb1;
pthread_t tcb2;

//Task names
extern void *task1(void);
extern void *task2(void);

//Stacks for tasks
extern int stack_task1[1024];
extern int stack_task2[1024];

void init_tasks(void)
{
	pthread_create(&tcb1,NULL,task1,stack_task1);
	pthread_create(&tcb2,NULL,task2,stack_task2);
}
