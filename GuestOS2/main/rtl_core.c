/*
 * Stand-Alone RTLinux IRQ Managment Independent Stuff
 *
 * Written by Vicente Esteve LLoret
 * Released under the terms of the GPL Version 2
 *
 */

#include <rtl_conf.h>
#include <arch/mprot.h>
#include <rtl_core.h>
#include <rtl_sync.h>


int rtl_request_global_irq(unsigned int irq,
			   unsigned int (*handler)(unsigned int irq,struct pt_regs *r))
{
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  

  STARTKERNELCODE(mprot);
  rtl_global_handlers[irq].handler = handler;
  ENDKERNELCODE(mprot);
  return 1;
};
