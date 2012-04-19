/*
 * Modified at IITB, ERTS LAB
 * Stand-Alone RTLinux IRQ Managment
 *
 * Written by Vicente Esteve LLoret
 * Released under the terms of the GPL Version 2
 *
 */
#include <arch/rtl_io.h>
#include <arch/hw_irq.h>
#include <arch/linkage.h>
#include <arch/segment.h>
#include <arch/timer.h>
#include <arch/mprot.h>
#include <rtl_core.h>
#include <rtl_conf.h>
#include <arch/processor.h>
#include <rtl_printf.h>
#include <sys/io.h>
#include <rtl_sched.h>

asmlinkage int hyper_call(void);
extern struct thread_struct *next_thread , *prev_thread;
extern int	iSwitchContext;

/* Intel 8259 ports */
#define I8259_A0	0x020			// 8259 #1, port #1
#define I8259_A1	0x021			// 8259 #1, port #2
#define I8259_B0	0x0a0			// 8259 #2, port #1
#define I8259_B1	0x0a1			// 8259 #2, port #2

/*
 * This contains the irq mask for both 8259A irq controllers,
 */
unsigned int cached_irq_mask = 0xffff;
#define __byte(x,y) 	(((unsigned char *)&(y))[x])
#define cached_21	(__byte(0,cached_irq_mask))
#define cached_A1	(__byte(1,cached_irq_mask))

DEF_DEFAULT_IRQ()
BUILD_RET_FROM_INTR()
BUILD_COMMON_IRQ()
COMMON_RET_GUEST_OS()

BUILD_INT(0)
BUILD_INT(1)
BUILD_INT(2)
BUILD_INT(3)
BUILD_INT(4)
BUILD_INT(5)
BUILD_INT(6)
BUILD_INT(7)
BUILD_INT(8)
BUILD_INT(9)
BUILD_INT(10)
BUILD_INT(11)

asmlinkage void	irq_12_handler(struct pt_regs regs)
{
	rtl_printf("\n\n***Exception:Stack Fault\teip:0x%x\tError code\t0x%x***\n\n",regs.eip,regs.orig_eax);
}

__asm__("\n"__ALIGN_STR"\n");
asmlinkage void IRQ12_int_int(void)
{
	__asm__(
			SAVE_ALL
			"call "SYMBOL_NAME_STR(irq_12_handler)"\n\t"
		);
	
	__asm__(
			RESTORE_ALL
			"int $0x3;         \t\n"
			"hlt;              \t\n"
			"                  \t\n"
			"iret              \t\n"
		);
}



asmlinkage void	irq_13_handler(struct pt_regs regs)
{
	rtl_printf("\n\n***Exception:General Protection\teip:0x%x\tError code\t0x%x***\n\n",regs.eip,regs.orig_eax);

	guestOS_thread[iCurrGuestOsIndex].iTotalBudget = 0;
	guestOS_thread[iCurrGuestOsIndex].iBudgetRemaining = 0;
        while(1);
	spark_schedule();
}

__asm__("\n"__ALIGN_STR"\n");
asmlinkage void IRQ13_int_int(void)
{
	__asm__(
			SAVE_ALL
			"call "SYMBOL_NAME_STR(irq_13_handler)"\n\t"
		);
	
	__asm__(
			RESTORE_ALL
		);

}



int	cr2=0;
asmlinkage void	irq_14_handler(struct pt_regs regs)
{
	__asm__("movl %cr2, %eax");
	__asm__("movl %%eax, %0": "=m" (cr2));

	rtl_printf("\n\n***Exception:Page Fault\taddr:0x%x ***\n\n",cr2);

	rtl_printf("\n\n***Exception:Page Fault Error Codewa:0x%x ***\n\n",regs.orig_eax);


	guestOS_thread[iCurrGuestOsIndex].iTotalBudget = 0;
	guestOS_thread[iCurrGuestOsIndex].iBudgetRemaining = 0;
        while(1);
	spark_schedule();
}

__asm__("\n"__ALIGN_STR"\n");
asmlinkage void IRQ14_int_int(void)
{
	__asm__(
			SAVE_ALL
			"call "SYMBOL_NAME_STR(irq_14_handler)"\n\t"
		);

	
	__asm__(
			RESTORE_ALL
		);

}


BUILD_INT(15)
BUILD_INT(16)
BUILD_INT(17)
BUILD_INT(18)
BUILD_INT(19)

#define BI(x,y) BUILD_IRQ(x##y)

#define BUILD_16_IRQS(x) \
	BI(x,0) BI(x,1) BI(x,2) BI(x,3) \
	BI(x,4) BI(x,5) BI(x,6) BI(x,7) \
	BI(x,8) BI(x,9) BI(x,a) BI(x,b) \
	BI(x,c) BI(x,d) BI(x,e) BI(x,f)

	/*
	 * ISA PIC or low IO-APIC triggered (INTA-cycle or APIC) interrupts:
	 * (these are usually mapped to vectors 0x20-0x2f)
	 */
