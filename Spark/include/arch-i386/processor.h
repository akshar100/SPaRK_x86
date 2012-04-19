
#ifndef __PROCESSOR_H_
#define __PROCESSOR_H_

#include <arch/segment.h>
#include <guests.h>
#include <arch/page.h>
#include <rtl_time.h>
/*
 * Size of io_bitmap in longwords: 32 is ports 0-0x3ff.
 */
#define IO_BITMAP_SIZE	32
#define INVALID_IO_BITMAP_OFFSET 0x8000

struct tss_struct {
	unsigned short	back_link,__blh;
	unsigned long	esp0;
	unsigned short	ss0,__ss0h;
	unsigned long	esp1;
	unsigned short	ss1,__ss1h;
	unsigned long	esp2;
	unsigned short	ss2,__ss2h;
	unsigned long	__cr3;
	unsigned long	eip;
	unsigned long	eflags;
	unsigned long	eax,ecx,edx,ebx;
	unsigned long	esp;
	unsigned long	ebp;
	unsigned long	esi;
	unsigned long	edi;
	unsigned short	es, __esh;
	unsigned short	cs, __csh;
	unsigned short	ss, __ssh;
	unsigned short	ds, __dsh;
	unsigned short	fs, __fsh;
	unsigned short	gs, __gsh;
	unsigned short	ldt, __ldth;
	unsigned short	trace, bitmap;
	unsigned long	io_bitmap[IO_BITMAP_SIZE+1];
	/*
	 * pads the TSS to be cacheline-aligned (size is 0x100)
	 */
	unsigned long __cacheline_filler[5];
};


struct thread_struct {
	// store the address of kernel mode stack, used while resuming a Guest OS
	unsigned long   *stack_addr;
	// store the address of gdt table corresponding to the Guest OS
	unsigned char   *gdt_addr;
	//unsigned long   cr0, cr1, cr2, cr3;

	unsigned char	bIsActive;
	int		iTotalBudget;
	int		iBudgetRemaining;
	//int		iTicksElapsed;
	//int		iTicksTotal;
	//store the pdes of all the tasks in a Guest OS
	page_entry	*pde[MAX_TASK_PER_GUEST];
	// where the Guest OS is residing in the memory
	unsigned long   phyOffset;
	// Task ID
	unsigned int	iCurrTaskId;
	unsigned int	iGOSNumber;
	// store the address of starting address of stack
	unsigned int	stack_start;
	unsigned long	stack_start_level2;
	// parameters for timer handler
	int		iCurrTicks;
	int		iTicks;
	unsigned int	iHandler;
	hrtime_t    *pllRealTime;
// ioperm data..
	unsigned long	lSerialPerm;
	unsigned long	lParallelPerm;
// ioperm data~
};


#define INIT_STACK_SIZE	2048
union task_union {
	struct thread_struct task;
	unsigned long stack[INIT_STACK_SIZE];
};

extern union task_union __attribute((aligned(16))) init_task_union[NUM_OF_GUESTOS];

#define __LDT(n) 	0

#define INIT_TSS  {						\
	0,0, /* back_link, __blh */				\
	0,   \
	__KERNEL_DS, 0, /* ss0 */				\
	0,0,0,0,__GUESTOS_DS,0, /* stack1, stack2 */		\
	0, /* cr3 */						\
	0,0, /* eip,eflags */					\
	0,0,0,0, /* eax,ecx,edx,ebx */				\
	0,0,0,0, /* esp,ebp,esi,edi */				\
	0,0,0,0,0,0, /* es,cs,ss */				\
	0,0,0,0,0,0, /* ds,fs,gs */				\
	__LDT(0),0, /* ldt */					\
	0, INVALID_IO_BITMAP_OFFSET, /* tace, bitmap */		\
	{~0, } /* ioperm */					\
}


#define load_TR(n) __asm__ __volatile__("ltr %%ax"::"a" (__TSS(n)<<3))
#define __load_GDT(n) __asm__ __volatile__("lgdt %%ax"::"a" (n))

extern struct tss_struct init_tss;
//extern struct thread_struct kernel_thread ;
extern struct thread_struct guestOS_thread[NUM_OF_GUESTOS+1];

struct gdt_desc_entry
{
	unsigned short seg_lim_0_15;
	unsigned short base_addr_0_15;
	unsigned char base_16_23;
	unsigned char par1;
	unsigned char seg_lim_16_19;
	unsigned char base_24_31;
};


struct gdt_desc
{
	struct gdt_desc_entry null0;
	struct gdt_desc_entry null1;
	struct gdt_desc_entry spark_code;
	struct gdt_desc_entry spark_data;
	struct gdt_desc_entry guest_ker_code;
	struct gdt_desc_entry guest_ker_data;
	struct gdt_desc_entry guest_user_code;
	struct gdt_desc_entry guest_user_data;
	//struct gdt_desc_entry dummy1;
	//struct gdt_desc_entry dummy2;
	struct gdt_desc_entry tss_entry1;
	struct gdt_desc_entry tss_entry2;
};


#endif
