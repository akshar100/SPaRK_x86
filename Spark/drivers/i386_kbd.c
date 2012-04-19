/*
 *  rtl_kbd.c 
 *
 * This file has several functions from oskit project
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) April, 2003 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

#include <rtl_conf.h>


#include <rtl_devices.h>
#include <rtl_posixio.h>
#include <rtl_sync.h>
#include <arch/kbd.h>
#include <arch/rtl_io.h>
#include <arch/vga.h>
#include <arch/rtl_screen.h>
#include <arch/hw_irq.h>
#include <pthread.h>
#include <errno.h>
#include <rtl_printf.h>

// Keyboard Polling Implementation

#define SHIFT -1
#define KEYBF1 -2
#define KEYBF2 -3
#define KEYBF3 -4
#define KEYBF4 -5
#define KEYBF5 -6
#define KEYBF6 -7
#define KEYBF7 -8
#define KEYBF8 -9
#define KEYBF9 -10
#define KEYBF10 -11

volatile int terminal_state = ACTIVATED;
unsigned char BYTE;

static char keymap[128][2] = {
	{0},                    /* 0 */
	{27,    27},            /* 1 - ESC */
	{'1',   '!'},           /* 2 */
	{'2',   '@'},
	{'3',   '#'},
	{'4',   '$'},
	{'5',   '%'},
	{'6',   '^'},
	{'7',   '&'},
	{'8',   '*'},
	{'9',   '('},
	{'0',   ')'},
	{'-',   '_'},
	{'=',   '+'},
	{8,     8},             /* 14 - Backspace */
	{'\t',  '\t'},          /* 15 */
	{'q',   'Q'},
	{'w',   'W'},
	{'e',   'E'},
	{'r',   'R'},
	{'t',   'T'},
	{'y',   'Y'},
	{'u',   'U'},
	{'i',   'I'},
	{'o',   'O'},
	{'p',   'P'},
	{'[',   '{'},
	{']',   '}'},           /* 27 */
	{'\r',  '\r'},          /* 28 - Enter */
	{0,     0},             /* 29 - Ctrl */
	{'a',   'A'},           /* 30 */
	{'s',   'S'},
	{'d',   'D'},
	{'f',   'F'},
	{'g',   'G'},
	{'h',   'H'},
	{'j',   'J'},
	{'k',   'K'},
	{'l',   'L'},
	{';',   ':'},
	{'\'',  '"'},           /* 40 */
	{'`',   '~'},           /* 41 */
	{SHIFT, SHIFT},         /* 42 - Left Shift */
	{'\\',  '|'},           /* 43 */
	{'z',   'Z'},           /* 44 */
	{'x',   'X'},
	{'c',   'C'},
	{'v',   'V'},
	{'b',   'B'},
	{'n',   'N'},
	{'m',   'M'},
	{',',   '<'},
	{'.',   '>'},
	{'/',   '?'},           /* 53 */
	{SHIFT, SHIFT},         /* 54 - Right Shift */
	{0,     0},             /* 55 - Print Screen */
	{0,     0},             /* 56 - Alt */
	{' ',   ' '},           /* 57 - Space bar */
	{0,     0},             /* 58 - Caps Lock */
	{KEYBF1,     KEYBF1},             /* 59 - F1 */
	{KEYBF2,     KEYBF2},             /* 60 - F2 */
	{KEYBF3,     KEYBF3},             /* 61 - F3 */
	{KEYBF4,     KEYBF4},             /* 62 - F4 */
	{KEYBF5,     KEYBF5},             /* 63 - F5 */
	{KEYBF6,     KEYBF6},             /* 64 - F6 */
	{KEYBF7,     KEYBF7},             /* 65 - F7 */
	{KEYBF8,     KEYBF8},             /* 66 - F8 */
	{KEYBF9,     KEYBF9},             /* 67 - F9 */
	{KEYBF10,    KEYBF10},             /* 68 - F10 */
	{0,      0},            /* 69 - Num Lock */
	{0,      0},            /* 70 - Scroll Lock */
	{'7',   '7'},           /* 71 - Numeric keypad 7 */
	{'8',   '8'},           /* 72 - Numeric keypad 8 */
	{'9',   '9'},           /* 73 - Numeric keypad 9 */
	{'-',   '-'},           /* 74 - Numeric keypad '-' */
	{'4',   '4'},           /* 75 - Numeric keypad 4 */
	{'5',   '5'},           /* 76 - Numeric keypad 5 */
	{'6',   '6'},           /* 77 - Numeric keypad 6 */
	{'+',   '+'},           /* 78 - Numeric keypad '+' */
	{'1',   '1'},           /* 79 - Numeric keypad 1 */
	{'2',   '2'},           /* 80 - Numeric keypad 2 */
	{'3',   '3'},           /* 81 - Numeric keypad 3 */
	{'0',   '0'},           /* 82 - Numeric keypad 0 */
	{'.',   '.'},           /* 83 - Numeric keypad '.' */
};

static unsigned char buffer_char = 0;
static volatile int waiting_a_char = DEACTIVATED;
static char scan_to_ascii(int scan_code);
unsigned char lee_status(void);  
unsigned char lee_scancode(void);   


