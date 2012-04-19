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

typedef unsigned long page_entry;
typedef unsigned long pdeindex;
typedef unsigned long pteindex;
typedef unsigned long mprot_t;

#define X86_CR4_PSE		0x0010	/* enable page size extensions */
#define X86_CR4_PGE		0x0080	/* enable global pages */

#define PAGE_PRESENT  (0x1)
#define PAGE_RW       (0x2)
#define PAGE_USER     (0x4)
#define PAGE_ACCESSED (0x20)
#define PAGE_DIRTY    (0x40)
#define PAGE_4M       (0x80)
#define PAGE_GLOBAL   (0x100)


#define RTL_PAGE_SIZE     0x1000
#define NUM_ENTRIES_PER_PAGE	0x400	// 1024
#define RTL_PAGE_MASK     0xFFF

#define SET_CR3(v) \
 __asm__ __volatile__ ("movl %%eax,%%cr3 \n"::"a" (v));


#define SET_CR4(v) \
 asm volatile ("movl %0,%%cr4 \n"::"r" (v));

#if 1                                    //Old Version less deterministic.
#define CLEAR_WP(v) \
 asm volatile("movl %%cr0,%%eax \n" \
	      "movl %%eax,%0    \n" \
              "movl %%eax,%%ecx \n" \
	      "andl $0x80000000,%%ecx \n" \
	      "jecxz 1f          \n" \
	      "andl $0xfffeffff,%%eax \n" \
              "movl %%eax,%%cr0 \n" \
	      "1:               \n" :"=m" (v)::"eax","ecx","memory","cc");

#define RESTORE_WP(v) \
 asm volatile ("andl  $0x10000,%0    \n" \
               "xchgl %0,%%ecx       \n" \
	       "jecxz 1f             \n" \
               "movl %%cr0,%%eax     \n" \
               "orl  $0x10000,%%eax  \n" \
               "movl %%eax,%%cr0     \n" \
	       "1:                   \n"::"r" (v):"eax","ecx","memory","cc");
#endif

#define SET_WP_ON() \
 __asm ("movl %cr0,%eax    \n" \
        "orl $0x10000,%eax \n" \
	"movl %eax,%cr0    \n");

#define SET_WP_OFF() \
 __asm ("movl %cr0,%eax    \n" \
        "andl $0xfffeffff,%eax \n" \
	"movl %eax,%cr0    \n");


#define ENABLE_PAGE() \
 __asm ("movl %cr0,%eax\n" \
        "orl  $0x80000000,%eax\n" \
        "movl %eax,%cr0\n" \
        "jmp 1f        \n" \
        "1:            \n" \
	"movl $1f,%eax \n" \
	"jmp *%eax     \n" \
	"1:            \n"); 

static inline pdeindex address2pdeindex(void *address){
  return (pdeindex) (((unsigned long) address)>>22);
};

static inline pteindex address2pteindex(void *address){
  return (pteindex) ((((unsigned long) address)>>12) & 0x3ff);
};

extern page_entry pde[1024];

void init_page(void);
unsigned long GET_CR3(void);
#endif // _PAGE_H_
