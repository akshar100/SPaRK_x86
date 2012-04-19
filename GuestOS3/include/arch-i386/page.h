/*
 * page.h
 *
 * Written by Vicente Esteve LLoret <viesllo@inf.upv.es>
 * Copyright (C) Jul, 2003 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * memory protection.
 *
 */

#ifndef _PAGE_H_
#define _PAGE_H_

#include <arch/segment.h>
#include <rtl_sched.h>

typedef unsigned long page_entry;
typedef unsigned long pdeindex;
typedef unsigned long pteindex;
typedef unsigned long mprot_t;

#define X86_CR4_PSE             0x0010  /* enable page size extensions */
#define X86_CR4_PGE             0x0080  /* enable global pages */

#define PAGE_PRESENT  (0x1)
#define PAGE_RW       (0x2)
#define PAGE_USER     (0x4)
#define PAGE_ACCESSED (0x20)
#define PAGE_DIRTY    (0x40)
#define PAGE_4M       (0x80)
#define PAGE_GLOBAL   (0x100)

#define RTL_PAGE_SIZE     0x1000
#define NUM_ENTRIES_PER_PAGE    1024
#define RTL_PAGE_MASK     0xFFF

#define TOT_SPACE_GUEST_OS 0x2000000   // 32MB
#define SPACE_PTE_PTE	   0x400000	// 4 MB
#define TOT_PTE_REQ	TOT_SPACE_GUEST_OS/SPACE_PTE_PTE	   // 8 entries

/*
struct task_data {
	page_entry *cr3;
	int *esp2;
};
*/
//extern pthread_t tcb[];


static inline pdeindex address2pdeindex(void *address){
  return (pdeindex) (((unsigned long) address)>>22);
};

static inline pteindex address2pteindex(void *address){
  return (pteindex) ((((unsigned long) address)>>12) & 0x3ff);
};


void init_page(void);

#endif // _PAGE_H_


