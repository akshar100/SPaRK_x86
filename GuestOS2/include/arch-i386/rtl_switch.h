#include <arch/fpu.h>
//#include <arch/timer.h>

#define __STR(x) #x
#define STR(x) __STR(x)

#define rtl_switch_to(current_task_ptr, new_task) \
	__asm__ __volatile__( \
	"pushl %%eax\n\t" \
	"pushl %%ebp\n\t" \
	"pushl %%edi\n\t" \
	"pushl %%esi\n\t" \
	"pushl %%edx\n\t" \
	"pushl %%ecx\n\t" \
	"pushl %%ebx\n\t" \
	"movl  (%%ebx), %%edx\n\t" \
	"pushl $1f\n\t" \
	"movl %%esp, (%%edx)\n\t" \
	"movl (%%ecx), %%esp\n\t" \
	"movl %%ecx, (%%ebx)\n\t" \
	"ret\n\t" \
"1:      popl %%ebx\n\t" \
	"popl %%ecx\n\t" \
	"popl %%edx\n\t" \
	"popl %%esi\n\t" \
	"popl %%edi\n\t" \
	"popl %%ebp\n\t" \
	"popl %%eax\n\t" \
	: /* no output */ \
	: "c" (new_task), "b" (current_task_ptr) \
	);

/*
#define rtl_init_stack(task,fn,data,rt_startup)\
	{\
	*--(task->stack) = (int) data;\
	*--(task->stack) = (int) fn;\
	*--(task->stack) = 0;	\
	*--(task->stack) = (int) rt_startup;\
}
*/

#define rtl_init_stack(task,stack_address,task_start_add)\
	do{\
	*--(task->stack) = __USER_DS;\
	*--(task->stack) = stack_address;\
	*--(task->stack) = 0x0202; \
	*--(task->stack) = __USER_CS;\
	*--(task->stack) = task_start_add;\
	*--(task->stack) = 0; /*dummy for orig_eax*/ \
	*--(task->stack) = __USER_DS; /*FS*/ \
	*--(task->stack) = __USER_DS; /*GS*/ \
	*--(task->stack) = __USER_DS; /*ES*/ \
	*--(task->stack) = __USER_DS; /*DS*/ \
	*--(task->stack) = 0; /*eax*/ \
	*--(task->stack) = 0; /*ebp*/ \
	*--(task->stack) = 0; /*edi*/ \
	*--(task->stack) = 0; /*esi*/ \
	*--(task->stack) = 0; /*edx*/ \
	*--(task->stack) = 0; /*ecx*/ \
	*--(task->stack) = 0; /*ebx*/ \
	*--(task->stack) = (unsigned int)first_time_return; \
} while(0)

// #define RTL_PSC_NEW 1
