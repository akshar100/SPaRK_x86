#
# Main RTLinux system.
#
# Copyright (C) 1999 FSMLabs
# Written by Cort Dougan <cort@fsmlabs.com>, Victor Yodaiken <yodaiken@fsmlabs.com>
# 
#

include ../rtl.mk

#all: i386_kbd.o i386_screen.o i386_serial_mouse.o i386_terminal.o i386_serial_mouse.o  i386_vga.o i386_ps2_mouse.o i386_serial.o i386_pci1710.o i386_pci82573v.o
all: i386_kbd.o i386_screen.o i386_terminal.o i386_vga.o i386_serial.o i386_pci1710.o i386_pci82573v.o
	echo "Drivers Compiling..."
	mv -f *.o $(OBJ_DIR)
main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

