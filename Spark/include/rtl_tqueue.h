/*
 * remnants of RTLinux task queues
 *
 *  Copyright (C) 1999 FSM Labs (http://www.fsmlabs.com/)
 *  Written by Michael Barabanov <baraban@fsmlabs.com>
 */

#ifndef __RTL_TQUEUE__
#define __RTL_TQUEUE__

#include <rtl.h>
#include <linux/tqueue.h>

#if LINUX_2_4_0_FINAL_OR_LATER 
#define rtl_schedule_task(tsk) schedule_task(tsk)
#else
#define rtl_schedule_task(tsk) queue_task(tsk, &tq_scheduler)
#endif

#endif
