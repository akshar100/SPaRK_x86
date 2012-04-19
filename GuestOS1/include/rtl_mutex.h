/*
 * RTLinux mutex implementation
 *
 * Written by Michael Barabanov
 * Copyright (C) Finite State Machine Labs Inc., 1999
 * Released under the terms of the GPL Version 2
 *
 */

#ifndef __RTL_MUTEX_H__
#define __RTL_MUTEX_H__


#include <rtl_conf.h>
#include <errno.h>
#include <rtl_spinlock.h>
#include <rtl_wait.h>

enum { PTHREAD_MUTEX_NORMAL, PTHREAD_MUTEX_RECURSIVE, PTHREAD_MUTEX_ERRORCHECK,
	PTHREAD_MUTEX_SPINLOCK_NP, PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL };

typedef struct {
	int type;
	int pshared;
	int protocol;
	int prioceiling;
#ifdef CONFIG_RTL_SRP
	struct rtl_sched_param preemptceiling;
#endif
} pthread_mutexattr_t;

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

typedef struct {
	rtl_irqstate_t flags;
	int busy;
	int valid;
	spinlock_t lock;
	int type;
	rtl_wait_t wait;
	int protocol;
	int prioceiling;
	int oldprio;
#ifdef CONFIG_RTL_SRP
	struct rtl_sched_param preemptceiling;
#endif
} pthread_mutex_t;


#define PTHREAD_MUTEX_INITIALIZER { 0,0,1, SPIN_LOCK_UNLOCKED, PTHREAD_MUTEX_DEFAULT, RTL_WAIT_INITIALIZER, PTHREAD_PRIO_NONE }

enum {PTHREAD_PRIO_NONE, PTHREAD_PRIO_PROTECT, PTHREAD_PRIO_SRP};


static inline int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	if (type != PTHREAD_MUTEX_NORMAL
			&& attr->type != PTHREAD_MUTEX_SPINLOCK_NP) {
		return EINVAL;
	}
	attr->type = type;
	return 0;
}



static inline int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
		    int *pshared)
{
	*pshared = attr->pshared;
	return 0;
}

/*TODO something sensible with this. We don't worry about
  shared and not shared yet.
  */
extern inline int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr,
		    int pshared)
{
	if (pshared != PTHREAD_PROCESS_SHARED
			&& pshared != PTHREAD_PROCESS_PRIVATE) {
		return EINVAL;
	}
	attr->pshared = pshared;
	return 0;
}


extern int pthread_mutexattr_init(pthread_mutexattr_t *attr);

extern inline int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	return 0;
}

extern int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);

extern inline int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
	*type = attr->type;
	return 0;
}


extern int pthread_mutex_init(pthread_mutex_t *mutex,
		    const pthread_mutexattr_t *attr);
extern inline int pthread_mutex_destroy(pthread_mutex_t *mutex);

extern int pthread_mutex_lock(pthread_mutex_t *mutex);
extern int pthread_mutex_trylock(pthread_mutex_t *mutex);
extern int pthread_mutex_unlock(pthread_mutex_t *mutex);

/* not supported for spinlock mutexes */
extern int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime);

static inline int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
#if ! (defined(_RTL_POSIX_THREAD_PRIO_PROTECT) || defined(CONFIG_RTL_SRP))
		return ENOTSUP;
#endif
	if (protocol != PTHREAD_PRIO_PROTECT && protocol != PTHREAD_PRIO_NONE) {
		return ENOTSUP;
	}
	attr->protocol = protocol;
	return 0;
}

static inline int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
	*protocol = attr->protocol;
	return 0;
}

static inline int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
	attr->prioceiling = prioceiling;
	return 0;
}

static inline int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling)
{
	*prioceiling = attr->prioceiling;
	return 0;
}


extern int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling);

static inline int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling)
{
	*prioceiling = mutex->prioceiling;
	return 0;
}



/* 
 * Stack Resource Protocol constants and functions
 */

#ifdef CONFIG_RTL_SRP


typedef struct {
        struct rtl_sched_param preempt_level;
        pthread_t owner;
} srp_stack_t;

static inline int cmp_preempt_level(struct rtl_sched_param *A, struct rtl_sched_param *B) 
{
        register int tmp;
	hrtime_t diff;
	if (!B) 
	        return 1;
        if ( (tmp = (A->sched_priority - B->sched_priority)) )
                return tmp;
        diff = B->sched_deadline - A->sched_deadline;
	return (diff>0 ? 1: (diff==0? 0: -1));
}

extern pthread_t srp_ceiling_owner(void);

extern struct rtl_sched_param *srp_current_sysceil(void);


/* External API functions */

static inline int pthread_mutexattr_setpreemptceiling_np(pthread_mutexattr_t *attr, struct rtl_sched_param *preemptceiling)
{
	attr->preemptceiling = *preemptceiling;
	return 0;
}


static inline int pthread_mutexattr_getpreemptceiling_np(const pthread_mutexattr_t *attr, struct rtl_sched_param *preemptceiling)
{
	*preemptceiling = attr->preemptceiling;
	return 0;
}

static inline int pthread_mutex_getpreemptceiling_np(const pthread_mutex_t *mutex, struct rtl_sched_param *old_ceiling)
{
	*old_ceiling = mutex->preemptceiling;
	return 0;
}

extern int pthread_mutex_setpreemptceiling_np(pthread_mutex_t *mutex, 
					      struct rtl_sched_param *preemptceiling,
					      struct rtl_sched_param *oldpreemptceiling );

extern int pthread_mutex_register_np(pthread_t thread, pthread_mutex_t *mutex);

#endif /* CONFIG_RTL_SRP */



#endif // __RTL_MUTEX_H__
