/*
 *  rtl_screen.c 
 *
 * This file has several functions from oskit project
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) April, 2003 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

/*
 * Copyright (c) 1994-1995, 1998 University of Utah and the Flux Group.
 * All rights reserved.
 * 
 * This file is part of the Flux OSKit.  The OSKit is free software, also known
 * as "open source;" you can redistribute it and/or modify it under the terms
 * of the GNU General Public License (GPL), version 2, as published by the Free
 * Software Foundation (FSF).  To explore alternate licensing terms, contact
 * the University of Utah at csl-dist@cs.utah.edu or +1-801-585-3271.
 * 
 * The OSKit is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GPL for more details.  You should have
 * received a copy of the GPL along with the OSKit; see the file COPYING.  If
 * not, write to the FSF, 59 Temple Place #330, Boston, MA 02111-1307, USA.
 */

/* This is a special "feature" (read: kludge)
   intended for use only for kernel debugging.
   It enables an extremely simple console output mechanism
   that sends text straight to CGA/EGA/VGA video memory.
   It has the nice property of being functional right from the start,
   so it can be used to debug things that happen very early
   before any devices are initialized.  */
/*
 * Modificated to be used by RTLinux.
 */

#include <rtl_conf.h>

#if DEVICE_I386_TERMINAL
#include <ctype.h>
#include <arch/vga.h>
#include <asm/types.h>
#include <pthread.h>

#define MAX_HOR 80
#define MAX_VER 25

 /*----------------------*
 **-- Global variables --*
 **----------------------*/
static unsigned char title_attr = 0x17;
static unsigned char text_attr = 0x07;
static unsigned char text_error = 0xa4;
static int scroll_active = 1;

#define BLACK 0x0
#define WHITE 0x7
#define YELLOW 0xE
#define MAGENTA 0x5
#define CYAN 0x3
#define RED 0x4
#define GREEN 0x2
#define BLUE 0x1


typedef struct {
  __u16 cursor_pos;
  __u16 start_addr;
  __u8 char_map_selec;
  __u16 screen_buffer [MAX_VER * MAX_HOR];
  
} screen_t;

static __u8 *vidbase = VGA_BUF;

static __s8 RT_CONSOLE_STRING[] = "Spark virtual terminal V1.0";
static __s8 RT_BUFFER_OVERFLOW[] = "Spark BUFFER OVERFLOW\n";


#define BG_CHAR ((unsigned short) (text_attr << 8) | ' ') 
// 0xTT00. 'TT' is the 'text_attr' char (remember: little-endian order)

static screen_t linux_screen;
static screen_t rt_terminal;
static __u8 buffer_overflow = 0;

static inline int ascii2int(unsigned char a) {
  return (int)(a - 48);
}



static void
fillw(unsigned short pat, const void *base, int cnt) {
  volatile unsigned short *p;  

  p = (unsigned short *) base;
  
  while (cnt--) 
    *p++ = pat;
}

/*---------------------*
 *-- set_scroll_mode --*
 *---------------------*/
void set_scroll_mode(int sm) 
{
  scroll_active = sm;
}

/*-------------------*
 *-- set_text_attr --*
 *-------------------*/
void set_text_attr (unsigned char attr)
{
  text_attr = attr;
}

/*----------------------------*
 *-- get_char_in_screen_pos --*
 *----------------------------*/
unsigned char get_char_in_screen_pos (unsigned int pos) 
{
  volatile unsigned char *p = vidbase + pos * 2;
  return p[0];
}

/*----------------------------*
 *-- get_attr_of_screen_pos --*
 *----------------------------*/
unsigned char get_attr_of_screen_pos (unsigned int pos) 
{
  volatile unsigned char *p = vidbase + pos * 2;
  return p[1];
}


//static spinlock_t rt_terminal_lock = SPIN_LOCK_UNLOCKED;

#define MAX_PRINTKBUF 2048