BUILD_16_IRQS(0x0)

#define IRQ(x,y) IRQ##x##y##_interrupt
#define IRQLIST_16(x) \
	IRQ(x,0), IRQ(x,1), IRQ(x,2), IRQ(x,3), \
	IRQ(x,4), IRQ(x,5), IRQ(x,6), IRQ(x,7), \
	IRQ(x,8), IRQ(x,9), IRQ(x,a), IRQ(x,b), \
	IRQ(x,c), IRQ(x,d), IRQ(x,e), IRQ(x,f)

void (*interrupt[])(void) = {
	IRQLIST_16(0x0)
};

struct desc_struct{
	unsigned long a,b;
};

/*
 * The IDT has to be page-aligned to simplify the Pentium
 * F0 0F bug workaround.. We have a special link segment
 * for this.
 */

#if CONFIG_KERNEL_MEMORYPROT
struct desc_struct idttable[256] __attribute__((__section__(".data.idt"))) = { {0, 0}, };
#else
struct desc_struct idttable[256] = { {0, 0}, };
#endif

void Program_IDT(void)
{
	__asm__(" lidt idtdescr             \t\n" \
			" jmp 1f                      \t\n" \
			__ALIGN_STR                  "\t\n" \
			" idtdescr:                 \t\n" \
			".word 256*8-1         	     \t\n" \
			".long "SYMBOL_NAME_STR(idttable)"  \t\n" \
			" 1:                                \t\n");
};

#define _set_gate(gate_addr,type,dpl,addr) \
	do { \
		int __d0, __d1; \
		__asm__ __volatile__ ("movw %%dx,%%ax\n\t" \
		"movw %4,%%dx\n\t" \
		"movl %%eax,%0\n\t" \
		"movl %%edx,%1" \
		:"=m" (*((long *) (gate_addr))), \
		"=m" (*(1+(long *) (gate_addr))), "=&a" (__d0), "=&d" (__d1) \
		:"i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
		"3" ((char *) (addr)),"2" (__KERNEL_CS << 16)); \
	} while (0)


#define _set_gate2(gate_addr,type,dpl,addr) \
	do { \
		int __d0, __d1; \
		__asm__ __volatile__ ("movw %%dx,%%ax\n\t" \
		"movw %4,%%dx\n\t" \
		"movl %%eax,%0\n\t" \
		"movl %%edx,%1" \
		:"=m" (*((long *) (gate_addr))), \
		"=m" (*(1+(long *) (gate_addr))), "=&a" (__d0), "=&d" (__d1) \
		:"i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
		"3" ((char *) (addr)),"2" (__GUESTOS_CS << 16)); \
	} while (0)




void set_intr_gate(unsigned int n, void *addr){
	_set_gate(idttable+n,14,0,addr);
}

void set_trap_gate(unsigned int n, void *addr){
	_set_gate(idttable+n,15,0,addr);
}

void set_system_gate(unsigned int n, void *addr)
{
	_set_gate(idttable+n,15,2,addr);
}

void set_system_gate3(unsigned int n, void *addr)
{
	_set_gate2(idttable+n,15,3,addr);
}

/*
 * This function assumes to be called rarely. Switching between
 * 8259A registers is slow.
 * This has to be protected by the irq controller spinlock
 * before being called.
 */
static inline int i8259A_irq_real(unsigned int irq)
{
	int value;
	int irqmask = 1<<irq;

	if (irq < 8) {
		rtl_outb(0x0B,0x20);		/* ISR register */
		value = rtl_inb(0x20) & irqmask;
		rtl_outb(0x0A,0x20);		/* back to the IRR register */
		return value;
	}
	rtl_outb(0x0B,0xA0);		/* ISR register */
	value = rtl_inb(0xA0) & (irqmask >> 8);
	rtl_outb(0x0A,0xA0);		/* back to the IRR register */
	return value;
}

