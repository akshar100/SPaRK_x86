/*
 *  kbd.h
 *
 * This file has several functions from deblib project.
 * Thanks to Vicente Esteve
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) April, 2003 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

#ifndef _KDB_H_
#define _KDB_H_

#include <rtl_core.h>

#define KEYBOARD_IRQ 1

#define DEACTIVATED 0
#define ACTIVATED 1

extern volatile int terminal_state;

                       /* Handle de la IRQ 1 (teclado) */
//unsigned handle_kbd_event (unsigned int irq, struct pt_regs *regs);

int rt_terminal_getchar (void);

int init_i386_kbd (void);
void close_kbd (void);

// 
extern int *kbd_data_gos;
extern int kbd_guest_os;
extern int spark_kbd_data;


#endif

