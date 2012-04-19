/*
 * RTLinux scheduler
 *
 * Written by Michael Barabanov, Victor Yodaiken
 * Copyright (C) Finite State Machine Labs Inc., 1998-1999
 * Released under the terms of the GPL Version 2
 *
 */

#ifndef __RTL__SCHED__
#define __RTL__SCHED__

#include <rtl_conf.h>
#include <arch/ptrace.h>
#include <arch/mprot.h>
#include <arch/bitops.h>

#include <rtl_core.h>
#include <rtl_time.h>
#include <arch/rtl_fpu.h>
#include <rtl_spinlock.h>
#include <signal.h>

#include <arch/page.h>

typedef unsigned long rtl_sigset_t;   // Perhaps This in arch directory

#define rtl_sigaddset(set, sig) set_bit(sig, set)
#define rtl_sigdelset(set, sig) clear_bit(sig, set)
#define rtl_sigismember(set, sig) test_bit(sig, set)
#define rtl_sigemptyset(set) do { *(set) = 0; } while (0)
#define rtl_sigfillset(set) do { *(set) = ~0; } while (0)

#define SA_FOCUS	0x00000008
#define SA_IRQ		0x00000010

struct rtl_siginfo {
	int junk;
};

struct rtl_sigaction {
	union {
		void (*_sa_handler)(int);
		void (*_sa_sigaction)(int, struct rtl_siginfo *, void *);
	} _u;
	int sa_flags;
	unsigned long sa_focus;
        struct rtl_thread_struct *owner;
	rtl_sigset_t sa_mask;
};

#define sa_sigaction _u._sa_sigaction
#define sa_handler _u._sa_handler
#define sigaction rtl_sigaction

#define RTL_SIGIRQMIN 256
#define RTL_SIGLOCALIRQMIN 512
#define RTL_SIGIRQMAX 1024

#ifdef _RTL_POSIX_SIGNALS
// Thread signals comes from 7 to 31.
#define RTL_THREAD_SIGNALS_MASK  0xFFFFFF80
/* Array of sigactions. Thread signals from (7..31) */
extern struct sigaction rtl_sigact[RTL_SIGIRQMAX];
#endif

extern int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
extern int sigprocmask(int how, const rtl_sigset_t *set, rtl_sigset_t *oset);
extern int pthread_sigmask(int how, const rtl_sigset_t *set, rtl_sigset_t *oset);
extern int sigsuspend(const rtl_sigset_t *sigmask);
extern int sigpending(rtl_sigset_t *set);
extern int kill(pid_t pid, int sig);

extern void first_time_return();



struct sched_param {
	int sched_priority;
};

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#if CONFIG_RTL_SCHED_EDF
#define SCHED_EDF_NP            3
#endif

#ifdef CONFIG_RTL_SCHED_EDF
struct rtl_sched_param {
  int sched_priority;
  hrtime_t sched_deadline;
};
#else
#define rtl_sched_param sched_param
#endif //CONFIG_RTL_SCHED_EDF
#define TIMER_ABSTIME 1
#define RTL_MAX_TIMERS 32


typedef struct rtl_timer_struct *rtl_timer_t;

struct rtl_cleanup_struct {
	void (*routine)(void*);
	void *arg;
	struct rtl_cleanup_struct *next;
};

/* threads have no clocks. Clocks belong to schedulers.  We can add scheduling
   policies in which a scheduler juggles multiple clocks, but there is no advantage
   that I can see in allowing a thread to specify its  hardware clock
   */
#define RTL_THREAD_MAGIC 0x79433743
   
struct rtl_thread_struct;
typedef struct rtl_thread_struct *pthread_t;

#include <rtl_posix.h>

struct rtl_thread_struct {
	int *stack;	/* hardcoded */
	int fpu_initialized;
	RTL_FPU_CONTEXT fpu_regs;
	int uses_fp;
	int *kmalloc_stack_bottom;
	struct rtl_sched_param sched_param;
	struct rtl_thread_struct *next;
	int cpu;
	hrtime_t resume_time;
	hrtime_t period;
	hrtime_t timeval;
	void *start;
	void (*abort)(void *);
	void *abortdata;
	int threadflags;
	rtl_sigset_t pending;
	rtl_sigset_t blocked;
	void *user[4];
	int errno_val;
	struct rtl_cleanup_struct *cleanup;
	int magic;
	struct rtl_posix_thread_struct posix_data;
	void *tsd [RTL_PTHREAD_KEYS_MAX];
	int tracer_id;
#if CONFIG_RTL_SCHED_EDF
	hrtime_t current_deadline;
	int policy;
#endif
// #if CONFIG_CONTEXT_MEMORYPROT
	int contextid;
// #endif
	//unsigned long *cr3;
	unsigned long top_of_stack; // esp2

};


     
      