static char in_printkbuf [MAX_PRINTKBUF]; 
/* please don't put this on my stack*/
static char *printkptr = &in_printkbuf[0];
static int rtl_screen_irq = 0;

/*-------------------------*
 *-- rt_terminal_putchar --*
 *-------------------------*/

void rtl_terminal_handler(void);

int rt_terminal_putchar (unsigned char c) {
  int i = 1;
  char initial_printkbuf [1];
  initial_printkbuf [0] = c;
  
  /* perhaps we should discard old data instead */
  if (i > MAX_PRINTKBUF - (printkptr - in_printkbuf) - 30) {
    i = MAX_PRINTKBUF - (printkptr - in_printkbuf) - 30;
  }
  if (i <= 0) {
    return 0;
  }

  memcpy (printkptr, initial_printkbuf, i);
  
  printkptr += i;

  if ((printkptr - in_printkbuf) > MAX_PRINTKBUF/2) {
    printkptr = &in_printkbuf[0];
    
    buffer_overflow = 1;
  }

  *printkptr = 0;

  if (rtl_screen_irq) {
//    rtl_global_pend_irq(rtl_screen_irq);
  }

  rtl_terminal_handler();
  return i;
}

/*---------------------------*
 *-- rt_terminal_putstring --*
 *---------------------------*/

int rt_terminal_putstring (char *initial_printkbuf, int size) {
 
  int i = size;

  /* perhaps we should discard old data instead */
  if (i > MAX_PRINTKBUF - (printkptr - in_printkbuf) - 30) {
    i = MAX_PRINTKBUF - (printkptr - in_printkbuf) - 30;
  }
  if (i <= 0) {
    return 0;
  }

  memcpy (printkptr, initial_printkbuf, i);
  
  printkptr += i;

  if ((printkptr - in_printkbuf) > MAX_PRINTKBUF/2) {
    printkptr = &in_printkbuf[0];
    buffer_overflow = 1;
  }
  *printkptr = 0;

  if (rtl_screen_irq) {
//    rtl_global_pend_irq(rtl_screen_irq);
  }
  rtl_terminal_handler();
  return i;
}

static unsigned char defined_attr = 0;

/*---------------------------*
 *-- rt_terminal_putchar_s --*
 *---------------------------*/
