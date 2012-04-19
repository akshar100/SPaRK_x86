/*
 * To add a new syste call increment NR_systemcalls by 1 and add the name of the new system call at the end of this file
 */
/*modified for PCI_DRIVER  */

#include <arch/errno.h>
#include <arch/linkage.h>
#include <arch/segment.h>

#define NR_systemcalls  41 

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


ENTRY(first_time_return)
	RESTORE_ALL

ENTRY(system_call)
	pushl %eax			# save orig_eax
	SAVE_ALL
	cmpl $(NR_systemcalls),%eax
	jae badsystem
	call *SYMBOL_NAME(system_table)(,%eax,4)
	movl %eax,EAX(%esp)		# save the return value
ENTRY(ret_from_system_call)
restore_all:
	RESTORE_ALL



badsystem:
	movl $-ENOSYS,EAX(%esp)
	jmp ret_from_system_call

.data
ENTRY(system_table)
	.long SYMBOL_NAME(sys_print_k)                  /* 0  */
	.long SYMBOL_NAME(pthread_make_periodic_np_k)   /* 1  */
	.long SYMBOL_NAME(pthread_wait_np_k)   		/* 2  */
	.long SYMBOL_NAME(sys_send_k)			/* 3  */
	.long SYMBOL_NAME(sys_recv_k)			/* 4  */
	.long SYMBOL_NAME(sys_rhine_get_ip_k)   	/* 5  */
	.long SYMBOL_NAME(sys_rhine_isready_k)   	/* 6  */
	.long SYMBOL_NAME(sys_rhine_transmit_k)   	/* 7  */
	.long SYMBOL_NAME(sys_pci1710_analog_out_k)	/* 8  */
	.long SYMBOL_NAME(sys_registerMQ_k)   		/* 9  */
	.long SYMBOL_NAME(sys_sendMQ_k)   		/* 10  */
	.long SYMBOL_NAME(sys_receiveMQ_k)   		/* 11  */
	.long SYMBOL_NAME(sys_reg_intrpt_k)   		/* 12  */
	.long SYMBOL_NAME(sys_get_kbd_data_k)   	/* 13 */
	.long SYMBOL_NAME(sys_getTime_k) 	  	/* 14 */
	.long SYMBOL_NAME(sys_printLong_k) 	  	/* 15 */
	.long SYMBOL_NAME(sys_timerExpired_k) 	  	/* 16 */
	.long SYMBOL_NAME(sys_timerSet_k) 	  	/* 17 */
	.long SYMBOL_NAME(sys_uip_init_k) 	  	/* 18 */
	.long SYMBOL_NAME(sys_uip_arp_init_k) 	  	/* 19 */
	.long SYMBOL_NAME(sys_uip_ip_init_k) 	  	/* 20 */
	.long SYMBOL_NAME(sys_uip_setethaddr_k)  	/* 21 */
	.long SYMBOL_NAME(sys_tapdev_read_k)     	/* 22 */
	.long SYMBOL_NAME(sys_uip_arp_ipin_k)    	/* 23 */
	.long SYMBOL_NAME(sys_uip_input_k)  	        /* 24 */
	.long SYMBOL_NAME(sys_uip_arp_out_k)  	        /* 25 */
	.long SYMBOL_NAME(sys_tapdev_send_k)  	        /* 26 */
	.long SYMBOL_NAME(sys_uip_arp_arpin_k)  	/* 27 */
	.long SYMBOL_NAME(sys_timerReset_k)             /* 28 */
	.long SYMBOL_NAME(sys_uip_periodic_k)   	/* 29 */
	.long SYMBOL_NAME(sys_uip_udp_periodic_k)  	/* 30 */
	.long SYMBOL_NAME(sys_uip_arp_timer_k)  	/* 31 */
	.long SYMBOL_NAME(sys_uip_get_len_k)    	/* 32 */
	.long SYMBOL_NAME(sys_uip_dummy_k)      	/* 33 */
	.long SYMBOL_NAME(sys_uip_send_response_k)    	/* 34 */
	.long SYMBOL_NAME(sys_uip_listen_k)             /* 35 */
	.long SYMBOL_NAME(sys_uip_connect_k)            /* 36 */
	.long SYMBOL_NAME(sys_uip_tcp_send_k)           /* 37 */
	.long SYMBOL_NAME(sys_perf_measurement_k)       /* 38 */
	.long SYMBOL_NAME(sys_task_switch_time_k)       /* 39 */
	.long SYMBOL_NAME(sys_rhine_poll_k)             /* 40 */

