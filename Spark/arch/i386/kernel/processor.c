/*
* Developed at IITB, ERTS LAB
*
*/

#include <rtl_conf.h>
#include <arch/processor.h>
#include <arch/page.h>
#include <arch/segment.h>
#include <arch/ptrace.h>
#include <rtl_printf.h>
#include <guests.h>

#define	showChar(x) \
	__asm__ __volatile__ ("movw $0x18, %%ax \n\t" \
			"movw %%ax, %%gs \n\t" \
			"movb %0, %%gs:0xB81E0 \n\t" \
			: : "i"(x))

struct desc_struct {
	unsigned long a;
	unsigned long b;
};

extern unsigned char  gdt_desc[];
extern unsigned char  head_stack_start[];
struct gdt_desc dynamic_gdts[NUM_OF_GUESTOS];
struct tss_struct init_tss = INIT_TSS;
//struct thread_struct kernel_thread ;
struct thread_struct guestOS_thread[NUM_OF_GUESTOS+1] ;
//struct thread_struct guestOS_thread[NUM_OF_GUESTOS] ;
union task_union __attribute((aligned(16))) init_task_union[NUM_OF_GUESTOS];
extern void common_return();

#define __FIRST_TSS_ENTRY 8
#define __TSS(n) (((n)<<2) + __FIRST_TSS_ENTRY)  // n*4 + first TSS entry
#define _set_tssldt_desc(n,addr,limit,type) \
	__asm__ __volatile__ ("movw %w3,0(%2)\n\t" \
			"movw %w1,2(%2)\n\t" \
			"rorl $16,%1\n\t" \
			"movb %b1,4(%2)\n\t" \
			"movb %4,5(%2)\n\t" \
			"movb $0,6(%2)\n\t" \
			"movb %h1,7(%2)\n\t" \
			"rorl $16,%1" \
			: "=m"(*(n)) : "q" (addr), "r"(n), "ir"(limit), "i"(type))

void set_tss_desc(struct desc_struct gdt[], unsigned int n, void *addr)
{
	_set_tssldt_desc( gdt + __TSS(n), (int)addr, 235, 0x89);
	//_set_tssldt_desc( gdt2 + __TSS(n), (int)addr, 235, 0x89);
}

unsigned long * init_stack_guestOS(unsigned long *stackaddr, unsigned long offset)
{

	*(--stackaddr) = __GUESTOS_DS;  // %ss stack segment saved by lcall
	*(--stackaddr) = offset + 0x100;   // %esp value .....
	// initial stack address would be anything in the proper segment
	// it will overwrite , as the first time guest OS will make its
	// own stack
	// Temp change *(--stackaddr) = 0x2202;   // initial EFLAG value
	*(--stackaddr) = 0x0202;   // initial EFLAG value
	*(--stackaddr) = __GUESTOS_CS; //0x22;   // %cs for guest os 1
	*(--stackaddr) = offset; // %eip
	*(--stackaddr) = (0x00-256);		// orig_eax
	*(--stackaddr) = __GUESTOS_DS;  // xfs
	*(--stackaddr) = __GUESTOS_DS;  // xgs
	*(--stackaddr) = __GUESTOS_DS;  // xes 
	*(--stackaddr) = __GUESTOS_DS;  // xds
	*(--stackaddr) = 0;		// eax
	*(--stackaddr) = 0;		// ebp
	*(--stackaddr) = 0;		// edi
	*(--stackaddr) = 0;		// esi
	*(--stackaddr) = 0;  		// edx
	*(--stackaddr) = 0;		// ecx
	*(--stackaddr) = 0;		// ebx
	*(--stackaddr) = (unsigned long)common_return;
	return stackaddr;
}

void  fill_gdt_entry(struct gdt_desc_entry *entry1,  unsigned int base_addr, unsigned int size, unsigned char par)
{	
	size = size/0x1000;
	entry1->seg_lim_0_15 = size & 0xffff;

	entry1->base_addr_0_15 = base_addr & 0xffff;
	entry1->base_16_23 = (base_addr>>16) & 0xff;
	entry1->par1       =  par;    // code read/exec = 0x9A, data read/write = 0x92
	entry1->seg_lim_16_19 = ((size >> 16) & 0xf) | 0xC0;
	entry1->base_24_31   = (base_addr>>24) & 0xff;

}

void make_gdt()
{
	int i, guest_ba;

	for(i=0; i< NUM_OF_GUESTOS; i++)
	{
		guest_ba = MEMORY_ALLOTED_TO_SPARK + i*	MEMORY_ALLOTED_TO_EACH_GUEST;
		fill_gdt_entry(&dynamic_gdts[i].spark_code, 0, 0xFFFFF000, 0x9A);
		fill_gdt_entry(&dynamic_gdts[i].spark_data, 0, 0xFFFFF000, 0x92);
		fill_gdt_entry(&dynamic_gdts[i].guest_ker_code, guest_ba, MEMORY_ALLOTED_TO_EACH_GUEST, 0xDA);
		fill_gdt_entry(&dynamic_gdts[i].guest_ker_data, guest_ba, MEMORY_ALLOTED_TO_EACH_GUEST, 0xD2);
		fill_gdt_entry(&dynamic_gdts[i].guest_user_code, guest_ba, MEMORY_ALLOTED_TO_EACH_GUEST, 0xFA);
		fill_gdt_entry(&dynamic_gdts[i].guest_user_data, guest_ba, MEMORY_ALLOTED_TO_EACH_GUEST, 0xF2);
		//fill_gdt_entry(&dynamic_gdts[i].dummy1, 0, 0xFFFFF000, 0xDA);
		//fill_gdt_entry(&dynamic_gdts[i].dummy2, 0, 0xFFFFF000, 0xD2);
	}
	return;
}

