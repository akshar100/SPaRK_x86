/*
 * Copyright (C) 1999 FSM Labs (http://www.fsmlabs.com/)
 *  Written by Cort Dougan <cort@fsmlabs.com>
 *  and Victor Yodaiken <yodaiken@fsmlabs.com>
 */
#ifndef __ARCH_RTL_SYNC__
#define __ARCH_RTL_SYNC__

typedef unsigned long rtl_irqstate_t;

//#define __rtl_hard_save_flags(x)		__asm__ __volatile__("pushfl ; popl %0":"=g" (x): /* no input */ :"memory")
//#define __rtl_hard_restore_flags(x) 	__asm__ __volatile__("pushl %0 ; popfl": /* no output */ :"g" (x):"memory")
//#define __rtl_hard_cli() 		__asm__ __volatile__("cli": : :"memory")
//#define __rtl_hard_sti()		__asm__ __volatile__("sti": : :"memory")

#define __rtl_hard_savef_and_cli(x) 
#define __rtl_hard_save_flags(x)	
#define __rtl_hard_restore_flags(x) 
#define __rtl_hard_cli() 
#define __rtl_hard_sti()	


#endif