static void rt_terminal_putchar_s (unsigned char c, unsigned char c_attr)
{
  static int state = 0;
  static int digit1 = 0, digit2 = 0, recv1 = 0, recv2=0;
  static int *digit = &digit1;
  unsigned char *ptr;
  unsigned short bg_c = ((unsigned short) (c_attr << 8) | ' ');
 
  
  if (c == 0) return;
//  if (terminal_state == ACTIVATED)
    ptr = vidbase;
//  else
//    ptr = (unsigned char *) rt_terminal.screen_buffer;
  
  //base_critical_enter();


  /* Analising VT100 symbols */
  switch (state) {
  case 0:
    // ESC code in ASCII == 27
    if (c == 27) {
      state = 1;
      return;
    }
    break;
  case 1:
    if (c == '[') {
      state = 2;
      return;
    }
    else state = 0;
    break;
  case 2:
    if (isdigit (c)) {
      if (digit == &digit1)
	recv1 = 1;
      if (digit == &digit2) {
	recv2 = 1;
      };	
      *digit *= 10;
      *digit += ascii2int(c);
      return;
    }
    if (c == ';') {
      digit = &digit2;
      return;
    } 
  case 3:
    switch (c) {
    case 'f':
    case 'H':
      if (recv1 != 0)
	rt_terminal.cursor_pos = (digit1 * 80);

      if (recv2 != 0) 
	rt_terminal.cursor_pos += digit2;
	
      if (recv1 == 0 && recv2 == 0) {
	rt_terminal.cursor_pos = 0;
      }
      recv1 = 0;
      recv2 = 0;
      digit1 = 0;
      digit2 = 0;
      digit = &digit1;
      state = 0;
      return;
      break;
    case 'A':
      if (recv1 == 0) digit1 = 1;
      rt_terminal.cursor_pos -= (digit1 * 80);
      recv1 = 0;
      recv2 = 0;
      digit1 = 0;
      digit2 = 0;
      digit = &digit1;
      state = 0;
      return;
      break;
    case 'B':
      if (recv1 == 0) digit1 = 1;
      rt_terminal.cursor_pos += (digit1 * 80);
      recv1 = 0;
      recv2 = 0;
      digit1 = 0;
      digit2 = 0;
      digit = &digit1;
      state = 0;
      return;
      break;
    case 'C':
      if (recv1 == 0) digit1 = 1;
      rt_terminal.cursor_pos += digit1;
      recv1 = 0;
      recv2 = 0;
      digit1 = 0;
      digit2 = 0;
      digit = &digit1;
      state = 0;
      return;
      break;
    case 'm':
      if (recv1 !=0) {
	switch (digit1) {
	case 0:
	  defined_attr = c_attr;
	  break;
	case 5:
	  defined_attr |= 0x80;
	  break;
	case 30:
	  defined_attr &= 0x70;
	  defined_attr |= BLACK;
	  break;
	case 31:
	  defined_attr &= 0x70;
	  defined_attr |= RED;
	  break;
	case 32:
	  defined_attr &= 0x70;
	  defined_attr |= GREEN;
	  break;
	case 33:
	  defined_attr &= 0x70;
	  defined_attr |= YELLOW;
	  break;
	case 34:
	  defined_attr &= 0x70;
	  defined_attr |= BLUE;
	  break;
	case 35:
	  defined_attr &= 0x70;
	  defined_attr |= MAGENTA;
	  break;
	case 36:
	  defined_attr &= 0x70;
	  defined_attr |= CYAN;
	  break;
	case 37:
	  defined_attr &= 0x70;
	  defined_attr |= WHITE;
	  break;
	case 40:
	  defined_attr &= 0x0F;
	  defined_attr |= (BLACK << 4);
	  break;
	case 41:
	  defined_attr &= 0x0F;
	  defined_attr |= (RED << 4);
	  break;
	case 42:
	  defined_attr &= 0x0F;
	  defined_attr |= (GREEN << 4);
	  break;
	case 43:
	  defined_attr &= 0x0F;
	  defined_attr |= (YELLOW << 4);
	  break;
	case 44:
	  defined_attr &= 0x0F;
	  defined_attr |= (BLUE << 4);
	  break;
	case 45:
	  defined_attr &= 0x0F;
	  defined_attr |= (MAGENTA << 4);
	  break;
	case 46:
	  defined_attr &= 0x0F;
	  defined_attr |= (CYAN << 4);
	  break;
	case 47:
	  defined_attr &= 0x0F;
	  defined_attr |= (WHITE << 4);
	  break;
	}
      }
      if (recv2 !=0) {
	switch (digit2) {
	case 0:
	  defined_attr = c_attr;
	  break;
	case 5:
	  defined_attr |= 0x80;
	  break;
	case 30:
	  defined_attr &= 0x70;
	  defined_attr |= BLACK;
	  break;
	case 31:
	  defined_attr &= 0x70;
	  defined_attr |= RED;
	  break;
	case 32:
	  defined_attr &= 0x70;
	  defined_attr |= GREEN;
	  break;
	case 33:
	  defined_attr &= 0x70;
	  defined_attr |= YELLOW;
	  break;
	case 34:
	  defined_attr &= 0x70;
	  defined_attr |= BLUE;
	  break;
	case 35:
	  defined_attr &= 0x70;
	  defined_attr |= MAGENTA;
	  break;
	case 36:
	  defined_attr &= 0x70;
	  defined_attr |= CYAN;
	  break;
	case 37:
	  defined_attr &= 0x70;
	  defined_attr |= WHITE;
	  break;
	case 40:
	  defined_attr &= 0x0F;
	  defined_attr |= (BLACK << 4);
	  break;
	case 41:
	  defined_attr &= 0x0F;
	  defined_attr |= (RED << 4);
	  break;
	case 42:
	  defined_attr &= 0x0F;
	  defined_attr |= (GREEN << 4);
	  break;
	case 43:
	  defined_attr &= 0x0F;
	  defined_attr |= (YELLOW << 4);
	  break;
	case 44:
	  defined_attr &= 0x0F;
	  defined_attr |= (BLUE << 4);
	  break;
	case 45:
	  defined_attr &= 0x0F;
	  defined_attr |= (MAGENTA << 4);
	  break;
	case 46:
	  defined_attr &= 0x0F;
	  defined_attr |= (CYAN << 4);
	  break;
	case 47:
	  defined_attr &= 0x0F;
	  defined_attr |= (WHITE << 4);
	  break;
	}
      }
      recv1 = 0;
      recv2 = 0;
      digit1 = 0;
      digit2 = 0;
      digit = &digit1;
      state = 0;
      return;
      break;
    }
  }

  if (defined_attr != 0) c_attr = defined_attr;
  //base_critical_enter();

  switch (c) {
  case '\n':
  case '\r':
    if (rt_terminal.cursor_pos / 80 == 24) {
      /* It's in the last line, so scroll screen */
      if (scroll_active) {
	memcpy(ptr, ptr + 80*2, 80*24*2);
	fillw(bg_c, ptr + 80*24*2, 80);
      }
      rt_terminal.cursor_pos =  80 * 24;
    } else {
      rt_terminal.cursor_pos = (rt_terminal.cursor_pos + 80) 
	- (rt_terminal.cursor_pos % 80);
    }
    break;
  case '\b':
    if (rt_terminal.cursor_pos > 0) {
      volatile unsigned char *p = ptr + (--rt_terminal.cursor_pos) * 2;
      
      p[0] = ' ';
      p[1] = c_attr;
    }
    
    break;
  case '\t':
    do {
      rt_terminal_putchar_s (' ', text_attr);
    } while ((rt_terminal.cursor_pos & 7) != 0);
    break;
    
  default:
    
    /* Stuff the character into the video buffer. */
    {
      volatile unsigned char *p = ptr + rt_terminal.cursor_pos * 2;
      
      p[0] = c;
      p[1] = c_attr;
      
      /* Wrap if we reach the end of a line.  */
      if (rt_terminal.cursor_pos % 80 == 79) {
	rt_terminal_putchar_s('\n', text_attr);
      } else {
	rt_terminal.cursor_pos++;
      }
    }
    
    break;
  }
  
//  if (terminal_state == ACTIVATED)
    set_cursor_pos(rt_terminal.cursor_pos);
}

