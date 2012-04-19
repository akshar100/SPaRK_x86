/*
 * RTLinux v1 compatibility
 *
 *  Copyright (C) 1999 FSM Labs (http://www.fsmlabs.com/)
 *  Written by Michael Barabanov <baraban@fsmlabs.com>
 */

#ifndef __RTL_COMPATIBILITY__
#define __RTL_COMPATIBILITY__

#ifdef CONFIG_RTL_USE_V1_API
/* compatibility */

#include <rtl_conf.h>

#include <asm/rt_time.h>

#define RT_TASK pthread_t

#define RT_TICKS_PER_SEC ((long long) 1193180)
#define RT_TIME_END (HRTIME_INFINITY / 838)

#ifdef __KERNEL__

extern inline int rt_task_suspend (RT_TASK *task)
{
	int v;
	v =  pthread_suspend_np(*task);
	return -v;
}


extern inline int rt_task_wakeup (RT_TASK *task)
{
	int v;
	v =  pthread_wakeup_np(*task);
        return -v;
}


extern inline int rt_task_make_periodic (RT_TASK *task, RTIME start_time, RTIME period)
{
	int v;
	v = pthread_make_periodic_np (*task, HRT_FROM_8254(start_time), HRT_FROM_8254(period));
	return -v;
}

extern inline hrtime_t rt_get_time(void)
{
	return HRT_TO_8254(gethrtime());
}

#define rt_delay(x) rtl_delay (HRT_FROM_8254(x))

extern inline hrtime_t rtl_set_periodic_mode (hrtime_t period)
{
	int mode_success;
	schedule_t *s = LOCAL_SCHED;
	mode_success = rtl_setclockmode (s->clock, RTL_CLOCK_MODE_PERIODIC, HRT_FROM_8254(period));
	if (mode_success == 0) {
		return HRT_TO_8254(clock_gethrtime(s->clock));
	} else {
		return RT_TIME_END;
	}
}

extern inline int rtl_set_oneshot_mode (void)
{
	schedule_t *s = LOCAL_SCHED;
	return rtl_setclockmode(s->clock, RTL_CLOCK_MODE_ONESHOT, 0);
}

#ifdef CONFIG_RTL_FP_SUPPORT
extern inline int rt_task_use_fp (pthread_t *task, int flag) {
	return -pthread_setfp_np (*task, flag);
}
#endif


extern inline int rt_task_delete(pthread_t *thread)
{
	int ret;
	ret = pthread_delete_np(*thread);
	if (ret) {
		return -EINVAL;
	}
	return 0;
}


/* for periodic tasks -- wait until the next period */
extern inline int rt_task_wait (void)
{
	pthread_wait_np();
	return 0;
}


static inline int rt_task_init (RT_TASK *t, void (*fn)(int data), int data, int stack_size, int priority)
{
	int ret;
	void *(*f)(void *);
	pthread_attr_t attr;
	struct rtl_sched_param param;
	pthread_attr_init (&attr);
	f = (void *(*)(void *))fn;
        rtl_sigemptyset(&attr.initial_state);
	attr.cpu = cpu_logical_map (0); /* all compatibility tasks execute on one CPU */
	pthread_attr_setstacksize(&attr, stack_size); /* ok to fail here */
	param.sched_priority = sched_get_priority_max(0) - priority; /* note the priority scheme is inverted to correspond to the POSIX one */
	pthread_attr_setschedparam(&attr, &param);
	ret = pthread_create(t, &attr, f, (void *)data);
	return -ret;
}

#define rtl_init (a,b,c,d,e) { rt_task_init(a,b,c,d,e); }
#define rtl_fpinit (a,b,c,d,e) { rt_task_init(a,b,c,d,e); rtl_task_use_fp (a, 1); }

#endif

#endif /* CONFIG_RTL_USE_V1_API */
#endif
