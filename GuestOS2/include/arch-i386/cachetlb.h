/*
 * cachetlb.h
 *
 * Written by Vicente Esteve LLoret <viesllo@inf.upv.es>
 * Copyright (C) Feb, 2003 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * cache and tlb managment
 */




#ifndef _RTLINUX_CACHETLB_H
#define _RTLINUX_CACHETLB_H


#define DISABLEPGE __asm("movl %cr4,%eax   \n" \
		      "andl $0xffffff7f,%eax \n" \
		      "movl %eax,%cr4 \n" \
		      ); 
#define ENABLEPGE __asm("movl %cr4,%eax   \n" \
             	      "orl $0x80,%eax \n" \
		      "movl %eax,%cr4 \n" \
		      ); 
#define DISABLECACHE __asm("movl %cr0,%eax   \n" \
		      "orl $0x60000000,%eax \n" \
		      "movl %eax,%cr0\n" \
		      ); 
#define ENABLECACHE __asm("movl %cr0,%eax   \n" \
             	      "andl $0x9fffffff,%eax \n" \
		      "movl %eax,%cr0 \n" \
		      );
#define INVALIDATLB __asm("movl %cr3,%eax\n" \
		            "movl %eax,%cr3\n");
#define INVALIDATLBALL DISABLEPGE; \
		       INVALIDATLB; \
                       ENABLEPGE;
#define INVALIDACACHE   __asm("wbinvd \n");
#define CACHEINVALIDATE __asm("wbinvd \n");

#endif
