/*
 * vga.c is a driver to use directly the vga adapter
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) April, 2003 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

#include <rtl_conf.h>

#if DEVICE_I386_TERMINAL
#include <arch/vga.h>
#include <arch/rtl_io.h>
#include <asm/types.h>
//#include <linux/kernel.h>

inline void set_cursor_pos (__u16 cursor_pos) {
  rtl_outb (CURSOR_LOC_H, CRTC_COM_REG);
  rtl_outb ((__u8)((cursor_pos >> 8)) & 0xff, CRTC_DATA_REG);
  rtl_outb (CURSOR_LOC_L, CRTC_COM_REG);
  rtl_outb ((__u8)(cursor_pos & 0xff), CRTC_DATA_REG);
}

inline __u16 get_cursor_pos (void) {
  __u8 tmp_H = 0, tmp_L = 0;

  rtl_outb (CURSOR_LOC_H, CRTC_COM_REG);
  tmp_H = rtl_inb (CRTC_DATA_REG);
  
  rtl_outb (CURSOR_LOC_L, CRTC_COM_REG);
  tmp_L = rtl_inb (CRTC_DATA_REG);
  
  return (__u16) ((tmp_H << 8) | tmp_L);
}

inline void set_start_addr (__u16 start_addr) {
  rtl_outb (START_ADDR_H, CRTC_COM_REG);
  rtl_outb ((__u8)((start_addr >> 8)) & 0xff, CRTC_DATA_REG);
  rtl_outb (START_ADDR_L, CRTC_COM_REG);
  rtl_outb ((__u8)(start_addr & 0xff), CRTC_DATA_REG);
}

inline __u16 get_start_addr (void) {
  __u8 tmp_H = 0, tmp_L = 0;
 
  rtl_outb (START_ADDR_H, CRTC_COM_REG);
  tmp_H = rtl_inb (CRTC_DATA_REG);

  rtl_outb (START_ADDR_L, CRTC_COM_REG);
  tmp_L = rtl_inb (CRTC_DATA_REG);
  
  return (__u16) ((tmp_H << 8) | tmp_L);
}

inline void set_char_map_selec (__u8 char_map) {
  rtl_outb (CHAR_MAP_SELEC, SEQ_COM_REG);
  rtl_outb (char_map, SEQ_DATA_REG);
}

inline __u8 get_char_map_selec (void) {
  __u8 tmp = 0;
  rtl_outb (CHAR_MAP_SELEC, SEQ_COM_REG);
  tmp = (__u8) rtl_inb (SEQ_DATA_REG);
  return tmp;
}

inline __u8 get_misc_output (void) {
  return rtl_inb (R_MISC_OUTPUT);
}

/* init vga does check the state of the vga adapter 
   it only can be used in a init_module function */
int init_vga (void){
  __u8 misc_output;
  misc_output = get_misc_output ();
  if ((misc_output & 0x1) == 0x1)
    {}
  else {
    return -1;
  }
    
  return 0;
}

#endif //DEVICE_I386_TERMINAL
