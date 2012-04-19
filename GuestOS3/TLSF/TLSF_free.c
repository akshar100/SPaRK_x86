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
 * see man free
 *
 * free () is only guaranteed  to work if ptr is the address
 * of a block allocated by rt_malloc() (and not yet freed).
 */
 
void FREE_FUNCTION_EX (void *ptr, char *block_ptr) {
  __s32 aux_size, fl, sl, new_size;
  block_header_t *bh, *bh2, *bh3;

  TLSF_t *ptr_TLSF;

  ptr_TLSF = (TLSF_t *) block_ptr;

#ifdef SANITY_CHECK
  checking_structure (ptr_TLSF, "Entering free");
  check_range_ptr (block_ptr, "malloc 1");
#endif

  if (!ptr_TLSF || ptr_TLSF -> magic_number != MAGIC_NUMBER) {
//    PRINT_MSG ("free() error: TLSF structure is not initialized\n");
//    PRINT_MSG
//      ("Hint: Execute init_memory_pool() before calling free()");
    return;
  }

  bh = (block_header_t *) ((__u8 *) ptr - 
			   TLSF_WORDS2BYTES(beg_header_overhead));
 
  THREAD_LOCK();
  /* now bh is a free block */
  
  SET_FREE_BLOCK (bh);
  bh -> ptr.free_ptr.prev = NULL;
  bh -> ptr.free_ptr.next = NULL;

  /*
   * first of all, we will try to merge bh with the
   * physically contiguos free block and
   * after we will inserte bh into TLSF structure
   */
  
  /* Now we will try if we can merge bh with the next phys. contiguos block */
  if (!IS_LAST_BLOCK (bh)) {
    /* is it the next block free? */
    /* The next block is easy to found */
    
    bh2 = (block_header_t *) (bh -> ptr.buffer + 
			      TLSF_WORDS2BYTES
			       (GET_BLOCK_SIZE(bh)));

#ifdef SANITY_CHECK
    check_range_bh (bh2, "free 1");   
    check_mn (bh2, "free 1");
#endif

   
    if (!IS_USED_BLOCK (bh2)) {
      /* we are lucky, we can merge bh with the following one */

      remove_block (bh2, ptr_TLSF);
      
      bh -> size += bh2 -> size + beg_header_overhead;
      
      if (!IS_LAST_BLOCK (bh2)) {
	bh3 = (block_header_t *) (bh2 -> ptr.buffer + 
				  TLSF_WORDS2BYTES
				  (GET_BLOCK_SIZE(bh2)));
	bh3 -> prev_phys_block = bh;
      }
    } 
  }
  
  /* is it free the previous physical block? */
  if (bh -> prev_phys_block) { // This block is not the first block
   
    bh2 = bh -> prev_phys_block;
#ifdef SANITY_CHECK
    check_range_bh (bh2, "free 2");   
    check_mn (bh2, "free 2");
#endif  
    if (!IS_USED_BLOCK (bh2)) {
  
      remove_block (bh2, ptr_TLSF);

      bh2 -> size += bh -> size + beg_header_overhead;
      
      bh = bh2;
      
      if (!IS_LAST_BLOCK (bh)) {
	bh3 = (block_header_t *) (bh -> ptr.buffer + 
				  TLSF_WORDS2BYTES
				  (GET_BLOCK_SIZE(bh)));
	bh3 -> prev_phys_block = bh;
	
#ifdef SANITY_CHECK
	check_range_bh (bh3, "free 3");   
	check_mn (bh3, "free 3");
#endif  
      } 
    }
  }
  
  /*
   * and now we can merge the free block with the initial memory
   */
  aux_size = GET_BLOCK_SIZE(bh);
  
  if (aux_size < ptr_TLSF ->  TLSF_max_struct_size) {
    mapping_function (aux_size, &fl, &sl, &new_size, ptr_TLSF);
#ifdef SANITY_CHECK
    check_fl_sl(fl, sl, ptr_TLSF, "free 1");
#endif
  } else {
    fl =  ptr_TLSF -> max_fl_index - 1;
    sl = ptr_TLSF -> max_sl_index - 1;
  }
  
  fl -= MIN_LOG2_SIZE;

#ifdef SANITY_CHECK
  check_fl_sl_2 (fl, sl, ptr_TLSF, "free 2");
#endif

  init_and_insert_block (ptr_TLSF, bh, fl, sl); 

  THREAD_UNLOCK();

#ifdef SANITY_CHECK
  checking_structure (ptr_TLSF, "Leaving free");
#endif
}
