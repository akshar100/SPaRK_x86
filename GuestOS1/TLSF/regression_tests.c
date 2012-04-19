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
#include "arch/msr.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_LEN 45

static int ncolors = 0;
static int failed = 0, passed = 0;

static void test_result_bool (char *title, int res) {
  int length, n;
  char buffer [MAX_LEN + 1];

  strncpy (buffer, title, MAX_LEN);
  buffer[MAX_LEN] = 0;
  fprintf (stderr, "%s ", buffer);
  
  length = strlen (buffer);
  for (n = length; n < MAX_LEN + 3; n++)
    fprintf (stderr, ".");
  
  fprintf (stderr, " [");
  
  if (res) {
    passed ++;
    if (ncolors)
      fprintf (stderr, "PASSED");
    else
      fprintf (stderr, "\E[32mPASSED\E[0m");
  } else {
    failed ++;
    if (ncolors)
      fprintf (stderr, "FAILED");
    else
      fprintf (stderr, "\E[31mFAILED\E[0m");
  }
  fprintf (stderr, "]\n");
}

static void test_result_int (char *title, int res, char *unit) {
  int length, n;
  char buffer [MAX_LEN + 1];

  strncpy (buffer, title, MAX_LEN);
  buffer[MAX_LEN] = 0;
  fprintf (stderr, "%s ", buffer);
  
  length = strlen (buffer);
  for (n = length; n < MAX_LEN + 3; n++)
    fprintf (stderr, ".");

  if (ncolors)
    fprintf (stderr, " %d", res);
  else
    fprintf (stderr, " \E[34m%d\E[0m", res);
  if (unit)
    fprintf (stderr, " %s\n", unit);
}

static void test_result_ll (char *title, unsigned long long res, char *unit) {
  int length, n;
  char buffer [MAX_LEN + 1];

  strncpy (buffer, title, MAX_LEN);
  buffer[MAX_LEN] = 0;
  fprintf (stderr, "%s ", buffer);
  
  length = strlen (buffer);
  for (n = length; n < MAX_LEN + 3; n++)
    fprintf (stderr, ".");

  if (ncolors)
    fprintf (stderr, " %llu", res);
  else
    fprintf (stderr, " \E[34m%llu\E[0m", res);
  if (unit)
    fprintf (stderr, " %s\n", unit);
}


/* This first test checks correct TLSF structure initialisation */
static int initialisation_tests (void) {
  unsigned char memory_pool[1024 * 1024];
  unsigned char *ptr = NULL, buffer[256];
  int mem_alloc, n , sli, total, max, min, m;
  int ext_frag_max, ext_frag_average;
  unsigned long long average;

  fprintf (stderr, "\n>>> INITIALISATION tests <<<\n\n");
  
  test_result_bool ("Normal initialisation", 
	       init_memory_pool (0, 5, 10, memory_pool) > 0);
  test_result_bool ("No memory pool provideed", 
	       init_memory_pool (0, 5, 10, NULL) <= 0);
  
  fprintf (stderr, "\n+ TLSF STRUCTURE SIZE tests (1 KB - 500 KB)\n\n");
  for (sli = 5; sli > 0; sli --) {
    average = 0;
    total = 0;
    max = 0;
    ext_frag_max = 0;
    ext_frag_average = 0;
    min = 1000000;
    for (n = 1; n < 500; n += 10) {
      memset (memory_pool, 0xFF, 1024 * 1024);
   
      mem_alloc = init_memory_pool (0, sli, n, memory_pool);
      if (mem_alloc < 0) {
	test_result_bool ("TLSF structure initialisation", 0);
	goto out;
      }
      if (max < (n*1024 - mem_alloc))
	max = (n*1024 - mem_alloc);

      if (min > (n*1024 - mem_alloc))
	min = (n*1024 - mem_alloc);

      average += (n*1024 - mem_alloc);
      total ++;

      for (m = mem_alloc; m > 0; m --) {
	ptr = rtl_malloc_ex (m, memory_pool);
	if (ptr != NULL) goto out1;
      }
      
    out1:
      ext_frag_average += (mem_alloc - m);
      if (ext_frag_max < (mem_alloc -m))
	ext_frag_max = (mem_alloc - m);
      memset (ptr, 0xAA, m);
      rtl_free_ex (ptr, memory_pool);
      if (memory_pool [n * 1024] != 0xFF)  {
	fprintf (stderr, "\n");
	test_result_bool ("TLSF structure size exceeded", 0);
	goto out;
      }
    }
    sprintf (buffer, "(SLI = %d) struct size average", sli);   
    test_result_ll (buffer, (unsigned long long) average/
		    (unsigned long long)total, "Bytes");
    sprintf (buffer, "(SLI = %d) struct size max", sli);
    test_result_int (buffer, max, "Bytes");
    sprintf (buffer, "(SLI = %d) struct size min", sli);
    test_result_int (buffer, min, "Bytes");
    sprintf (buffer, "(SLI = %d) first block wasted size average", sli);   
    test_result_ll (buffer, (unsigned long long) 
		    ext_frag_average / (unsigned long long)total, "Bytes");
    sprintf (buffer, "(SLI = %d) first block wasted size max", sli);   
    test_result_int (buffer, ext_frag_max, "Bytes");
    fprintf (stderr, "\n");
  }
  fprintf (stderr, "\n");
  test_result_bool ("TLSF structure size exceeded", 1);
  test_result_bool ("TLSF structure initialisation", 1);
  
 out:

  return 0;
}

