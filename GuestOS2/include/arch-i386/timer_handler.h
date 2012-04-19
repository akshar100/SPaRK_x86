
#include <arch/linkage.h>
#include <arch/segment.h>


#define __STR(x) #x
#define STR(x) __STR(x)

//	"popf		\t\n"	\
//	"pushf \n\t" \


#define RESTOREALL  	\
	"popl %ebx;	\t\n"	\
	"popl %ecx;	\t\n"	\
	"popl %edx;	\t\n"	\
	"popl %esi;	\t\n"	\
	"popl %edi;	\t\n"	\
	"popl %ebp;	\t\n"	\
	"popl %eax;	\t\n"	\
	"popl %ds;	\t\n"	\
	"popl %es;	\t\n"	\
	"popl %gs;	\t\n"	\
	"popl %fs;	\t\n"	\

#define SAVEALL \
	"cld\n\t" \
	"pushl %fs\n\t" \
	"pushl %gs\n\t" \
	"pushl %es\n\t" \
	"pushl %ds\n\t" \
	"pushl %eax\n\t" \
	"pushl %ebp\n\t" \
	"pushl %edi\n\t" \
	"pushl %esi\n\t" \
	"pushl %edx\n\t" \
	"pushl %ecx\n\t" \
	"pushl %ebx\n\t" \
	"movl $" STR(__KERNEL_DS) ",%edx\n\t" \
	"movl %edx,%ds\n\t" \
	"movl %edx,%es\n\t" \


#define TIMER_IRQ() \
__asm__( \
	".globl timer_irq\n\t" \
	"\n" __ALIGN_STR"\n" \
	"timer_irq:\n\t" \
	SAVEALL \
	"call "SYMBOL_NAME_STR(timer_irq_handler)"\n\t" \
	RESTOREALL \
	"iret \n\t");


void timer_irq_handler();
