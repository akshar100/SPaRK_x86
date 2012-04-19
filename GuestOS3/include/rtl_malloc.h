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

#ifndef _THREE_LEVELS_SEGREGATE_FIT_MALLOC_H_
#define _THREE_LEVELS_SEGREGATE_FIT_MALLOC_H_
#include <stdlib.h>

/*------------------------------------------------------------------------*/
/******************************/
/*  CONFIGURATION PARAMETERS  */
/******************************/

// Following parameters allows to tune TLSF

// When SANITY_CHECK is set  TLSF functions execute different tests to
// check TLSF structure state, is something is wrong, an error message
// will be shown on the screen, but care must be taken because sanity 
// checks get worse TLSF performance

//#define SANITY_CHECK

/* TLSF_DEBUG_FUNCTIONS allows to use all debugging functions */
//#define TLSF_DEBUG_FUNCTIONS

/* 
 * MAX_FL_INDEX defines the maximum first index which
 * will be used by TLSF. The maximum first index is
 * calculated in the init_memory_pool (size)
 * 
 * if (log2 (size) <= MAX_FL_INDEX) then
 *   max_fl_index := log2 (size);
 * else
 *   max_fl_index := MAX_FL_INDEX;
 * end if;
 *
 */

#define MAX_FL_INDEX 32 // TLSF default MAX_FL_INDEX is 16 MBytes

/* Standard functions name */
/* 
   Following macros define standard allocation/deallocation functions
 */

#define MALLOC_FUNCTION rtl_malloc
#define MALLOC_FUNCTION_EX rtl_malloc_ex

#define REALLOC_FUNCTION rtl_realloc
#define REALLOC_FUNCTION_EX rtl_realloc_ex

#define CALLOC_FUNCTION rtl_calloc
#define CALLOC_FUNCTION_EX rtl_calloc_ex

#define FREE_FUNCTION rtl_free
#define FREE_FUNCTION_EX rtl_free_ex


// Please, don't modify anything beyond this line
/*------------------------------------------------------------------------*/


/* RTLinux module */

#include <string.h>

#ifdef SANITY_CHECK
#define SANITY_PRINTF 

#endif

#define PRINT_MSG 
#define PRINT_DBG_C(message) 
#define PRINT_DBG_D(message) 
#define PRINT_DBG_F(message) 
#define PRINT_DBG_H(message) 

#define INIT_THREAD_MUTEX()

#include <rtl_sync.h>

#define THREAD_LOCK() rtl_stop_interrupts();
#define THREAD_UNLOCK() rtl_allow_interrupts();



extern char *main_buffer; // This buffer is associated with 
                          // a block of memory by the user

/*
 * associate  buffer allows  to indicate  to TLSF  that  one specific
 * buffer  must be  used by  default, this  allow to  the user  to use
 * malloc, free, calloc and realloc functions
 */

#define associate_buffer(ptr) main_buffer = (char *) ptr;

/*
 * max_sl_log2_index defines  the maximum  second index which  will be
 * used by TLSF.
 *
 * max_sl_log2_index allows  to the user to tune  the maximum internal
 * fragmentation, but  a high  max_sl_log2_index value will  cause big
 * TLSF structure.
 *
 * max_sl_log2_index  max. internal fragmentation (approximately)
 * -----------------  -------------------------------------------
 *     1                             25 %
 *     2                           12.5 %
 *     3                           6.25 %
 *     4                          3.125 %
 *     5                          1.563 %
 */

// max_size is in Kbytes
extern int init_memory_pool (int max_size, int max_sl_log2_index, 
			     size_t block_size, char *block_ptr);

extern void destroy_memory_pool (char *block_ptr);

/* see man malloc */
extern void *MALLOC_FUNCTION_EX (size_t size, char *block_ptr);

static inline void *MALLOC_FUNCTION (size_t size) {
  return (void *)MALLOC_FUNCTION_EX (size, main_buffer);
}   

/* see man realloc */
extern void *REALLOC_FUNCTION_EX (void *p, size_t new_len, char *block_ptr);

static inline void *REALLOC_FUNCTION (void *p, size_t new_len) {
  return (void *)REALLOC_FUNCTION_EX (p, new_len, main_buffer);
}

/* see man calloc */
extern void *CALLOC_FUNCTION_EX(size_t nelem, size_t elem_size, 
				char *block_ptr);

static inline void *CALLOC_FUNCTION(size_t nelem, 
				    size_t elem_size) {
  return (void *)CALLOC_FUNCTION_EX(nelem, elem_size, main_buffer);
}

/*
 * see man free
 *
 * free () is only guaranteed  to work if ptr is the address
 * of a block allocated by malloc() (and not yet freed).
 */

extern void FREE_FUNCTION_EX (void *ptr, char *block_ptr);

static inline void FREE_FUNCTION (void *ptr) {
  FREE_FUNCTION_EX (ptr, main_buffer);
}

#ifdef TLSF_DEBUG_FUNCTIONS
extern void free_blocks_status (char *block_ptr);
extern void structure_status (char *block_ptr);
extern void dump_memory_region (unsigned char *mem_ptr, unsigned int size);
#endif

#endif // #ifndef _THREE_LEVELS_SEGREGATE_FIT_MALLOC_H_
