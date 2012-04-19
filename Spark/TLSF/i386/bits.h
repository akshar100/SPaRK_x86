/* 
 * bits.h generic arch
 *
 * Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) Feb, 2004 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

#ifndef _BITS_H_
#define _BITS_H_

static __inline__ int TLSF_fls(int x)
{
	int r;

	__asm__("bsrl %1,%0\n\t"
		"jnz 1f\n\t"
		"movl $-1,%0\n"
		"1:" : "=r" (r) : "g" (x));
	return r;
}

#endif
