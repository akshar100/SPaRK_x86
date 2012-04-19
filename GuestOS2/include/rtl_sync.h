/*
 * Copyright (C) 1999 FSM Labs (http://www.fsmlabs.com/)
 *  Written by Cort Dougan <cort@fsmlabs.com> and
 *  Victor Yodaiken <yodaiken@fsmlabs.com>
 *
 */
#ifndef __RTL_SYNC__
#define __RTL_SYNC__


#include <rtl_conf.h>
#include <arch/rtl_sync.h>


#define	rtl_hard_savef_and_cli(s) __rtl_hard_savef_and_cli(s)
#define	rtl_hard_restore_flags(s) __rtl_hard_restore_flags(s)
#define	rtl_hard_cli() __rtl_hard_cli()
#define	rtl_hard_sti() __rtl_hard_sti()
#define rtl_hard_save_flags(s) __rtl_hard_save_flags(s)

#define	rtl_no_interrupts(s) rtl_hard_savef_and_cli(s)
#define	rtl_restore_interrupts(s) rtl_hard_restore_flags(s)
#define	rtl_stop_interrupts() rtl_hard_cli()
#define	rtl_allow_interrupts() rtl_hard_sti()

#define rtl_spinlock_t spinlock_t
#define rtl_spin_lock_init(x) spin_lock_init(x)
#define rtl_spin_lock(x) spin_lock(x)
#define rtl_spin_trylock(x) spin_trylock(x)
#define rtl_spin_unlock(x) spin_unlock(x)

#define rtl_spin_lock_irqsave(x, flags) \
	do { rtl_no_interrupts(flags); rtl_spin_lock(x); } while (0)
#define rtl_spin_unlock_irqrestore(x, flags) \
	do { rtl_spin_unlock(x); rtl_restore_interrupts(flags); } while (0)
//#define rtl_spin_lock_irqsave
//#define rtl_spin_unlock_irqrestore
#define rtl_critical(f) rtl_no_interrupts(f)
#define rtl_end_critical(f) rtl_restore_interrupts(f)

#endif

