//#include <arch/fpu.h>
//#include <arch/timer.h>

#ifndef __RTL_SWITCH_TO__
#define __RTL_SWITCH_TO__

//#define __STR(x) #x
//#define STR(x) __STR(x)

#define rtl_switch_to(current_task_ptr, new_task) \
	__asm__ __volatile__( \
	"pushl %%es\n\t" \
	"pushl %%ds\n\t" \
	"pushl %%fs\n\t" \
	"pushl %%gs\n\t" \
	"pushl %%eax\n\t" \
	"pushl %%ebp\n\t" \
	"pushl %%edi\n\t" \
	"pushl %%esi\n\t" \
	"pushl %%edx\n\t" \
	"pushl %%ecx\n\t" \
	"pushl %%ebx\n\t" \
	"pushl $1f\n\t" \
        "movl %%esp,%1\n\t"	/* save ESP */		\
	"movl %0,%%esp\n\t"	/* restore ESP */	\
	"ret\n\t" \
"1:      popl %%ebx\n\t" \
	"popl %%ecx\n\t" \
	"popl %%edx\n\t" \
	"popl %%esi\n\t" \
	"popl %%edi\n\t" \
	"popl %%ebp\n\t" \
	"popl %%eax\n\t" \
	"popl %%gs\n\t" \
	"popl %%fs\n\t" \
	"popl %%ds\n\t" \
	"popl %%es\n\t" \
	: \
	: "m" (new_task->stack_addr), "m" (current_task_ptr->stack_addr) \
	);

#if 0
#define switch_to(prev,next,last) do {					\
	asm volatile("pushl %%esi\n\t"					\
		     "pushl %%edi\n\t"					\
		     "pushl %%ebp\n\t"					\
		     "movl %%esp,%0\n\t"	/* save ESP */		\
		     "movl %3,%%esp\n\t"	/* restore ESP */	\
		     "movl $1f,%1\n\t"		/* save EIP */		\
		     "pushl %4\n\t"		/* restore EIP */	\
		     "jmp __switch_to\n"				\
		     "1:\t"						\
		     "popl %%ebp\n\t"					\
		     "popl %%edi\n\t"					\
		     "popl %%esi\n\t"					\
		     :"=m" (prev->stack_addr),"=m" (prev->thread.eip),	\
		      "=b" (last)					\
		     :"m" (next->thread.esp),"m" (next->thread.eip),	\
		      "a" (prev), "d" (next),				\
		      "b" (prev));					\
} while (0)

#define rtl_init_stack(task,fn,data,rt_startup)\
	{\
	*--(task->stack) = (int) data;\
	*--(task->stack) = (int) fn;\
	*--(task->stack) = 0;	/* dummy return addr*/\
	*--(task->stack) = (int) rt_startup;\
}
#endif

//#define RTL_PSC_NEW 1
#endif // __RTL_SWITCH_TO__

