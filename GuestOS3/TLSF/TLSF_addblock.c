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
#include "TLSF_struct_functions.h"

/* 
 * This  functions  allow  to  insert   a  new  memory  block  into  a
 * preexistent TLSF structure
 */

int add_new_block (char *new_block, int size, char *block_ptr) {
  block_header_t *new_block_ptr;
  __s32 fl, sl, new_size;
  __u32 *ptr_following;
  TLSF_t *ptr_TLSF;
  
  ptr_TLSF = (TLSF_t *) block_ptr;

  if (size < TLSF_WORDS2BYTES(beg_header_overhead)) {
//    PRINT_MSG ("add_new_block() error: New block is too much small\n");
//    PRINT_MSG ("Hint: New added blocks size must be > %d\n", 
//	       TLSF_WORDS2BYTES(beg_header_overhead));
    return -1;
  }
  
  new_block_ptr = (block_header_t *) ((__u32 *) new_block + sizeof (__u32 *));

  new_block_ptr -> prev_phys_block = NULL;

  ptr_following =  ptr_TLSF -> following_non_cont_bh;
  
  while (*ptr_following)
    ptr_following = (__u32 *) *ptr_following;
  
  // Now the new block is linked
  *ptr_following = (__u32) new_block;
  ptr_following = (__u32 *)*ptr_following;
  *ptr_following = 0;
  
  new_block_ptr -> size = 
    BYTES2TLSF_WORDS(size - sizeof (__u32 *)) - beg_header_overhead;

  SET_LAST_BLOCK (new_block_ptr);

  if (TLSF_WORDS2BYTES(GET_BLOCK_SIZE(new_block_ptr)) 
      <= MIN_SIZE) {
    return -1;
  } else {
    if (GET_BLOCK_SIZE(new_block_ptr) < ptr_TLSF ->  TLSF_max_struct_size)
      mapping_function (GET_BLOCK_SIZE(new_block_ptr), &fl, &sl, &new_size,
			ptr_TLSF);
    else {
      fl =  ptr_TLSF -> max_fl_index - 1;
      sl = ptr_TLSF -> max_sl_index - 1;
    }
    fl -= MIN_LOG2_SIZE;
  }

  init_and_insert_block (ptr_TLSF, new_block_ptr, fl, sl); 

  return TLSF_WORDS2BYTES(GET_BLOCK_SIZE(new_block_ptr));
}
