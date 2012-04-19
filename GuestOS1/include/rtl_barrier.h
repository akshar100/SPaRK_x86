/*
 * rtl_barrier.h
 *
 * Written by Patricia Balbastre <patricia@disca.upv.es>
 * Copyright (C) Dec, 2002 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * RTLinux barrier implementation
 */

#ifndef __RTL_BARRIER_H__
#define __RTL_BARRIER_H__



#include <rtl_conf.h>
#include <rtl_mutex.h>
#include <rtl_spinlock.h>
#include <errno.h>

#ifdef CONFIG_OC_PBARRIERS

typedef struct {
	int pshared;
        int valid;
} pthread_barrierattr_t;

//pthread_barrierattr_t last_barrierattr;

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1
#define PTHREAD_BARRIER_SERIAL_THREAD 2

typedef struct {
	rtl_irqstate_t flags;
	int valid;
	spinlock_t lock;
        int count;
        int waiting;
        int busy;
        rtl_wait_t wait;
} pthread_barrier_t;


static inline int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared)
{
	*pshared = attr->pshared;
	return 0;
}

extern inline int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr,
		    int pshared)
{
	if (pshared != PTHREAD_PROCESS_SHARED
			&& pshared != PTHREAD_PROCESS_PRIVATE) {
		return EINVAL;
	}
	attr->pshared = pshared;
	return 0;
}


extern int pthread_barrierattr_init(pthread_barrierattr_t *attr);

extern inline int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
	return 0;
}

extern int pthread_barrier_init(pthread_barrier_t *barrier,
          const pthread_barrierattr_t *attr, unsigned count);

extern int pthread_barrier_destroy(pthread_barrier_t *barrier);

extern int pthread_barrier_wait(pthread_barrier_t *barrier);

#endif /*CONFIG_OC_PBARRIERS*/

#endif /*__RTL_BARRIER_H__*/