#define RTL_POSIX_DATA(th) (&(th)->posix_data)

#define hrt2ts(hrt) ((const struct timespec *) ({ pthread_self()->timeval = hrt; (&pthread_self()->timeval); }))

#define RTL_PRIO(th) ((th)->sched_param.sched_priority)

enum {RTL_CANCELPENDING, RTL_CANCELTYPE, RTL_THREAD_JOINABLE, RTL_THREAD_FINISHED, RTL_THREAD_TIMERARMED, RTL_THREAD_WAIT_FOR_JOIN, RTL_THREAD_OK_TO_FINISH_JOIN, RTL_THREAD_SIGNAL_INTERRUMPIBLE};

#define RTL_MAX_SIGNAL 31 /* this is max for internal RTLinux signals */
/* these are bit positions */
#define RTL_SIGNAL_NULL 0 /* posix wants signal=0 to simply check */
#define RTL_SIGNAL_WAKEUP 1
#define RTL_SIGNAL_CANCEL 2
#define RTL_SIGNAL_SUSPEND 3
#define RTL_SIGNAL_TIMER 5
#define RTL_SIGNAL_READY 6

#define RTL_SIGNAL_HANDLER_EXECUTION_INPROGRESS 4
#define RTL_SIGUSR1  (RTL_SIGNAL_READY+1)
#define RTL_SIGUSR2  (RTL_SIGUSR1+1)
#define RTL_SIGRTMIN (RTL_SIGUSR2+1)
#define RTL_SIGRTMAX RTL_MAX_SIGNAL

/*TODO How will this work on PPC */
#define RTL_LINUX_MIN_SIGNAL 256  /* signals to Linux start here. global then local */
#define RTL_LINUX_MAX_SIGNAL 1024
#define RTL_LINUX_MIN_LOCAL_SIGNAL  512

extern int rtl_schedule (void);

#define RTL_TIMED_OUT(x) rtl_sigismember((x), RTL_SIGNAL_TIMER)
#define RTL_SIGINTR(x) (0)

typedef struct rtl_thread_struct RTL_THREAD_STRUCT;


#define RTL_MARK_READY(th) rtl_sigaddset(&(th)->pending, RTL_SIGNAL_READY)
#define RTL_MARK_SUSPENDED(th) rtl_sigdelset(&(th)->pending, RTL_SIGNAL_READY)

struct rtl_sched_cpu_struct {
	struct rtl_thread_struct *rtl_current;
	struct rtl_thread_struct *rtl_task_fpu_owner;	/* the task whose FP context is currently in the FPU unit */
	struct rtl_thread_struct *rtl_tasks; /* the queue of  RT tasks */
	struct rtl_thread_struct *rtl_new_tasks;
	rtl_clockid_t clock;
	spinlock_t rtl_tasks_lock;
	int sched_flags;
	int sched_user[4];	/* on x86 sched_user[0] is the Linux TS flag */
}/* __attribute__ ((aligned (64)))*/;

typedef struct rtl_sched_cpu_struct  schedule_t;

extern struct rtl_sched_cpu_struct rtl_scheduler[1];

#define sched_data(cpu) (&rtl_scheduler[0])
#define LOCAL_SCHED (&rtl_scheduler[0])
#define RTL_CURRENT (LOCAL_SCHED->rtl_current)


/* RTL-specific function TODO: write POSIX equivalents for these */


extern int pthread_delete_np (pthread_t thread);

extern int pthread_setfp_np (pthread_t thread, int flag);

extern int pthread_wakeup_np (pthread_t thread);

extern int pthread_suspend_np (pthread_t thread);

/* end RTL-specific */


/* POSIX interface */

