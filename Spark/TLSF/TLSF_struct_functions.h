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

#ifndef _TLSF_STRUCT_FUNCTIONS_H_
#define _TLSF_STRUCT_FUNCTIONS_H_

#include "TLSF_nondep.h"

/* This code  is used to initialise  and to insert a  new block header
   into  the TLSF  structure by  rtl_malloc,  rtl_free, add_free_block
   functions
*/

static inline void init_and_insert_block (TLSF_t *ptr_TLSF, 
					  block_header_t *bh, 
					  int fl, int sl) {
  bh -> ptr.free_ptr.first_index = fl;
  bh -> ptr.free_ptr.second_index = sl;
  bh -> ptr.free_ptr.prev = NULL;
  bh -> ptr.free_ptr.next = ptr_TLSF -> fl_array [fl].sl_array [sl];
  
  if (ptr_TLSF -> fl_array [fl].sl_array [sl]) {
#ifdef SANITY_CHECK
    check_range_bh (bh -> ptr.free_ptr.next, "i_i 1");
    check_mn (bh -> ptr.free_ptr.next, "i_i 1");
#endif
    ptr_TLSF -> fl_array [fl].sl_array [sl] -> ptr.free_ptr.prev = bh;
  }
  ptr_TLSF -> fl_array [fl].sl_array [sl] = bh;
#ifdef SANITY_CHECK
  check_range_bh (bh, "i_i 1");
  check_mn (bh, "i_i 1");
#endif
  
  TLSF__set_bit (sl, ptr_TLSF -> fl_array[fl].bitmapSL);
  TLSF__set_bit (fl, ptr_TLSF -> bitmapFL);
}

/* Following function merges a given block with the TLSF structure */
static inline void remove_block (block_header_t *bh2, TLSF_t *ptr_TLSF) {
  
  __s32 fl, sl;

  /* we are lucky, we can merge bh with the following one */  
  if (bh2 -> ptr.free_ptr.next)
    bh2 -> ptr.free_ptr.next -> ptr.free_ptr.prev =
      bh2  -> ptr.free_ptr.prev;
  
  if (bh2 -> ptr.free_ptr.prev)
    bh2 -> ptr.free_ptr.prev -> ptr.free_ptr.next =
      bh2 -> ptr.free_ptr.next;
  
  fl = bh2 -> ptr.free_ptr.first_index;
  sl = bh2 -> ptr.free_ptr.second_index;
  
#ifdef SANITY_CHECK
  check_fl_sl_2 (fl, sl, ptr_TLSF, "r_b 1");
#endif
  
  /* bh2 must be deleted from fl_array */
  if (ptr_TLSF -> fl_array [fl].sl_array [sl] == bh2) {
    ptr_TLSF -> fl_array [fl].sl_array [sl] = bh2 -> ptr.free_ptr.next;
#ifdef SANITY_CHECK
    if (ptr_TLSF -> fl_array [fl].sl_array [sl]) {
      check_range_bh (ptr_TLSF -> fl_array [fl].sl_array [sl], "r_b 1");
      check_mn (ptr_TLSF -> fl_array [fl].sl_array [sl], "r_b 1");
    }
#endif
  }
  if (!ptr_TLSF -> fl_array [fl].sl_array [sl]) {
    TLSF__clear_bit (sl, ptr_TLSF -> fl_array[fl].bitmapSL);
    if (!ptr_TLSF -> fl_array[fl].bitmapSL)
      TLSF__clear_bit (fl, ptr_TLSF -> bitmapFL);
  }
}

#endif
