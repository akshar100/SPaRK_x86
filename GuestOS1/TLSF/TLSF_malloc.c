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
// #include <hypercall.h>

/* see man malloc */
/*
 * malloc searchs a free block of size 'size',
 * after the free block will be splitted in two new blocks,
 * one of these new blocks will be given to the user and the
 * other will be inserted into TLSF structure
 *
 * The cost of this operation is
 *      best case: (K) = (1)
 *      worst case: (MAX_FL_LOG2_INDEX - MIN_FL_LOG2_INDEX + MAX_SL_INDEX + K)
 *                   = (1)
 * where K is a constant integer
 */

void *MALLOC_FUNCTION_EX (size_t size, char *block_ptr) {
  TLSF_t *ptr_TLSF; 
#ifdef SANITY_CHECK
  __u32 req_size = size;
#endif
  __s32 fl, sl;
  __u32 old_size, last_block, aux_size, new_size;
  block_header_t *bh, *bh2, *bh3;
 
  // spark_print("Inside malloc\n");
 
  ptr_TLSF = (TLSF_t *) block_ptr;

#ifdef SANITY_CHECK
  checking_structure(ptr_TLSF, "Entering Malloc");
  check_range_ptr (block_ptr, "malloc 1");
#endif

  if (!ptr_TLSF || ptr_TLSF -> magic_number != MAGIC_NUMBER) {
//    PRINT_MSG ("malloc() error: TLSF structure is not initialized\n");
//    PRINT_MSG
//      ("Hint: Execute init_memory_pool() before calling malloc()");
    return NULL;
  }
  
  if (!size) {
//    PRINT_MSG ("malloc() error: requested size must be > 0\n");
    return NULL;
  }

  // Requested size must be translated in TLSF_WORDS
  old_size = BYTES2TLSF_WORDS(size);
  
  if (old_size < MIN_SIZE) {
    size = MIN_SIZE;
    fl = 0;
    sl = 0;
  } else {
 
    mapping_function (old_size, &fl, &sl, &size, ptr_TLSF);
#ifdef SANITY_CHECK
    check_fl_sl (fl, sl, ptr_TLSF, "malloc 1");
#endif

    if (++sl == ptr_TLSF -> max_sl_index) {
      fl ++;
      sl = 0;
    }
    
    /*
     * This is the reason of the internal fragmentation
     * The block given is greater that the asked for size
     */
    
    // The TLSF structure begins indexing size on MIN_LOG2_SIZE
    fl -= MIN_LOG2_SIZE;
    
  }

#ifdef SANITY_CHECK
  if (req_size > TLSF_WORDS2BYTES(size)) {
    SANITY_PRINTF("SANITY error: resquested %d given %d\n", req_size,
		  TLSF_WORDS2BYTES(size));
  }
  check_fl_sl_2 (fl, sl, ptr_TLSF, "malloc 2");
#endif

  /*----------------------------------------*/
  /* The search for a free block begins now */
  /*----------------------------------------*/
  
  /*
   * Our first try, we take the first free block 
   * from fl_array or its buddy
   */
  
  THREAD_LOCK();

  sl = ptr_TLSF -> fl_array[fl].bitmapSL & ((~0) << sl);
  if (sl != 0) {
    sl = TLSF_fls(sl);
#ifdef SANITY_CHECK
    check_fl_sl_2 (fl, sl, ptr_TLSF, "malloc 3");
#endif
    goto found;
  }
 
  /*
   * On the last case a free block is looked for using the bitmaps
   */
  fl = TLSF_fls(ptr_TLSF -> bitmapFL & ((~0) << (fl + 1)));
  
  if (fl > 0) {
    sl = TLSF_fls(ptr_TLSF -> fl_array[fl].bitmapSL);
#ifdef SANITY_CHECK
    check_fl_sl_2 (fl, sl, ptr_TLSF, "malloc 4");
#endif
    goto found;
  }
  
  /*
   * HUGGGG, NOT ENOUGHT MEMORY
   * I think that we have done all that we have been able, I'm sorry
   */
  
  THREAD_UNLOCK();
//  PRINT_MSG ("malloc() error: Memory pool exhausted!!!\n");
//  PRINT_MSG ("Hint: You can add memory through add_new_block()\n");
//  PRINT_MSG ("Hint: However this is not a real-time guaranteed way\n");
  
  return NULL;

  /* end of the search */
  /*------------------------------------------------------------*/
  

  /*
   * we can say: YESSSSSSSSSSS, we have enought memory!!!!
   */
  
 found:
  bh = ptr_TLSF -> fl_array [fl].sl_array [sl];

#ifdef SANITY_CHECK
  check_range_bh (bh, "malloc 1");
  check_mn (bh, "malloc 1");
#endif    

  ptr_TLSF -> fl_array [fl].sl_array [sl] = bh -> ptr.free_ptr.next;
#ifdef SANITY_CHECK
  bh3 = ptr_TLSF -> fl_array[fl].sl_array[sl];
  if (bh3 != NULL) {
    check_range_bh (bh3, "malloc 2");
    check_mn (bh3, "malloc 2");
  }
#endif

  if (ptr_TLSF -> fl_array [fl].sl_array [sl]){
    ptr_TLSF -> fl_array [fl].sl_array [sl] -> ptr.free_ptr.prev = NULL;
  } else {
    TLSF__clear_bit (sl, ptr_TLSF -> fl_array[fl].bitmapSL);
    if (!ptr_TLSF -> fl_array[fl].bitmapSL)
      TLSF__clear_bit (fl, ptr_TLSF -> bitmapFL);
  }
  
  /* can bh be splitted? */ 
  
  new_size = (int)(GET_BLOCK_SIZE(bh) - size -  beg_header_overhead);
  /* The result of the substraction, may be negative... but new_size is unsigned */
  if ((int) new_size >= (int) MIN_SIZE) {
    /*
     * Yes, bh will be splitted into two blocks
     */
    
    /* The new block will begin at the end of the current block */
    /* */
    last_block = IS_LAST_BLOCK(bh)?1:0;
    bh -> size = size;
    SET_USED_BLOCK(bh);
    
    bh2 = (block_header_t *) (bh -> ptr.buffer + 
			      TLSF_WORDS2BYTES
			      (GET_BLOCK_SIZE(bh)));
#ifdef SANITY_CHECK
    bh2 -> mw = MAGIC_NUMBER;
#endif
    
    bh2 -> prev_phys_block = bh;
    bh2 -> size = new_size;

    if (last_block) SET_LAST_BLOCK (bh2);
    
    //aux_size = GET_BLOCK_SIZE(bh2);
   
    if (new_size < ptr_TLSF ->  TLSF_max_struct_size) {
      mapping_function (new_size, &fl, &sl, &aux_size, ptr_TLSF);
#ifdef SANITY_CHECK
      check_fl_sl (fl, sl, ptr_TLSF, "malloc 5");
#endif
    } else {
      fl =  ptr_TLSF -> max_fl_index - 1;
      sl = ptr_TLSF -> max_sl_index - 1;
    }
 
    fl -= MIN_LOG2_SIZE;
#ifdef SANITY_CHECK
    check_fl_sl_2 (fl, sl, ptr_TLSF, "malloc 6");
#endif
    init_and_insert_block (ptr_TLSF, bh2, fl, sl);

#ifdef SANITY_CHECK
    check_range_bh (bh2, "malloc 3");
    check_mn (bh2, "malloc 3");
#endif

    if (!last_block) {
      bh3 = (block_header_t *) (bh2 -> ptr.buffer + 
				TLSF_WORDS2BYTES(new_size));

      bh3 -> prev_phys_block = bh2;

#ifdef SANITY_CHECK
      check_range_bh (bh3, "malloc 4");
      check_mn (bh3, "malloc 4");
#endif  
    }
  }
  
  SET_USED_BLOCK(bh);
  
  THREAD_UNLOCK();

#ifdef SANITY_CHECK
  checking_structure (ptr_TLSF, "Leaving Malloc");
#endif

  // spark_print("Leaving malloc\n");
  return (void *)  bh -> ptr.buffer; 
}
