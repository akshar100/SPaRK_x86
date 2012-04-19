/* 
 * bits.h generic arch
 *
 * Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) Feb, 2004 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

#ifndef _BITS_H_
#define _BITS_H_
/*
static __inline__ int TLSF_fls(int x) {
  unsigned int r, x2 = (unsigned int) x;
 
  if (!x) return -1;
  for (r = 0; x2 >>= 1; r ++);
  return r;
}
*/

static __inline__ int TLSF_fls (int x) {
  int r = 31;

  if (!x)
    return -1;
  
  if (!(x & 0xffff0000)) {
    x <<= 16;
    r -= 16;
  }
  if (!(x & 0xff000000)) {
    x <<= 8;
    r -= 8;
  }
  if (!(x & 0xf0000000)) {
    x <<= 4;
    r -= 4;
  }
  if (!(x & 0xc0000000)) {
    x <<= 2;
    r -= 2;
  }
  if (!(x & 0x80000000)) {
    x <<= 1;
    r -= 1;
  }
  return r;
}

#endif
