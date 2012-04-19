
.include "arch/hw_irq.h"

.globl jumptoloc1
.globl jumptoloc2

Segment1 = 0x22
Segment2 = 0x32
Offset1 = 0x0
Offset2 = 0x0

.text

/*
copybinary:

        movw 	$0x18, %ax
        movw 	%ax, %gs
        movb 	$1, %gs:0xB8000
        movb 	$2, %gs:0xB8001

        movw    $0x18, %ax
        movw    %ax, %ds
#       movw    $0x18, %ax
        movw    %ax, %es
        movl    $GuestOSSize, %ecx
	movl	$SparkSize, %esi
	movl	$MEMLOCTOLOAD, %edi
        cld
        rep
        movsb
	ret
*/

jumptoloc1:
	#lcall $Segment, $ Offset
	pushl	$0x2A           #stack segment
	pushl	$100
	pushl	$Segment1
	pushl	$Offset1
	lret

jumptoloc2:
	#lcall $Segment, $ Offset
	pushl	$0x3A
	pushl	$4000100
	pushl	$Segment2
	pushl	$Offset2
	lret