void clrscr (void) {
  fillw(BG_CHAR, vidbase, MAX_VER * MAX_HOR);
  rt_terminal.cursor_pos = MAX_HOR;
  set_cursor_pos (rt_terminal.cursor_pos);
}

void rt_clrscr (void) {
  int n, len;
  unsigned short *ptr;
//  if (terminal_state == ACTIVATED)
    ptr = (unsigned short *) vidbase;
//  else
//    ptr = rt_terminal.screen_buffer;
  
  len = strlen (RT_CONSOLE_STRING);
  for (n = 0; n < 80; n++ ) {
    if (n < len)
      ptr [n] = ((title_attr <<8) | RT_CONSOLE_STRING [n]);
    else
      ptr [n] = ((title_attr <<8) | ' ');
  }
        
  fillw(BG_CHAR, ptr + 80, MAX_VER * MAX_HOR);
  rt_terminal.cursor_pos = MAX_HOR;
  set_cursor_pos (rt_terminal.cursor_pos);
}
    
static void Start_Display(void) {
  char *ptr=(char *) VGA_BUF;

  /* we copy the preview buffer in order to retore it later */
  //memcpy ((__u8 *) linux_screen.screen_buffer, (__u8 *) ptr, 
 // 	  sizeof (__u8) * MAX_HOR * MAX_VER * 2);
  /* What is the start address of linux ? */
  linux_screen.start_addr = get_start_addr();
  /* we change the start address to be the 0 */
  set_start_addr (0);
  /* we try to get the current position of the cursor */
  linux_screen.cursor_pos = get_cursor_pos ();
  /* we set our cursor position */
  set_cursor_pos (rt_terminal.cursor_pos);
  /* the characters map can be different  */
  linux_screen.char_map_selec = get_char_map_selec ();
  /* if yes, we change it */
  if (linux_screen.char_map_selec != rt_terminal.char_map_selec)
     set_char_map_selec (rt_terminal.char_map_selec);
  /* we change it */
  //set_char_map_selec (rt_terminal.char_map_selec);
  /* we copy the preview buffer in order to retore it later */
  memcpy ((__u16 *) linux_screen.screen_buffer, (__u16 *) ptr,
          sizeof (__u16) * MAX_HOR * MAX_VER);  
  /* and finally we copy our buffer screen on the screen */
  memcpy ((__u16 *) ptr, (__u16 *) rt_terminal.screen_buffer, 
  	  sizeof (__u16) * MAX_HOR * MAX_VER);
}