#define pthread_cleanup_push(p_routine, p_arg) \
{ \
	rtl_irqstate_t __flags; \
	struct rtl_cleanup_struct __cleanup; \
	__cleanup.routine = (p_routine); \
	__cleanup.arg = (p_arg); \
	rtl_no_interrupts (__flags); \
	__cleanup.next = pthread_self()->cleanup; \
	pthread_self()->cleanup = &__cleanup; \
	rtl_restore_interrupts (__flags);


#define pthread_cleanup_pop(execute) \
	rtl_no_interrupts (__flags); \
	pthread_self()->cleanup = pthread_self()->cleanup->next; \
	if (execute) \
		__cleanup.routine(__cleanup.arg); \
	rtl_restore_interrupts (__flags); \
}


extern inline int sched_get_priority_max(int policy) { return 1000000; }
extern inline int sched_get_priority_min(int policy) { return 0; }

extern inline pthread_t pthread_self(void) {
	return (LOCAL_SCHED)->rtl_current;
}

extern inline int pthread_equal(pthread_t thread1, pthread_t thread2)
{
	return thread1 == thread2;
}

typedef int pthread_key_t;

extern inline int pthread_setspecific(pthread_key_t key, const void *value)
{
	pthread_self()->tsd[key] = (void *) value;
	return 0;
}

extern inline void *pthread_getspecific(pthread_key_t key)
{
	return pthread_self()->tsd[key];
}

#define RTL_PTHREAD_STACK_MIN 8192

#define PTHREAD_CANCEL_ENABLE 0
#define PTHREAD_CANCEL_DISABLE 1
#define PTHREAD_CANCEL_DEFERRED 0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1
#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

extern int pthread_setcancelstate(int state, int *oldstate);
extern int pthread_setcanceltype(int type, int *oldtype);
extern int pthread_cancel (pthread_t thread);
extern void pthread_testcancel(void);

typedef struct STRUCT_PTHREAD_ATTR {
	size_t stack_size;
	void *stack_addr;
	struct rtl_sched_param sched_param;
	int cpu;
	int use_fp;
	rtl_sigset_t initial_state;
	int detachstate;
#if CONFIG_RTL_SCHED_EDF
	int policy;
#endif
} pthread_attr_t;

extern inline int pthread_attr_init(pthread_attr_t *attr)
{
	attr->stack_addr = 0;
	attr->stack_size = 1024*4;
	attr->sched_param.sched_priority = sched_get_priority_min(0);
#if CONFIG_RTL_SCHED_EDF
	attr->sched_param.sched_deadline = HRTIME_INFINITY;
	attr->policy = SCHED_FIFO;
#endif
	attr->cpu = rtl_getcpuid();
        rtl_sigemptyset(&attr->initial_state);
        rtl_sigaddset(&attr->initial_state, RTL_SIGNAL_READY);
	attr->use_fp = 0;
	attr->detachstate = PTHREAD_CREATE_JOINABLE;
	return 0;
}

extern inline int pthread_attr_destroy(pthread_attr_t *attr)
{
	return 0;
}

extern inline int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
	if (stacksize < RTL_PTHREAD_STACK_MIN) {
		return EINVAL;
	}

	attr->stack_size = stacksize;
	return 0;
}

extern inline int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
	*stacksize = attr->stack_size;
	return 0;
}

extern int pthread_attr_setfp_np (pthread_attr_t *attr, int flag);
extern inline int pthread_attr_getfp_np (const pthread_attr_t *attr, int *use_fp)
{
	*use_fp = attr->use_fp;
	return 0;
}

#define pthread_attr_setfp(attr,flag) pthread_attr_setfp_np(attr,flag)
#define pthread_attr_getfp(attr,flag) pthread_attr_getfp_np(attr,flag)

extern inline int pthread_attr_getcpu_np(const pthread_attr_t *attr, int * cpu)
{
	*cpu = attr->cpu;
	return 0;
}

extern int pthread_attr_setcpu_np(pthread_attr_t *attr, int cpu);

#define pthread_attr_getcpu(attr,cpu) pthread_attr_getcpu_np(attr,cpu)
#define pthread_attr_setcpu(attr,cpu) pthread_attr_setcpu_np(attr,cpu)


#ifdef CONFIG_RTL_SCHED_EDF
extern inline int pthread_attr_setdeadline_np(pthread_attr_t *attr, hrtime_t deadline)
{
	attr->sched_param.sched_deadline = deadline;
	return 0;
}

