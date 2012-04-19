#ifndef _IDLE_H_
#define _IDLE_H_
 extern unsigned long stack_idletask[100];

#define IDLE_STACK_PTR (&stack_idletask[99])
 
#define SET_IDLE_STACK \
 asm volatile ("movl %0,%%esp \n" \
		::"i" (IDLE_STACK_PTR):"esp");


#endif //_IDLE_H_

