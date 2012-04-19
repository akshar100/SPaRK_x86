/*
 *  linux/boot/head.S
 *
 *  Copyright (C) 1991, 1992, 1993  Linus Torvalds
 */


/*
 *  head.S contains the 32-bit startup code.
 *
 * NOTE!!! Startup happens at absolute address 0x00001000, which is also where
 * the page directory will exist. The startup code will be overwritten by
 * the page directory. [According to comments etc elsewhere on a compressed
 * kernel it will end up at 0x1000 + 1Mb I hope so as I assume this. - AC]
 *
 * Page 0 is deliberately kept safe, since System Management Mode code in 
 * laptops may need to access the BIOS data stored there.  This is also
 * useful for future device drivers that either access the BIOS via VM86 
 * mode.
 */

/*
 * High loaded stuff by Hans Lermen & Werner Almesberger, Feb. 1996
 */
.text

/* #include <linux/linkage.h> */
#include <asm/segment.h>

	.globl startup_32
	
startup_32:
	cld
	cli
	movl $0x18,%eax
	movl %eax,%ds
	movl %eax,%es
	movl %eax,%fs
	movl %eax,%gs
	
	lss head_stack_start,%esp
	xorl %eax,%eax
1:	incl %eax		# check that A20 really IS enabled
	movl %eax,0x000000	# loop forever if it isn't
	cmpl %eax,0x100000
	je 1b

/*
 * Initialize eflags.  Some BIOS's leave bits like NT set.  This would
 * confuse the debugger if this code is traced.
 * XXX - best to initialize before switching to protected mode.
 */
	pushl $0
	popfl
/*
 * Clear BSS
 */
	xorl %eax,%eax
	movl $ _edata,%edi
	movl $ _end,%ecx
	subl %edi,%ecx
	cld
	rep
	stosb

        xorl %eax,%eax
	lldt %ax
	cld		# gcc2 wants the direction flag cleared at all times

                        # In this point Interrupts disable 
        call start_kernel

.globl gdt_desc
.globl head_stack_start
#.globl gdt1

.align 4,0x90
gdt_desc:
	.word	10*8 - 1
	.long	gdt1
		
.align 4,0x90

gdt1:

stack_bottom:
        # This is the initial Stack Space
        .space 2000
head_stack_start:
	.long head_stack_start
	.long 0x18

