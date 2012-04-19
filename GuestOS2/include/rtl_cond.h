/*
 * RTLinux condition vars implementation
 *
 * Written by Michael Barabanov
 * Copyright (C) Finite State Machine Labs Inc., 1999
 * Released under the terms of the GPL Version 2
 *
 */


#ifndef __RTL_COND_H__
#define __RTL_COND_H__

#include <rtl_conf.h>
#include <errno.h>
#include <rtl_spinlock.h>
#include <rtl_wait.h>
#include <rtl_mutex.h>


typedef struct {
  int pshared;
} pthread_condattr_t;

extern inline int pthread_condattr_getpshared(const pthread_condattr_t *attr,
		    int *pshared)
{
  *pshared = attr->pshared;
  return 0;
}

extern inline int pthread_condattr_setpshared(pthread_condattr_t *attr,
		    int pshared) 
{
  if (pshared != PTHREAD_PROCESS_SHARED
      && pshared != PTHREAD_PROCESS_PRIVATE) 
  {
    return EINVAL;
  }
  attr->pshared = pshared;
  return 0;
}


extern inline int pthread_condattr_init(pthread_condattr_t *attr)
{
  attr->pshared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}

extern inline int pthread_condattr_destroy(pthread_condattr_t *attr)
{
  return 0;
}

typedef struct {
  rtl_wait_t wait;
  spinlock_t lock;
} pthread_cond_t;


#define PTHREAD_COND_INITIALIZER { RTL_WAIT_INITIALIZER, SPIN_LOCK_UNLOCKED }

extern int pthread_cond_init(pthread_cond_t *cond,
		    const pthread_condattr_t *attr);
extern int pthread_cond_destroy(pthread_cond_t *cond);

extern int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

extern int pthread_cond_broadcast(pthread_cond_t *cond);

#define pthread_cond_signal(cond) pthread_cond_broadcast(cond)

extern int pthread_cond_timedwait(pthread_cond_t *cond,
		    pthread_mutex_t *mutex, const struct timespec *abstime);


#endif //__RTL_COND_H_
