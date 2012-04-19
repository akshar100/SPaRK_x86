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

	.globl startup_32
	
startup_32:
/*
 *	movw $0x3A, %ax
 *	movw %ax, %gs
 *	movb $67, %gs:0xB8380		# 5th line 
 *	movb $2, %gs:0xB8381
 *	movb $68, %gs:0xB8382
 *	movb $2, %gs:0xB8383
 */
	cld
	# cli "running in level 2"
	movl $0x2A,%eax
	movl %eax,%ds
	movl %eax,%es
	movl %eax,%fs
	movl %eax,%gs

	lss stack_start,%esp
	xorl %eax,%eax
	cld		# gcc2 wants the direction flag cleared at all times
                        # In this point Interrupts disable

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
	cld		# gcc2 wants the direction flag cleared at all times

        call start_kernel
stack_bottom:
        # This is the initial Stack Space
        .space 2000
stack_start:
	.long stack_start
	.long 0x2A

// #include "entry.S"
