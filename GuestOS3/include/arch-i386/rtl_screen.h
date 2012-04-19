/*
 *  rtl_screen.h
 *
 * This file has several functions from oskit project
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) April, 2003 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

#ifndef _RTL_SCREEN_H_
#define _RTL_SCREEN_H_

void set_scroll_mode(int sm);
void set_text_attr (unsigned char attr);
unsigned char get_char_in_screen_pos (unsigned int pos);
unsigned char get_attr_of_screen_pos (unsigned int pos);
int rt_terminal_putchar (unsigned char c);
int rt_terminal_putstring (char *initial_printkbuf, int size);
void clrscr (void);
void rt_clrscr (void);
void console_on(void);
void console_off(void);
int init_rtl_screen (void);
void close_rtl_screen (void);

#endif