// current ioperm offsets..
#define	IOPERM_OFFSET_DUMMY	4
#define	IOPERM_OFFSET_SERIAL	31
#define	IOPERM_OFFSET_PARALLEL	27
// current ioperm offsets~

void fLoadNewMemContext(struct thread_struct *next_gos)
{
	struct tss_struct * t = &init_tss;
	unsigned long *a;

	a = (unsigned long *)(gdt_desc + 2);
	*a = (unsigned long *)next_gos->gdt_addr; 
	__asm__ __volatile__( "lgdt %0": "=m" (gdt_desc) );
	t->esp0 = next_gos->stack_start;
	t->esp2 = next_gos->stack_start_level2;
// ioperms..
	t->io_bitmap[IOPERM_OFFSET_PARALLEL] = next_gos->lParallelPerm;
	t->io_bitmap[IOPERM_OFFSET_SERIAL] = next_gos->lSerialPerm;
// ioperms~	
	SET_CR3(next_gos->pde[next_gos->iCurrTaskId]);
}

void fLoadEsp2(void)
{
	struct tss_struct * t = &init_tss;
	t->esp2 = guestOS_thread[iCurrGuestOsIndex].stack_start_level2;
}


void    make_tss_ioperm(void)
{
	struct tss_struct *t = &init_tss;
	int i = 0;

	// EB
	t->bitmap = 0x68;   // 104 bytes

	for(i = 0; i < IO_BITMAP_SIZE ; i++)
	{
		t->io_bitmap[i] = ~0L;
	}
	t->io_bitmap[IOPERM_OFFSET_DUMMY] = 0L;
	t->io_bitmap[IO_BITMAP_SIZE] = ~0L;

// io permissions for individual guest OSs... 0 = access, ~0 = deny
// partition 1
	guestOS_thread[0].lSerialPerm = ~0L;
	guestOS_thread[0].lParallelPerm = ~0L;
// partition 2
	guestOS_thread[1].lSerialPerm = ~0L;
	guestOS_thread[1].lParallelPerm = 0L;
// partition 3
	guestOS_thread[2].lSerialPerm = 0L;
	guestOS_thread[2].lParallelPerm = ~0L;

}


void cpu_init (void)
{
	int	iCount,i;
	struct tss_struct *t = &init_tss;
	unsigned long *a;

	make_gdt();

	make_tss_ioperm();

	for (i = 0 ; i < NUM_OF_GUESTOS ; i++)
	{
		set_tss_desc( (struct desc_struct *) &(dynamic_gdts[i]), 0, t);
		guestOS_thread[i].stack_start = init_task_union[i].stack + INIT_STACK_SIZE;
		guestOS_thread[i].stack_addr = init_task_union[i].stack + INIT_STACK_SIZE;
#ifdef _USE_SEGMENTATION_FOR_PROT_
		guestOS_thread[i].phyOffset = MEMORY_ALLOTED_TO_SPARK + i*MEMORY_ALLOTED_TO_EACH_GUEST;
		guestOS_thread[i].stack_addr = init_stack_guestOS(guestOS_thread[i].stack_addr, 0);
#else
		guestOS_thread[i].phyOffset = 0;
		guestOS_thread[i].stack_addr = init_stack_guestOS(guestOS_thread[i].stack_addr, \
				MEMORY_ALLOTED_TO_SPARK + i*MEMORY_ALLOTED_TO_EACH_GUEST );
#endif
		guestOS_thread[i].gdt_addr = (unsigned char *)(&(dynamic_gdts[i]));
		guestOS_thread[i].iCurrTaskId = 0;
		guestOS_thread[i].bIsActive = 1; 
		guestOS_thread[i].iGOSNumber = i;
	}
	// fill the entries for kernel_thread
	guestOS_thread[NUM_OF_GUESTOS].stack_start = (unsigned long *)head_stack_start;
	guestOS_thread[NUM_OF_GUESTOS].gdt_addr = (unsigned char *)(&(dynamic_gdts[0]));
	guestOS_thread[NUM_OF_GUESTOS].iCurrTaskId = 0;
	guestOS_thread[NUM_OF_GUESTOS].bIsActive = 1; 
	guestOS_thread[NUM_OF_GUESTOS].iGOSNumber = NUM_OF_GUESTOS;
	guestOS_thread[NUM_OF_GUESTOS].iBudgetRemaining = 0;
	guestOS_thread[NUM_OF_GUESTOS].pde[0] = pde;
	guestOS_thread[NUM_OF_GUESTOS].iTicks = 0;

	a = (unsigned long *)(gdt_desc + 2);
	*a = (unsigned long)(&dynamic_gdts[0]); 
	__asm__ __volatile__( "lgdt %0": "=m" (gdt_desc) );
	load_TR(0);
}
