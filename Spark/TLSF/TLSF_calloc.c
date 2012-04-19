/*
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 1.3
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 *
 * Thanks to Ismael Ripoll for his suggestions and reviews
 *
 * Copyright (C) April 2004 UPVLC, OCERA Consortium
 * 
 * This code is released using a dual license strategy: GPL/LGPL
 * You can choose the license that better fits your requirements.
 *
 * Released under the terms of the GNU General Public License Version 2.0
 * Released under the terms of the GNU Lesser General Public License Version 2.1
 *
 */

#include "TLSF_nondep.h"

/* see man calloc */

void *CALLOC_FUNCTION_EX (size_t nelem, size_t elem_size, char *block_ptr) {
  __u8 *p;

  if (nelem <= 0 || elem_size <= 0) return NULL;
  
  if (!(p = (__u8 *) MALLOC_FUNCTION_EX (nelem * elem_size, block_ptr)))
    return NULL;

  memset (p, 0, nelem * elem_size);
  
  return ((void *) p);
}
