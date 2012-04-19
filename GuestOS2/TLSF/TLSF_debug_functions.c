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
#ifdef TLSF_DEBUG_FUNCTIONS

static void print_block (block_header_t *b){
  if (!b) return;

  PRINT_DBG_C (">>>> Address 0x");
  PRINT_DBG_H (b);
  
  if ((b -> size & USED_BLOCK) != USED_BLOCK)
    PRINT_DBG_C ("\n>>>> Status FREE");
  else
    PRINT_DBG_C ("\n>>>> Status USED");
  
  PRINT_DBG_C (" Block Size ");
  PRINT_DBG_D (TLSF_WORDS2BYTES(b -> size));
  PRINT_DBG_C (" bytes");
  
  if (!b -> prev_phys_block)
    PRINT_DBG_C ("\n>>>> FIRST BLOCK");
  else {
    PRINT_DBG_C ("\n>>>> PREV. PHYS. BLOCK 0x");
    PRINT_DBG_H (b -> prev_phys_block);
  }

  if ((b -> size & LAST_BLOCK) == LAST_BLOCK)
    PRINT_DBG_C (" LAST BLOCK");
  else
    PRINT_DBG_C (" NOT LAST BLOCK");
  
  if ((b -> size & FREE_BLOCK) == FREE_BLOCK){
    PRINT_DBG_C ("\n---- Prev Free 0x");
    PRINT_DBG_H (b -> ptr.free_ptr.prev);
    PRINT_DBG_C (" Next Free 0x");
    PRINT_DBG_H (b -> ptr.free_ptr.next);
  }
  PRINT_DBG_C ("\n\n");
}

/*
 * structure_status () shows the status of all the blocks
 */
void structure_status (char *block_ptr) {
  block_header_t *b;
  TLSF_t *ptr_TLSF;
  int end = 0, end2 = 0;
  __u32 *ptr_following;
  
  ptr_TLSF = (TLSF_t *) block_ptr; 
  if (!ptr_TLSF || ptr_TLSF -> magic_number != MAGIC_NUMBER) {
    PRINT_MSG 
      ("structure_status() error: TLSF structure is not initialized\n");
    PRINT_MSG 
      ("Hint: Execute init_memory_pool() before calling structure_status()");
      return;
  }

  PRINT_DBG_C ("\nTLSF structure address 0x");
  PRINT_DBG_H (ptr_TLSF);
  PRINT_DBG_C ("\nMax. first level index: ");
  PRINT_DBG_D (ptr_TLSF -> max_fl_index);
  PRINT_DBG_C ("\nMax. second level index: ");
  PRINT_DBG_D (ptr_TLSF -> max_sl_index);
  PRINT_DBG_C ("\n\nALL BLOCKS\n");
  
  ptr_following = ptr_TLSF -> following_non_cont_bh;
  while (!end2) {
    end = 0;
    b = (block_header_t *) (ptr_following + sizeof (__u32 *));
    
    while (!end) {
      print_block (b);
      if (IS_LAST_BLOCK(b))
	end = 1;
      else
	b = (block_header_t *) (b -> ptr.buffer + 
			        TLSF_WORDS2BYTES (GET_BLOCK_SIZE(b)));
    }
    if (!(__u32 *) *ptr_following) 
      end2 = 1;
    else {
      ptr_following = (__u32 *) *ptr_following;
    }    
  }
}

/*
 * free_blocks_status () only shows the status
 * of the free blocks
 */
void free_blocks_status (char *block_ptr){
  int i, j;
  block_header_t *b;

  TLSF_t *ptr_TLSF;
  
  ptr_TLSF = (TLSF_t *) block_ptr; 
  if (!ptr_TLSF || ptr_TLSF -> magic_number != MAGIC_NUMBER) {
    PRINT_MSG 
      ("free_blocks_status() error: TLSF structure is not initialized\n");
    PRINT_MSG 
      ("Hint: Execute init_memory_pool() before calling free_blocks_status()");

    return;
  }

  PRINT_DBG_C ("\nTLSF structure address 0x");
  PRINT_DBG_H (ptr_TLSF);
  PRINT_DBG_C ("\nFREE BLOCKS\n\n");
  for (i = ptr_TLSF -> max_fl_index  - 1 - MIN_LOG2_SIZE; i >= 0; i--) {
    if (ptr_TLSF -> fl_array [i].bitmapSL > 0)
      for (j = ptr_TLSF -> max_sl_index - 1; j >= 0; j--) {
        if (ptr_TLSF -> fl_array [i].sl_array[j]) {
          b = ptr_TLSF -> fl_array [i].sl_array [j];
          PRINT_DBG_C ("[");
          PRINT_DBG_D (i + MIN_LOG2_SIZE);
          PRINT_DBG_C ("] ");
          PRINT_DBG_D (TLSF_WORDS2BYTES(1 << (i + MIN_LOG2_SIZE)));
	  PRINT_DBG_C (" bytes -> Free blocks: 0x");
	  PRINT_DBG_H (ptr_TLSF -> fl_array [i].bitmapSL);
          PRINT_DBG_C ("\n");
	  
          while (b) {
	    PRINT_DBG_C (">>>> First_Level [");
	    PRINT_DBG_D (i + MIN_LOG2_SIZE);
	    PRINT_DBG_C ("] Second Level [");
	    PRINT_DBG_D (j);
	    PRINT_DBG_C ("] -> ");
	    PRINT_DBG_D (TLSF_WORDS2BYTES((1 << (i + MIN_LOG2_SIZE)) + 
			  ( ((1 << (i + MIN_LOG2_SIZE)) / 
			     ptr_TLSF -> max_sl_index) * j)));

            PRINT_DBG_C (" bytes\n");
            print_block (b);
            b = b -> ptr.free_ptr.next;
          }
        }
      }
  }
}

void dump_memory_region (unsigned char *mem_ptr, unsigned int size) {
  
  unsigned int begin = (unsigned int) mem_ptr;
  unsigned int end = (unsigned int) mem_ptr + size;
  int column = 0;

  begin >>= 2;
  begin <<= 2;
  
  end >>= 2;
  end ++;
  end <<= 2;
  
  PRINT_DBG_C ("\nMemory region dumped: 0x");
  PRINT_DBG_H (begin);
  PRINT_DBG_C (" - ");
  PRINT_DBG_H (end);
  PRINT_DBG_C ("\n\n");
  
  column = 0;
  PRINT_DBG_C ("\t\t+0");
  PRINT_DBG_C ("\t+1");
  PRINT_DBG_C ("\t+2");
  PRINT_DBG_C ("\t+3");
  PRINT_DBG_C ("\t+4");
  PRINT_DBG_C ("\t+5");
  PRINT_DBG_C ("\n0x");
  PRINT_DBG_H (begin);
  PRINT_DBG_C ("\t");

  while (begin < end) {
    if (((unsigned char *) begin) [0] == 0)
      PRINT_DBG_C ("00");
    else
      PRINT_DBG_H (((unsigned char *) begin) [0]);
    if (((unsigned char *) begin) [1] == 0)
      PRINT_DBG_C ("00");
    else
      PRINT_DBG_H (((unsigned char *) begin) [1]);
    PRINT_DBG_C ("\t");
    begin += 2;
    column ++;
    if (column == 6) {
      PRINT_DBG_C ("\n0x");
      PRINT_DBG_H (begin);
      PRINT_DBG_C ("\t");
      column = 0;
    }

  }
  PRINT_DBG_C ("\n\n"); 
}

#endif