int rt_terminal_getchar (void) {
	unsigned char temp = 0;
	int scancode;
	unsigned char status=lee_status();


	if (status & 0x1) {
		scancode = lee_scancode();   /* Read un scancode */
		temp = scan_to_ascii((int) scancode);
		if (temp == '\r' || temp == '\n') return -1;
	} else return -1;
	return temp;
}

/* Translate keyboard Scancode to ASCII code */
static char scan_to_ascii(int scan_code) {
	static unsigned shift_state;
	char ch;

	/* Handle key releases - only release of SHIFT is important. */
	if (scan_code & 0x80) {
		scan_code &= 0x7f;
		if (keymap[scan_code][0] == SHIFT)
			shift_state = 0;
		ch = -1;
	} else {
		/* Translate the character through the keymap. */
		ch = keymap[scan_code][shift_state];
		if (ch == SHIFT) {
			shift_state = 1;
			ch = -1;
		} else if (ch == 0)
			ch = -1;
	}
	return ch;
}

/* Manage the scancode that we've read */
static int rtl_handle_scancode(unsigned char scancode) {
	char car = scan_to_ascii((int) scancode);
	static unsigned char c_map = 0x0;

	if (car == -1) return 0;
	switch (car) {
		case KEYBF1:
			if (waiting_a_char == ACTIVATED) {
				buffer_char = -1;
				waiting_a_char = DEACTIVATED;
				//      pthread_mutex_unlock (&getchar2_mutex);
				return 1;
			} 
			return 0;
			break;
		case KEYBF8: 
			c_map ++;
			set_char_map_selec (c_map);
			return 1;
			break;
		case KEYBF9:
			rt_clrscr ();
			return 1;
			break;
		default:
			if (waiting_a_char == ACTIVATED) {
				buffer_char = car;
				waiting_a_char = DEACTIVATED;
				//      pthread_mutex_unlock (&getchar2_mutex);
			}
			return 1;
	}
	return 0;
}


unsigned char lee_status(void)   /* Read keyboard state from  0x64  port */
{
	return rtl_inb(0x64);
};

unsigned char lee_scancode(void) /* Read keyboard scancode from port 0x60 */   
{
	return rtl_inb(0x60);
};
/* Reset Keyboard */
void resetkbd(void)
{
#if 1	
	__asm__("inb $0x61,%al \n\t"
			"movb %al,%ah \n\t"
			"orb $0x80,%al \n\t"
			"outb %al,$0x61 \n\t"
			"xchg %al,%ah \n\t"
			"outb %al,$0x61 \n\t");
#endif
};


static ssize_t tty_read (struct rtl_file *file, char *buffer, size_t size, 
		loff_t *s){

	int n, temp;
	for (n = 0; n < size; n++) {
		temp = (char) rt_terminal_getchar ();
		if (temp == -1)
			return n;
		buffer [n] = temp;
	}
	return size;
}

static ssize_t tty_write (struct rtl_file *file, const char *buffer, 
		size_t size, loff_t *s){
	return rt_terminal_putstring ((char *)buffer, size);  
}

static int tty_open (struct rtl_file *file){
	return 0;
}

static int tty_release (struct rtl_file *file){
	return 0;
}

static struct rtl_file_operations rtl_tty_fops = {
	NULL,
	tty_read,
	tty_write,
	NULL,
	NULL,
	tty_open,
	tty_release
};

int spark_kbd_data =0xFF;

unsigned int keyboardhandler(unsigned int irq,struct pt_regs *r) {
	unsigned char scancode=0;
	unsigned char asciicode=0;

	scancode = lee_scancode();   /* Read un scancode */
        asciicode = scan_to_ascii(scancode);

        if(asciicode != 0xFF)         /* Error code */
        {
	    if (kbd_guest_os == iCurrGuestOsIndex)
	    	*kbd_data_gos = (int)asciicode;

	    spark_kbd_data = (int)asciicode;
	    // how to call the ISR routine of guest os
	    // call kbd_gos_isr() appropriately 
        }
	return 0;
};

/*
 *	Controller Mode Register Bits
 */

#define KBD_MODE_KBD_INT	0x01	/* Keyboard data generate IRQ1 */
#define KBD_MODE_MOUSE_INT	0x02	/* Mouse data generate IRQ12 */
#define KBD_MODE_SYS 		0x04	/* The system flag (?) */
#define KBD_MODE_NO_KEYLOCK	0x08	/* The keylock doesn't affect the keyboard if set */
#define KBD_MODE_DISABLE_KBD	0x10	/* Disable keyboard interface */
#define KBD_MODE_DISABLE_MOUSE	0x20	/* Disable mouse interface */
#define KBD_MODE_KCC 		0x40	/* Scan code conversion to PC format */
#define KBD_MODE_RFU		0x80

int init_i386_kbd (void){
	int flags;
	rtl_stop_interrupts ();
	rtl_request_global_irq(KEYBOARD_IRQ,&keyboardhandler);
	enable_8259_irq(KEYBOARD_IRQ);
	waiting_a_char == DEACTIVATED;
	if (rtl_register_chrdev (I386_KBD_MAJOR, "rt_kbd", &rtl_tty_fops)) {
		return -EIO;
	}
	rtl_allow_interrupts ();
	return 0;
}

void close_i386_kbd (void) {
	rtl_stop_interrupts ();
	rtl_allow_interrupts ();
}