static void End_Display(void) {
  char *ptr = (char *) VGA_BUF;
  
  /* we store the char map, because the user can change it */
  rt_terminal.char_map_selec = get_char_map_selec ();
  /* we retore the linux characters map */
  if (linux_screen.char_map_selec != rt_terminal.char_map_selec)
     set_char_map_selec (linux_screen.char_map_selec);
  /* we restore the before position of the cursor */
  set_cursor_pos (linux_screen.cursor_pos);
  /* we copy our screen buffer */
  memcpy ((__u16 *) rt_terminal.screen_buffer, (__u16 *) ptr, 
  	  sizeof (__u16) * MAX_HOR * MAX_VER);
  /* we restore the linux screen buffer */
  memcpy ((__u16 *) ptr, (__u16 *) linux_screen.screen_buffer,
          sizeof (__u16) * MAX_HOR * MAX_VER);
    
  /* we restore the start addr of linux */
  set_start_addr (linux_screen.start_addr);
}


void console_off(void){
  End_Display();
}

 
void console_on(void) {
  Start_Display();
}

static char out_printkbuf [MAX_PRINTKBUF]; /* the buffer printk actually prints from */

//static spinlock_t rtl_terminal_handler_lock = SPIN_LOCK_UNLOCKED;

void rtl_terminal_handler(void) {
  int n, len, len2;
  //DebugString("Term Handler\n"); 
  len =  printkptr - in_printkbuf + 1;
  memcpy (out_printkbuf, in_printkbuf, printkptr - in_printkbuf + 1);
  printkptr = &in_printkbuf[0];
  *printkptr = 0;

  if (buffer_overflow) {
    buffer_overflow = 0;
    len2 = strlen (RT_BUFFER_OVERFLOW);
    for (n = 0; n < len2; n++) {
      rt_terminal_putchar_s (RT_BUFFER_OVERFLOW [n], text_error);
    }
  }
  for (n = 0; n < len; n++)
    rt_terminal_putchar_s (out_printkbuf [n], text_attr);
}

int init_rtl_screen (void){
  rt_terminal.char_map_selec = get_char_map_selec ();
  linux_screen.char_map_selec = get_char_map_selec ();
  rt_terminal.start_addr = 0;
  rt_terminal.cursor_pos = 80;
  console_on();
  rt_clrscr ();
  return 0;
}

void close_rtl_screen (void) {
  if (rtl_screen_irq) {
//    rtl_free_soft_irq(rtl_screen_irq);
  }
}

#endif //DEVICE_I386_TERMINAL
