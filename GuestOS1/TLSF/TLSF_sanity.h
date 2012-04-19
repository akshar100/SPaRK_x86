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

#ifndef _SANITY_CHECK_H_
#define _SANITY_CHECK_H_

#ifdef SANITY_CHECK

#ifdef SANITY_CHECK
extern char *START_MB, *END_MB;
#endif


static inline int check_range_ptr (char *ptr_TLSF, char *msg) {
  if ((char *)ptr_TLSF < START_MB || (char *)ptr_TLSF > END_MB) {
    SANITY_PRINTF ("SANITY_CHECK error (ptr is out of range): %s\n", msg);
    return 1;
  }
  return 0;
}

static inline int check_fl_sl (int fl, int sl, 
				TLSF_t *ptr_TLSF, char *msg) {
  if (fl < 0 || fl > ptr_TLSF -> max_fl_index ||
      sl < 0 || sl > ptr_TLSF -> max_sl_index) {
    SANITY_PRINTF ("SANITY_CHECK error ([fl] %d [sl] %d): %s\n", fl, sl, msg);
    return 1;
  }
  return 0;
}

static inline int check_fl_sl_2 (int fl, int sl, 
				  TLSF_t *ptr_TLSF, char *msg) {
  if (fl < 0 || fl > ptr_TLSF -> max_fl_index - MIN_LOG2_SIZE ||
      sl < 0 || sl > ptr_TLSF -> max_sl_index) {
    SANITY_PRINTF ("SANITY_CHECK error ([fl] %d [sl] %d): %s\n", fl, sl, msg);
    return 1;
  }
  return 0;
}

static inline int check_mn(block_header_t *bh, char *msg) {
  if (bh -> mw != MAGIC_NUMBER) {
    SANITY_PRINTF ("SANITY_CHECK error (corrupted block): %s\n", msg);
    return 1;
  }
  return 0;
}

static inline int check_range_bh (block_header_t *bh, char *msg) {
  if ((char *) bh < START_MB || (char *) bh > END_MB) {
    SANITY_PRINTF ("SANITY_CHECK error (bh is out of range): %s\n", msg);
    SANITY_PRINTF ("S_C range: [0x%x - 0x%x] block 0x%x\n", (int)START_MB,
		   (int)END_MB, (int)bh);
    return 1;
  }

  if (((char *) bh) + TLSF_WORDS2BYTES(GET_BLOCK_SIZE(bh)) < START_MB  ||
      ((char *) bh) + TLSF_WORDS2BYTES(GET_BLOCK_SIZE(bh)) > END_MB) {
    SANITY_PRINTF ("S_C block size %d\n", GET_BLOCK_SIZE(bh));
    return 1;
  }
  return 0;
}

static inline int checking_structure(TLSF_t *ptr, char *msg) {
  int i, j;
  block_header_t *b;
  __u32 *ptr_following;
  int end;
  if (check_range_ptr ((char *)ptr, "checking structure\n")) {
    SANITY_PRINTF("%s\n",msg);
    return 1;
  }
  for (i = 0; i < ptr -> max_fl_index - MIN_LOG2_SIZE; i++)
    for (j = 0; j < ptr -> max_sl_index; j ++) {
      if (check_range_ptr ((char *)ptr -> fl_array [i].sl_array, 
			   "checking_structure")) {
	SANITY_PRINTF("%s\n",msg);
	return 1;
      }
      if (ptr -> fl_array [i].sl_array [j] != NULL) {
	b = ptr -> fl_array [i].sl_array[j];
	// First we check free blocks
	while (b != NULL) {
	  if (check_range_bh (b, "checking free blocks") || 
	      check_mn (b, "checking free blocks")) {
	    SANITY_PRINTF("%s\n",msg);
	    return 1;
	  }
	  b = b -> ptr.free_ptr.next;
	}
      }
    }
  
  // OK, now are checked physical contigous blocks
  ptr_following = ptr -> following_non_cont_bh; 
  b = (block_header_t *) (ptr_following + sizeof (__u32 *));
  end = 0;
  while (!end) {
    if (check_range_bh (b, "checking phys-cont blocks") ||
	check_mn (b, "checking phys-cont blocks")) {
      SANITY_PRINTF("%s\n",msg);
      return 1;
    }
    if (IS_LAST_BLOCK(b))
      end = 1;
    else
      b = (block_header_t *) (b -> ptr.buffer +
			      TLSF_WORDS2BYTES (GET_BLOCK_SIZE(b)));
  }
  return 0;
}

#endif
#endif
