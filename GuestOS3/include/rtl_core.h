/*
 * Main system for RTL.
 * 
 * Copyright (C) 1999 Cort Dougan <cort@fsmlabs.com>
 * Copyright (C) 1999 Victor Yodaiken <yodaiken@fsmlabs.com>
 */

#ifndef __RTL_CORE_H__
#define __RTL_CORE_H__

#include <rtl_version.h>
#include <arch/ptrace.h>
#include <arch/constants.h>

/* some old programs need this */
struct rtl_global_handlers{
    unsigned int (*handler)(unsigned int irq, struct pt_regs *r);
  } rtl_global_handlers[IRQ_MAX_COUNT];


#define dispatch_rtl_handler(irq,r) \
    if (*((unsigned long *) &rtl_global_handlers[irq])!=0) \
                  rtl_global_handlers[irq].handler(irq,r)

void rtl_hard_disable_irq(unsigned int);
void rtl_hard_enable_irq(unsigned int);
int rtl_request_global_irq(unsigned int irq, unsigned int (*handler)(unsigned int, struct pt_regs *));
int rtl_free_global_irq(unsigned int irq );


#define rtl_request_irq rtl_request_global_irq
#define rtl_free_irq rtl_free_global_irq



#endif