void mask_and_ack_8259A(unsigned int irq)
{
	unsigned int irqmask = 1 << irq;

	if (cached_irq_mask & irqmask)
		goto spurious_8259A_irq;
	cached_irq_mask |= irqmask;
handle_real_irq:
	if (irq & 8) {
		rtl_inb(0xA1);		/* DUMMY - (do we need this?) */
		rtl_outb(cached_A1,0xA1);
		rtl_outb(0x60+(irq&7),0xA0);/* 'Specific EOI' to slave */
		rtl_outb(0x62,0x20);	/* 'Specific EOI' to master-IRQ2 */
	} else {
		rtl_inb(0x21);		/* DUMMY - (do we need this?) */
		rtl_outb(cached_21,0x21);
		rtl_outb(0x60+irq,0x20);	/* 'Specific EOI' to master */
	}
	return;

spurious_8259A_irq:
	/*
	 * this is the slow path - should happen rarely.
	 */
	if (i8259A_irq_real(irq))
		/*
		 * oops, the IRQ _is_ in service according to the
		 * 8259A - not spurious, go handle it.
		 */
		goto handle_real_irq;

	{
		static int spurious_irq_mask;
		/*
		 * At this point we can be sure the IRQ is spurious,
		 * lets ACK and report it. [once per IRQ]
		 */
		if (!(spurious_irq_mask & irqmask)) {
			spurious_irq_mask |= irqmask;
		}
		//		atomic_inc(&irq_err_count);
		/*
		 * Theoretically we do not have to handle this IRQ,
		 * but in Linux this does not cause problems and is
		 * simpler for us.
		 */
		goto handle_real_irq;
	}
}

void enable_8259_irq(unsigned int irq)
{
	unsigned int mask = ~(1 << irq);
	cached_irq_mask &= mask;
	if (irq & 8)
		rtl_outb(cached_A1,0xA1);
	else
		rtl_outb(cached_21,0x21);
}

asmlinkage unsigned int do_IRQ(struct pt_regs regs)
{
	mask_and_ack_8259A(regs.orig_eax & 0xff);
	dispatch_rtl_handler(regs.orig_eax & 0xff,&regs);
	enable_8259_irq(regs.orig_eax & 0xff);

	return 1; 
}

asmlinkage unsigned int ret_Guest(struct pt_regs regs)
{
	enable_8259_irq(regs.orig_eax & 0xff);
	return 1;
}


void setup_arch(void)
{
	int i;
	unsigned int vector;

	rtl_outb(cached_21, 0x21);	/* restore master IRQ mask */
	rtl_outb(cached_A1, 0xA1);	/* restore slave IRQ mask */

	/* Reprogram the master 8259. */
	rtl_outb_p(0x11,I8259_A0);      /* ICW1: select 8259A-1 init */
	rtl_outb_p(0x20,I8259_A1);      /* ICW2: 8259A-1 IR0-7 mapped to 0x20-0x27 */
	rtl_outb_p(0x04,I8259_A1);     	/* 8259A-1 (the master) has a slave on IR2 */

	rtl_outb_p(0x01,I8259_A1);      /* master expects normal EOI */

	/* Reprogram the slave 8259. */
	rtl_outb_p(0x11,I8259_B0);     /* ICW1: select 8259A-2 init */
	rtl_outb_p(0x28,I8259_B1);     /* ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f */
	rtl_outb_p(0x02,I8259_B1);     /* 8259A-2 is a slave on master's IR2 */
	rtl_outb_p(0x01,I8259_B1);     /* (slave's support for AEOI in flat mode is not clear) */

	for (i=0;i<0x1FFFFFFF;i++){}; /* wait for 8259A to initialize */

	cached_21 = 0xff;
	rtl_outb(cached_21, 0x21);	/* restore master IRQ mask */
	rtl_outb(cached_A1, 0xA1);	/* restore slave IRQ mask */

	/*
	 * Cover the whole vector space, no vector can escape
	 * us. (some of these will be overridden and become
	 * 'special' SMP interrupts)
	 */
	for (vector = 0; vector < 256; vector++) {
		set_intr_gate(vector, &default_interrupt);
	};
	for (vector = 0x0; vector < 0x10; vector++) {
		set_intr_gate(vector+0x20, interrupt[vector]);
	};

	set_intr_gate(0,&IRQ0_int_int);
	set_intr_gate(1,&IRQ1_int_int);
	set_intr_gate(2,&IRQ2_int_int);
	set_intr_gate(3,&IRQ3_int_int);
	set_intr_gate(4,&IRQ4_int_int);
	set_intr_gate(5,&IRQ5_int_int);
	set_intr_gate(6,&IRQ6_int_int);
	set_intr_gate(7,&IRQ7_int_int);
	set_intr_gate(8,&IRQ8_int_int);
	set_intr_gate(9,&IRQ9_int_int);
	set_intr_gate(10,&IRQ10_int_int);
	set_intr_gate(11,&IRQ11_int_int);
	set_intr_gate(12,&IRQ12_int_int);
	set_intr_gate(13,&IRQ13_int_int);
	set_intr_gate(14,&IRQ14_int_int);
	set_intr_gate(15,&IRQ15_int_int);
	set_intr_gate(16,&IRQ16_int_int);
	set_intr_gate(17,&IRQ17_int_int);
	set_intr_gate(18,&IRQ18_int_int);
	set_intr_gate(19,&IRQ19_int_int);

	set_system_gate(HYPERCALL_VECTOR,&hyper_call);
	//set_intr_gate(HYPERCALL_VECTOR,&hyper_call);
	Program_IDT();
}
