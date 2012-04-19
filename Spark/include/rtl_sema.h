/*
 * RT-Linux semaphore implementation
 *
 * Written by Michael Barabanov <baraban@fsmlabs.com>
 * Copyright (C)  Finite State Machine Labs Inc. ,2000
 * Released under the terms of the GPL Version 2
 *
 */

#ifndef __RTL_SEMA_H__
#define __RTL_SEMA_H__

#include <rtl_conf.h>

#include <rtl_mutex.h>
#include <errno.h>

struct rtl_sem_struct
{
	int value;
	spinlock_t lock;
	rtl_wait_t wait;
};

typedef struct rtl_sem_struct sem_t;

extern int sem_init(sem_t *sem, int pshared, unsigned int value);
extern int sem_destroy(sem_t *sem);
extern int sem_getvalue(sem_t *sem, int *sval);
extern int sem_wait(sem_t *sem);
extern int sem_trywait(sem_t *sem);
extern int sem_post(sem_t *sem);

#ifdef _RTL_POSIX_TIMEOUTS
extern int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
#endif

#endif
