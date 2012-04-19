
/*
 */

#include <rtl_conf.h>
#include <arch/memory.h>
#include <arch/context.h>
#include <arch/rtl_tracer.h>
#include <rtl_sched.h>
#include <rtl_ipc.h>
#include <rtl_sync.h>

#include <arch/rtl_switch.h>  // Needed for Context Switch
#include <arch/context.h>
#include <arch/linkage.h>

struct rtl_sched_cpu_struct rtl_scheduler[1];
struct rtl_thread_struct rtl_idle_task;
/***************************************************************/
static inline int cmp_prio(pthread_t A, pthread_t B)
{
#ifdef CONFIG_RTL_SCHED_EDF   
	register int tmp;
	if ( (tmp= (A->sched_param).sched_priority - (B->sched_param).sched_priority) )
		return tmp;
	else 
		return ( (B->current_deadline) > (A->current_deadline) );
#else
	return (A->sched_param).sched_priority - (B->sched_param).sched_priority;
#endif
}

/***************************************************************/
/***************************************************************/
#define do_abort(t) do { clear_bit(RTL_THREAD_TIMERARMED, &t->threadflags); if (t->abort) { t->abort(t->abortdata); }} while (0)

static inline void do_signal(pthread_t t)
{
	if (test_and_clear_bit(RTL_SIGNAL_SUSPEND, &t->pending)) 
	{
		do_abort(t);
		RTL_MARK_SUSPENDED(t);
	}

	if (test_and_clear_bit(RTL_SIGNAL_WAKEUP, &t->pending)) 
	{
		RTL_MARK_READY(t);
		do_abort(t);
	}

	if (test_and_clear_bit(RTL_SIGNAL_TIMER, &t->pending)) 
	{
		RTL_MARK_READY(t);
		do_abort(t);
	}

}
/***************************************************************/

int rtl_schedule(void)
{
	unsigned long interrupt_state;
	struct rtl_thread_struct *new_task = 0;
	struct rtl_thread_struct *t;
	schedule_t *s; 
	hrtime_t now;
	rtl_sigset_t mask;
idle:
	now = gethrtime();

	s = LOCAL_SCHED;
	for (t = s->rtl_tasks; t; t = t->next) {
		if (now >= t->resume_time){
			clear_bit(RTL_THREAD_TIMERARMED, &t->threadflags);
			//RTL_MARK_READY(t);
			rtl_sigaddset (&t->pending, RTL_SIGNAL_TIMER);
			if (t->period != 0){
				// this is to sync the task time with the updation of time bu Spark after deactivation of gos
				while(t->resume_time <= now ){
					t->resume_time += t->period;
				}
			} 
			else {
				t->resume_time = HRTIME_INFINITY;
			}
		}

		if ((t->pending & ~t->blocked) && (!new_task || (cmp_prio(t, new_task)>0) ) ) {
			new_task = t;
		}
	}


	if (s->rtl_current != new_task)
	{
		spark_loadPDE(new_task->contextid);
		// spark_printLong(new_task->top_of_stack);
		spark_setLevel2Stack(new_task->top_of_stack);
		rtl_switch_to(&s->rtl_current,new_task);
#ifdef CONFIG_RTL_FP_SUPPORT
		if (s->rtl_current->uses_fp && \
				s->rtl_task_fpu_owner != s->rtl_current) {
			if (s->rtl_task_fpu_owner) {
				rtl_fpu_save(s,s->rtl_task_fpu_owner);
			};
			rtl_fpu_restore(s,s->rtl_current);
			s->rtl_task_fpu_owner = s->rtl_current;
		};
#endif    
	}

	mask = pthread_self()->pending;

	if (pthread_self()->pending & ~(1 << RTL_SIGNAL_READY)) 
	{
		do_signal(pthread_self());
	}
	if (!rtl_sigismember(&pthread_self()->pending, RTL_SIGNAL_READY)) 
	{
		goto idle;
	}
	return mask;
}
/***************************************************************/

#define CHECK_VALID(thread) do { if ((thread)->magic != RTL_THREAD_MAGIC) return ESRCH; } while (0)

int pthread_kill(pthread_t thread, int signal)
{
	if ((unsigned) signal <= RTL_MAX_SIGNAL) 
	{
		CHECK_VALID(thread);
		if (signal == 0) 
		{
			return 0;
		}
		set_bit(signal, &thread->pending);
		return 0;
	}
	return 0;
}
/***************************************************************/

int pthread_suspend_np (pthread_t thread)
{
	if (thread == pthread_self()) 
	{
		RTL_MARK_SUSPENDED (pthread_self());
		rtl_schedule();
	}
	else 
	{
		pthread_kill(thread,RTL_SIGNAL_SUSPEND);
	}
	return 0;
}
/***************************************************************/

int pthread_wakeup_np (pthread_t thread)
{
	pthread_kill(thread,RTL_SIGNAL_WAKEUP);
	rtl_schedule();
	return 0;
}
/***************************************************************/

void pthread_exit(void *retval)
{
	rtl_irqstate_t flags;
	pthread_t self = pthread_self();
	set_bit (RTL_THREAD_FINISHED, &self->threadflags);
	rtl_schedule();
}
/***************************************************************/

static void add_to_task_list(pthread_t thread)
{
	schedule_t *s = LOCAL_SCHED;
	thread->next = s->rtl_tasks;
	s->rtl_tasks = thread;
}

/***************************************************************/
int pthread_setpriority (pthread_t *thread, int priority)
{
	((*thread)->sched_param).sched_priority = priority; 
	return 0;
}
/***************************************************************/

