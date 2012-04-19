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
/* see man realloc */

void *REALLOC_FUNCTION_EX (void *p, size_t new_len, char *block_ptr) {
  __u32 cpsize;
  __u8 *ptr_aux;
  block_header_t *b;

#ifdef SANITY_CHECK
  check_range_ptr (block_ptr, "realloc 1\n");
  checking_structure((TLSF_t *)block_ptr, "Entering Realloc");
#endif

  if (!p)
    return (void *) MALLOC_FUNCTION_EX (new_len, block_ptr);
  else if (!new_len) {
    FREE_FUNCTION_EX (p, block_ptr);
    return NULL;
  }

  ptr_aux = (__u8 *) MALLOC_FUNCTION_EX (new_len * sizeof (__u8), block_ptr);

  b = (block_header_t *) (((__u8 *) p) - 
			  TLSF_WORDS2BYTES(beg_header_overhead));

#ifdef SANITY_CHECK
  check_range_bh (b, "realloc 1");   
  check_mn (b, "realloc 1");
#endif  

  cpsize = (TLSF_WORDS2BYTES(GET_BLOCK_SIZE(b)) > new_len)? 
    new_len : TLSF_WORDS2BYTES(GET_BLOCK_SIZE(b));

  memcpy ((__u8 *) ptr_aux, (__u8 *) p, cpsize);

  FREE_FUNCTION_EX (p, block_ptr);
#ifdef SANITY_CHECK
  checking_structure((TLSF_t *)block_ptr, "Leaving Realloc");
#endif
  return ((void *) ptr_aux);
}
