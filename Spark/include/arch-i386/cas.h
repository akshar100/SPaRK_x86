/*
 * cas.h
 *
 * Written by Vicente Esteve LLoret <viesllo@inf.upv.es>
 * Copyright (C) Jul, 2003 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * Compare and Swap instruction support.
 *
 */



 /* atomic compare and exchange 32 bit value */
extern inline unsigned int cas(volatile unsigned int * dest, unsigned int cmp_val, unsigned int new_val)
{
 unsigned int tmp;

  __asm__ __volatile__ 
 (
   "cmpxchgl %1, %3 \n\t"
  : 
    "=a" (tmp)      /* 0 EAX, return val */
  : 
    "r"  (new_val), /* 1 reg, new value */
    "0"  (cmp_val), /* 2 EAX, compare value */
    "m"  (*dest)    /* 3 mem, destination operand */
  : 
    "memory", "cc" 
  );
               //Return 0 if CAS fail?
 return tmp == cmp_val;
} 

/* atomic compare and exchange 64 bit value */
extern inline unsigned char cas2(volatile unsigned long long * dest, unsigned long long cmp_val, unsigned long long new_val)
 {
   unsigned char ret;
 
  __asm__ __volatile__
    (
     "cmpxchg8b %4\n\t"
     "sete      %0\n\t"
     :
     "=q" (ret)      /* return val, 0 or 1 */
     :
     "A"  (cmp_val),
     "c"  ((unsigned int)(new_val>>32ULL)),
     "b"  (*((unsigned long *) &new_val)),
     "m"  (*dest)    /* 3 mem, destination operand */
     : 
     "memory", "cc"
     );
              //Return 0 if cas fail
   return ret;
 }
