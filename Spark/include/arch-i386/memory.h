/*
 * memory.h
 *
 * Written by Vicente Esteve LLoret <viesllo@inf.upv.es>
 * Copyright (C) Feb, 2003 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * memory managment.
 *
 */


#ifndef _MEMORY_H_
#define _MEMORY_H_

#define GFP_KERNEL 0
#define GFP_ATOMIC 0

#define	KERNEL_HEAP_SIZE	10*1024*1024	// 10 MB

// char *kmalloc(unsigned int size,int type);
// unsigned long GetMemorySize(void);
void init_memory(void);

#endif //_MEMORY_H_

