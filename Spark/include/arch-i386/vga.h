/*
 *  vga.h is a driver to use directly the vga adapter
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) April, 2003 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 *
 * 
 */

#ifndef _VGA_H_
#define _VGA_H_

#include <asm/types.h>

#define VGA_BUF ((unsigned char *) 0xB8000)

#define R_MISC_OUTPUT 0x3cc
#define W_MISC_OUTPUT 0x3c2

#define CRTC_COM_REG 0x3d4
#define CRTC_DATA_REG 0x3d5
#define CURSOR_LOC_H 0x0e
#define CURSOR_LOC_L 0x0f
#define START_ADDR_H 0x0c
#define START_ADDR_L 0x0d

#define SEQ_COM_REG 0x3c4
#define SEQ_DATA_REG 0x3c5
#define CHAR_MAP_SELEC 0x3

inline void set_cursor_pos (__u16 cursor_pos);
inline __u16 get_cursor_pos (void);
inline void set_start_addr (__u16 start_addr);
inline __u16 get_start_addr (void);
inline void set_char_map_selec (__u8 char_map);
inline __u8 get_char_map_selec (void);
inline __u8 get_misc_output (void);
int init_vga (void);

#endif