extern inline int pthread_attr_getdeadline_np(const pthread_attr_t *attr, hrtime_t *deadline)
{
	*deadline = attr->sched_param.sched_deadline;
	return 0;
}


extern inline int pthread_setdeadline_np(pthread_t thread, 
					 hrtime_t deadline) {
        thread->sched_param.sched_deadline = deadline;
	return 0;
}
 

extern inline int pthread_getdeadline_np(pthread_t thread, 
					 hrtime_t *deadline) {
        *deadline = thread->sched_param.sched_deadline;
	return 0;
}
 
#endif /* CONFIG_RTL_SCHED_EDF */




// extern int pthread_wait_np(void);

// extern int pthread_make_periodic_np (pthread_t p, hrtime_t start_time, hrtime_t period);


/* this one is deprecated */
extern inline rtl_clockid_t rtl_getschedclock(void)
{
	return LOCAL_SCHED->clock;
}

#define CLOCK_RTL_SCHED (LOCAL_SCHED->clock)
#define CLOCK_REALTIME CLOCK_RTL_SCHED

extern int rtl_setclockmode (rtl_clockid_t clock, int mode, hrtime_t period);


extern int sched_yield(void);


extern inline int pthread_setschedparam(pthread_t thread, int policy,
		    const struct rtl_sched_param *param) {
	thread->sched_param = *param;
	return 0;
}
 
extern inline int pthread_getschedparam(pthread_t thread, int *policy,
		    struct rtl_sched_param *param) {
	*param = thread->sched_param;
#ifdef CONFIG_RTL_SCHED_EDF
	*policy = thread->policy;
#endif
	return 0;
}

extern inline int pthread_attr_setschedparam(pthread_attr_t *attr,
		    const struct rtl_sched_param *param) {
	(attr->sched_param).sched_priority = param->sched_priority;
	return 0;
}

extern inline int pthread_attr_getschedparam(const pthread_attr_t *attr,
		    struct rtl_sched_param *param) {
	*param = attr->sched_param;
	return 0;
}

#ifdef CONFIG_RTL_SCHED_EDF
extern inline int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
        attr->policy = policy;
        return 0;
}

extern inline int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy)
{
        *policy = attr->policy;
        return 0;
}
#endif // CONFIG_RTL_SCHED_EDF
 


extern int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);
extern int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);

struct module;
extern struct module __this_module;
extern int __pthread_create (pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg, struct module *mod);

//#define pthread_create(thread, attr, start, arg) __pthread_create(thread, attr, start, arg, 0)


#define PTHREAD_CANCELED ((void *) -1)

extern void pthread_exit(void *retval);

int pthread_kill(pthread_t , int signo);

extern inline int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	attr->detachstate = detachstate;
	return 0;
}

extern inline int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
	*detachstate = attr->detachstate;
	return 0;
}

//typedef unsigned useconds_t;

//extern int usleep(useconds_t useconds);

extern int clock_nanosleep(rtl_clockid_t clock_id, int flags,
		const struct timespec *rqtp, struct timespec *rmtp);

extern int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);

extern int rtl_reserve_cpus(unsigned cpumask);
extern int rtl_unreserve_cpus(void);

struct task_struct *get_linux_current(void);

#define RTL_SCHED_TIMER_OK 0

#define __rtl_setup_timeout(th,timeout) do { \
	(th)->resume_time = (timeout); \
	set_bit (RTL_THREAD_TIMERARMED, &(th) -> threadflags); \
	clear_bit (RTL_SCHED_TIMER_OK, &sched_data(0)->sched_flags); \
} while (0)


//	clear_bit (RTL_SCHED_TIMER_OK, &sched_data((th)->cpu)->sched_flags);
//
//
static inline hrtime_t __rtl_fix_timeout_for_clock(rtl_clockid_t clock, hrtime_t timeout)
{
	if (clock == CLOCK_RTL_SCHED) {
		return timeout;
	}
/*	if (clock->mode == RTL_CLOCK_MODE_ONESHOT) {
		return timeout - clock->delta;
	} */
	return timeout - clock_gethrtime(clock) + clock_gethrtime(CLOCK_RTL_SCHED);
}

extern int init_sched(void); 

#endif
