/*
 * RTLinux POSIX spinlock implementation
 *
 * Written by Michael Barabanov
 * Copyright (C) Finite State Machine Labs Inc., 1999
 * Released under the terms of the GPL Version 2
 *
 */

#ifndef __RTL_SPINLOCK_H__
#define __RTL_SPINLOCK_H__

#include <rtl_sync.h>
#include <arch/rtl_sync.h>
#include <arch/spinlock.h>
#include <errno.h>

typedef struct {
	spinlock_t lock;
	rtl_irqstate_t flags;
} pthread_spinlock_t;

#define PTHREAD_SPINLOCK_INITIALIZER { SPIN_LOCK_UNLOCKED, 0 }

static inline int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
	spin_lock_init(&lock->lock);
	return 0;
}

static inline int pthread_spin_destroy(pthread_spinlock_t *lock)
{
	return 0;
}

static inline int pthread_spin_lock(pthread_spinlock_t *lock)
{
	rtl_irqstate_t flags;
	rtl_no_interrupts (flags);
	spin_lock (&lock->lock);
	lock->flags = flags;
	return 0;
}

static inline int pthread_spin_trylock(pthread_spinlock_t *lock)
{
	rtl_irqstate_t flags;
	rtl_no_interrupts (flags);
	if (rtl_spin_trylock(&lock->lock)) {
		rtl_restore_interrupts (flags);
		return EBUSY;
	} else {
		lock->flags = flags;
		return 0;
	}
}

static inline int pthread_spin_unlock(pthread_spinlock_t *lock)
{
	rtl_irqstate_t flags;
	flags = lock->flags;
	rtl_spin_unlock(&lock->lock);
	rtl_restore_interrupts (flags);
	return 0;
}

#endif
