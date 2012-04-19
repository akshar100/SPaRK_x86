/*
 * rtl_tracer.h
 *
 * Written by Vicente Esteve LLoret <viesllo@inf.upv.es>
 * Copyright (C) Feb, 2003 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * RTL_TRACE macro
 */

#ifndef _RTL_TRACER_H
#define _RTL_TRACER_H

#include <rtl_conf.h>

#define TASKIN        0x1
#define INTERRUPT     0x2
#define CLISTI        0x3

#if CONFIG_RTL_TRACER

extern unsigned long long tracerbuf[CONFIG_RTL_TRACER_EVENTS];   
extern unsigned long *tracer_ptr;     
 
#define RTL_TRACE(event,value)   {                     \
       asm volatile(" rdtsc                         \n"\
	            " shll $8,%%edx                 \n"\
		    " movl %0,%%ebx                 \n"\
		    " movb %%bl,%%al                \n"\
		    " movl %1,%%ebx                 \n"\
		    " movb %%bl,%%dl                \n"\
		    " movl (%2),%%edi               \n"\
		    " movl %%eax,(%%edi)            \n"\
		    " movl %%edx,4(%%edi)           \n"\
		    " addl $0x8,%%edi               \n"\
		    " movl %%edi,(%2)               \n"\
		    " cmpl %3,%%edi                 \n"\
		    " jnz  no_overflow%=            \n"\
		    "  int  $0x3                    \n"\
		    "  movl %4,(%2)                 \n"\
		    "no_overflow%=:                 \n"\
		    ::"r" (value),                     \
		      "i" (event),                     \
		      "m" (tracer_ptr),                \
		      "i" (&tracerbuf[CONFIG_RTL_TRACER_EVENTS]), \
		      "i" (&tracerbuf[0])              \
		    : "eax","edx","ebx","edi","cc");   \
        };  
#else
#define RTL_TRACE(event,value)
#endif // CONFIG_RTL_TRACER


#endif //_RTL_TRACER_H
