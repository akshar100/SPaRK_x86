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

__s32 beg_header_overhead = 0;
char *main_buffer = NULL;

#ifdef SANITY_CHECK
char *START_MB, *END_MB;
#endif

/* 
   Max_size is in Kbytes, On success this function returns the free size
 */


int init_memory_pool (int max_fl_index, int max_sl_log2_index, 
		      size_t block_size, char *block_ptr) {
  __s32 n, free_mem = 0, total_size, new_size;
  block_header_t *initial_block_ptr;
  __s32 size_fl_sl_array, i, fl, sl, aux_size = 0;
  TLSF_t *ptr_TLSF;
 
  if (!block_ptr) {
    return -1;
  } 
  if (!block_size) {
    return -1;
  }
  
#ifdef SANITY_CHECK
  START_MB = (char *) block_ptr; 
  END_MB = (char *) block_ptr + (block_size * 1024);
#endif

  if (max_sl_log2_index > 5 || max_sl_log2_index < 1) {
    return -1;
  }
  
  if (max_sl_log2_index <= 0) {
    return -1;
  }

  if ((((__u32) block_ptr >> LOG2_TLSF_WORD_SIZE) << LOG2_TLSF_WORD_SIZE) 
      != (__u32) block_ptr) {
    return -1;
  }

  memset ((char *) block_ptr, 0x00, block_size * 1024);

  INIT_THREAD_MUTEX();
  ptr_TLSF = (TLSF_t *) block_ptr;
  
  ptr_TLSF -> magic_number = MAGIC_NUMBER;
  
  /* Total size of the block, TLSF_struct + free_memory */
  
  total_size = BYTES2TLSF_WORDS(block_size * 1024);

  ptr_TLSF -> max_sl_log2_index = max_sl_log2_index;
  ptr_TLSF -> max_sl_index = (1 << ptr_TLSF -> max_sl_log2_index);
  
  size_fl_sl_array = BYTES2TLSF_WORDS((__s32) sizeof (fl_array_t) +
    ((__s32) sizeof (block_header_t *) * (__s32) ptr_TLSF -> max_sl_index));

#ifdef SANITY_CHECK
  if (size_fl_sl_array > block_size * 1024)
    SANITY_PRINTF ("SANITY_CHECK error: fl_sl_array size > block_size\n");
#endif

  free_mem = total_size - BYTES2TLSF_WORDS(sizeof (TLSF_t)) - size_fl_sl_array;

  if (max_fl_index <= MIN_LOG2_SIZE + 1 || max_fl_index > MAX_FL_INDEX) {
    n = MIN_LOG2_SIZE + 1;
  
    while ((int) TLSF_WORDS2BYTES(free_mem) > 
	   (1 << (n + LOG2_TLSF_WORD_SIZE)) && n < MAX_FL_INDEX) {
      n ++;
      free_mem -= size_fl_sl_array;
    }
  } else {
    n = max_fl_index;
    free_mem -= (size_fl_sl_array * (n - 1 - MIN_LOG2_SIZE)); 
  }
  if (free_mem < 0) {
//    PRINT_MSG 
//      ("init_memory_pool() error: Initial free block is not big enought\n");
    return -1;
  }

  // max_fl_index never will be greater than 32 (4 Gbytes)
  
  ptr_TLSF -> max_fl_index = n;
  ptr_TLSF ->  TLSF_max_struct_size = BYTES2TLSF_WORDS
    (1 << (ptr_TLSF -> max_fl_index + 1 ));

  n -= MIN_LOG2_SIZE;
  
  /* max_fl_index will never be greater than MAX_FL_INDEX */
  if (ptr_TLSF -> max_fl_index < 0 || MAX_FL_INDEX < 0) return -1;

  ptr_TLSF -> fl_array = ( fl_array_t *) 
    ((__u32) &(ptr_TLSF -> fl_array) 
     + (__u32) sizeof (ptr_TLSF -> fl_array));
#ifdef SANITY_CHECK
  check_range_ptr ((char *) ptr_TLSF -> fl_array, "initialisation 1\n");
#endif 

  for (i = 0 ; i < n; i ++) {
    ptr_TLSF -> fl_array [i] .sl_array = (block_header_t **) 
      (((__s32) ptr_TLSF -> fl_array + ((__s32) sizeof (fl_array_t) * n)) +
       ((__s32) sizeof (block_header_t *) *
	(__s32) ptr_TLSF -> max_sl_index * i));
    
#ifdef SANITY_CHECK
    check_range_ptr ((char *)ptr_TLSF -> fl_array [i].sl_array, 
		     "initialisation 2\n");
#endif 
  }
  
  initial_block_ptr = (block_header_t *)  
    ((__u32 *)((__u32) ptr_TLSF -> fl_array + 
	       (TLSF_WORDS2BYTES(size_fl_sl_array) * n)) + sizeof (__u32 *));
  
  ptr_TLSF -> following_non_cont_bh = (((__u32 *) initial_block_ptr)
				       - sizeof (__u32 *));
  
  *ptr_TLSF -> following_non_cont_bh = 0;
 
  beg_header_overhead = BYTES2TLSF_WORDS((int) initial_block_ptr -> ptr.buffer
					 - (int) initial_block_ptr); 
  ptr_TLSF -> bitmapFL = 0;

  if (free_mem < MIN_SIZE) {
    return 0;
  }

#ifdef SANITY_CHECK
  initial_block_ptr -> mw = MAGIC_NUMBER;
#endif
  initial_block_ptr -> size =
    free_mem - beg_header_overhead - BYTES2TLSF_WORDS(sizeof (__u32 *)) 
	- BYTES2TLSF_WORDS(12);
  
#ifdef SANITY_CHECK
  check_range_bh (initial_block_ptr, "initialisation 1");   
  check_mn (initial_block_ptr, "initialisation 1");
#endif

  SET_LAST_BLOCK (initial_block_ptr);

  initial_block_ptr -> ptr.free_ptr.prev = NULL;
  initial_block_ptr -> ptr.free_ptr.next = NULL;
  initial_block_ptr -> prev_phys_block = NULL;
 
  if (GET_BLOCK_SIZE(initial_block_ptr) <= MIN_SIZE) {
    return 0;
  } else {
    aux_size = GET_BLOCK_SIZE(initial_block_ptr);
    if (aux_size < ptr_TLSF ->  TLSF_max_struct_size) {
      mapping_function (aux_size, &fl, &sl, &new_size, ptr_TLSF);
    } else {
      fl =  ptr_TLSF -> max_fl_index - 1;
      sl = ptr_TLSF -> max_sl_index - 1;
    }

    
    fl -= MIN_LOG2_SIZE;
  }

  initial_block_ptr -> ptr.free_ptr.first_index = fl;
  initial_block_ptr -> ptr.free_ptr.second_index = sl;
  ptr_TLSF -> fl_array [fl].sl_array [sl] = initial_block_ptr;
  TLSF__set_bit (sl, ptr_TLSF -> fl_array[fl].bitmapSL);
  TLSF__set_bit (fl, ptr_TLSF -> bitmapFL);

  return TLSF_WORDS2BYTES(GET_BLOCK_SIZE(initial_block_ptr));
}
