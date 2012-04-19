

#include <arch/errno.h>
#include <arch/hypercall.h>
#include <arch/linkage.h>
#include <arch/segment.h>


EBX		= 0x00
ECX		= 0x04
EDX		= 0x08
ESI		= 0x0C
EDI		= 0x10
EBP		= 0x14
EAX		= 0x18
DS		= 0x1C
ES		= 0x20
GS		= 0x24
FS		= 0x28
ORIG_EAX	= 0x2C
EIP		= 0x30
CS		= 0x34
EFLAGS		= 0x38
OLDESP		= 0x3C
OLDSS		= 0x40

#define SAVE_ALL \
	cld; \
	pushl %fs;  \
	pushl %gs;  \
	pushl %es; \
	pushl %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $(__KERNEL_DS),%edx; \
	movl %edx,%ds; \
	movl %edx,%es;

#define RESTORE_ALL	\
	popl %ebx;	\
	popl %ecx;	\
	popl %edx;	\
	popl %esi;	\
	popl %edi;	\
	popl %ebp;	\
	popl %eax;	\
1:	popl %ds;	\
2:	popl %es;	\
	popl %gs;   \
	popl %fs;   \
	addl $4,%esp;	\
3:	iret;		


ENTRY(hyper_call)
	pushl %eax			# save orig_eax
	SAVE_ALL
	movl $1, iCallOn
	#movl %eax, hypercall_called 
	cmpl $(NR_hypercalls),%eax
	jae badhyper
	call *SYMBOL_NAME(hyper_table)(,%eax,4)
	movl %eax,EAX(%esp)		# save the return value
ENTRY(ret_from_hyper_call)
	movl $0, iCallOn
restore_all:
	RESTORE_ALL



badhyper:
	movl $-ENOSYS,EAX(%esp)
	jmp ret_from_hyper_call

.data
ENTRY(hyper_table)
	.long SYMBOL_NAME(spark_ni_hypercall)		/* 0  */
	.long SYMBOL_NAME(spark_print)			/* 1  */
	.long SYMBOL_NAME(spark_registerTimer)		/* 2  */
	.long SYMBOL_NAME(spark_registerPages)		/* 3  */
	.long SYMBOL_NAME(spark_loadPDE)		/* 4  */
	.long SYMBOL_NAME(spark_rhine_poll)		/* 5  */
	.long SYMBOL_NAME(spark_perf_measurement) 	/* 6  */
	.long SYMBOL_NAME(spark_parallel_out)		/* 7  */
	.long SYMBOL_NAME(spark_registerSysCallHandler)	/* 8  */
	.long SYMBOL_NAME(spark_setLevel2Stack ) 	/* 9  */
	.long SYMBOL_NAME(spark_printLong) 		/* 10  */
	.long SYMBOL_NAME(spark_register_interrupt) 	/* 11  */
	.long SYMBOL_NAME(spark_registerMQ)	 	/* 12  */
	.long SYMBOL_NAME(spark_send) 			/* 13  */
	.long SYMBOL_NAME(spark_receive) 		/* 14  */
/*PCI_DRIVER..*/
	.long SYMBOL_NAME(spark_rhine_get_ip)   	/* 15  */
	.long SYMBOL_NAME(spark_rhine_isready) 	        /* 16  */
	.long SYMBOL_NAME(spark_rhine_transmit)         /* 17  */
	.long SYMBOL_NAME(spark_rhine_receive)    	/* 18  */
/*PCI_DRIVER ~*/