int pthread_create (pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), int *stack_task)
{
	int *stack_addr;
	int stack_size;
	struct rtl_thread_struct *task;
	long interrupt_state;
	pthread_attr_t default_attr;
	int contextid;
	if (!attr)
	{
		pthread_attr_init(&default_attr);
		attr = &default_attr;
	}

	stack_size = attr->stack_size;

	contextid = get_contextid(start_routine);
	task = (struct rtl_thread_struct *) kGetPage();
	stack_addr = (int *) kGetPage();
	memset((void *) stack_addr,0,stack_size);

	task->kmalloc_stack_bottom = stack_addr;
	*thread = task;
	//task->magic = RTL_THREAD_MAGIC;
	task->pending = attr->initial_state;
	task->blocked = 0;
	task->abort = 0;
	task->cpu = attr->cpu;
	task->cleanup = 0;
	task->resume_time = HRTIME_INFINITY;
#ifdef CONFIG_RTL_SCHED_EDF
	task->policy = attr->policy;
	task->current_deadline = 0LL;
#endif
	task->period = 0;
	(task->sched_param).sched_priority = (attr->sched_param).sched_priority;
	task->stack = (int *) (((int) stack_addr) + (stack_size-sizeof(int)));
	task->top_of_stack = (unsigned long)task->stack;

	//spark_printLong(task->top_of_stack);

	task->fpu_initialized = 0;
	task->uses_fp = attr->use_fp;
	task->start = (void *) start_routine;
	task->contextid = contextid;
	((RT_TASK_IPC *) task)->sem_at = NULL;
	//((RT_TASK_IPC *) task)->magic = RT_TASK_IPC_MAGIC;
	task->user[IPC_DATA_INDEX] = task;
	//rtl_init_stack(task,start_routine,arg,rtl_startup);
	rtl_init_stack(task,stack_task + 1024 ,start_routine);
	add_to_task_list(task);
	rtl_sigaddset(&task->pending,RTL_SIGNAL_READY);

	return 0;
}
/***************************************************************/

int init_sched(void)
{
	schedule_t *s = LOCAL_SCHED;
	unsigned int cpu_id = rtl_getcpuid();
	rtl_irqstate_t interrupt_state;

	memset(rtl_scheduler,0,sizeof(struct rtl_sched_cpu_struct));
	rtl_spin_lock_init (&s->rtl_tasks_lock);

	rtl_idle_task.next = 0;
	rtl_idle_task.sched_param.sched_priority = sched_get_priority_min(0);
	rtl_idle_task.pending = 0;
	rtl_idle_task.blocked = 0;
	rtl_idle_task.uses_fp = 0;
	rtl_idle_task.top_of_stack = 0x100; // dummy value initialized randomly to 0x100
	rtl_idle_task.fpu_initialized = 0;
#ifdef CONFIG_RTL_SCHED_EDF
	rtl_idle_task.sched_param.sched_deadline = HRTIME_INFINITY;
	rtl_idle_task.current_deadline = HRTIME_INFINITY;
#endif

	rtl_sigemptyset(&rtl_idle_task.pending);
	rtl_sigaddset (&rtl_idle_task.pending, RTL_SIGNAL_READY);
	s->rtl_current = &rtl_idle_task;
	s->rtl_tasks = &rtl_idle_task;
	s->sched_flags = 0;
#if CONFIG_RTL_FP_SUPPORT
	s->rtl_task_fpu_owner = 0;
#endif  

	return 0;
}
/***************************************************************/
/* asmlinkage int pthread_make_periodic_np_k (pthread_t p, hrtime_t start_time, unsigned long period)
 * We dont require pthread_t....it will always be the currently invoked task
 * We dont require start_time....it needs to be 0 for all tasks 
 */
#define MULCON 1000000
asmlinkage int pthread_make_periodic_np_k (unsigned long period)
{
	pthread_t p = (LOCAL_SCHED)->rtl_current;

	if (period < 0) 
	{
		return EINVAL;
	}
	p->period = ((hrtime_t)(period));
	p->period = (hrtime_t)((p->period) * MULCON);
#ifdef CONFIG_RTL_SCHED_EDF
	if (p->policy == SCHED_EDF_NP)
		p->current_deadline = start_time + p->sched_param.sched_deadline - p->period;
#endif
	// __rtl_setup_timeout(p, start_time);
	__rtl_setup_timeout(p, 0);
	return 0;
}
/***************************************************************/
asmlinkage int pthread_wait_np_k(void)
{
	rtl_irqstate_t interrupt_state;
	pthread_t self = pthread_self();
#ifdef CONFIG_RTL_SCHED_EDF
	if (self->policy == SCHED_EDF_NP)
		self->current_deadline += self->period;
#endif
	RTL_MARK_SUSPENDED (self);
	__rtl_setup_timeout (self, self->resume_time);
	rtl_schedule();

	return 0;
}
/***************************************************************/
#ifdef CONFIG_RTL_FP_SUPPORT
int pthread_attr_setfp_np (pthread_attr_t *attr, int flag) {
	attr->use_fp = flag;
	return 0;
}

int pthread_setfp_np (pthread_t thread, int flag)
{
	schedule_t *sched = LOCAL_SCHED;
	rtl_irqstate_t flags;

	if (thread -> uses_fp != flag) {
		thread -> uses_fp = flag;
		if (flag) {
			if (thread == pthread_self()) {
				if (sched->rtl_task_fpu_owner) {
					rtl_fpu_save (sched, sched->rtl_task_fpu_owner);
				}
				rtl_fpu_restore (sched, thread);
				sched->rtl_task_fpu_owner = thread;
			}
		}
	}
	return 0;
}
#endif
/***************************************************************/