/* This first test checks correct TLSF structure initialisation */
static int malloc_tests (void) {
  block_header_t *bh;
  char memory_pool[1024 * 1024]; /* For the  initialisation test with 10
				    Kb will be enought */
  char *ptr, buffer [256];
  int n, sli, total, max, real;
  unsigned long long average, t1, t2, t3, maxll, minll, resll;

  fprintf (stderr, "\n>>> MALLOC tests <<<\n\n");
  init_memory_pool (0, 5, 1024, memory_pool);
  test_result_bool ("Requesting a negative value", 
		    rtl_malloc_ex(-1, memory_pool) == NULL);
  test_result_bool ("Requesting 0 Bytes", 
		    rtl_malloc_ex (0, memory_pool) == NULL);
  
  fprintf (stderr, "\n+ INTERNAL FRAGMENTATION tests (1 KB - 500 KB)\n\n");
  for (sli = 5; sli > 0; sli--) {
    init_memory_pool (0, sli, 1024, memory_pool);
    average = 0;
    total = 0;
    max = 0;
    for (n = 1024; n < 500 * 1024; n ++) {
      ptr = rtl_malloc_ex (n, memory_pool);
      if (ptr == NULL) {
	fprintf (stderr, "\n");
	test_result_bool ("Internal Fragmentation", 0);
	goto out;
      }
      bh = (block_header_t *) ((__u8 *) ptr -
			     TLSF_WORDS2BYTES(beg_header_overhead));
      if (TLSF_WORDS2BYTES(bh -> size) < n) {
	fprintf (stderr, "\n");
	test_result_bool ("Internal Fragmentation", 0);
	goto out;
      }

      if (TLSF_WORDS2BYTES(bh -> size) - n > max) 
	max = TLSF_WORDS2BYTES(bh -> size) - n;
      average += (unsigned long long) TLSF_WORDS2BYTES(bh -> size) - n;
      total ++;
      rtl_free_ex (ptr, memory_pool);
    }
    sprintf (buffer, "(SLI = %d) Int. Frag. average", sli);
    test_result_ll (buffer, (unsigned long long) average / 
		    (unsigned long long)total, "Bytes");
    sprintf (buffer, "(SLI = %d) Int. Frag. max", sli);
    test_result_int (buffer, max, "Bytes");
    fprintf (stderr, "\n");
  }
  fprintf (stderr, "\n");
  test_result_bool ("Internal fragmentation", 1);
  
  /**/
  fprintf (stderr, 
	   "\n+ Response time tests (1 KB - 500 KB) (On the worst case)\n\n");
  for (sli = 5; sli > 0; sli--) {
    init_memory_pool (0, sli, 1024, memory_pool);
    real = 0;
    average = 0;
    total = 0;
    maxll = 0;
    minll = 10000;
    for (n = 1024; n < 500 * 1024; n ++) {
      rdtscll(t1);
      ptr = rtl_malloc_ex (n, memory_pool);
      rdtscll(t2);
      rdtscll(t3);
      if (ptr == NULL) {
	fprintf (stderr, "\n");
	test_result_bool ("Response time", 0);
	goto out;
      }
      resll = (t2 - t1) - (t3 - t2);

      /* This is not a real-time OS so there exist some interferences, this
	 anomalous cases must been eliminated */
      if (resll < 1000) {
	if (maxll < resll) maxll = resll;
	if (minll > resll) minll = resll;
	average += (unsigned long long) resll;
	total ++;
      }
      real ++;
      rtl_free_ex (ptr, memory_pool);
    }
    sprintf (buffer, "(SLI = %d) Response time average", sli);
    test_result_ll (buffer, (unsigned long long) average / 
		    (unsigned long long)total, "ticks");
    sprintf (buffer, "(SLI = %d) Response time max", sli);
    test_result_ll (buffer, maxll, "ticks");
    sprintf (buffer, "(SLI = %d) Response time min", sli);
    test_result_ll (buffer, minll, "ticks");
    sprintf (buffer, "(SLI = %d) Total used samples", sli);
    test_result_int (buffer, total, "");
    sprintf (buffer, "(SLI = %d) Ignored samples", sli);
    test_result_int (buffer, real - total, "");
    sprintf (buffer, "(SLI = %d) Real total samples", sli);
    test_result_int (buffer, real, "");
    fprintf (stderr, "\n");
  }
  fprintf (stderr, "\n");
  test_result_bool ("Response time", 1);
 out:

  return 0;
}

/* This first test checks correct TLSF structure initialisation */
static int free_tests (void) {
  //char memory_pool[10 * 1024]; // For the  initialisation test with 10
  // Kb will be enought
  return 0;
}

/* This first test checks correct TLSF structure initialisation */
static int miscellaneous_tests (void) {
  //char memory_pool[10 * 1024]; // For the  initialisation test with 10
			       // Kb will be enought

  return 0;
}


int main (int argc, char **argv) {
  // stdout is closed because TLSF error msg are given through it
  if (argc == 2) {
    if (strcmp ("-nocolors", argv[1]) == 0)
      ncolors = 1;
  }
  close (1);
  initialisation_tests ();
  malloc_tests ();
  free_tests (); 
  miscellaneous_tests ();
  fprintf (stderr, "\n>>> TESTS Summary <<<\n");
  fprintf (stderr, 
	   "\nFAILED tests: %d\nPASSED tests: %d\nTotal tests: %d\n\n", 
	   failed, passed, failed + passed);
  
  return 0;
}
