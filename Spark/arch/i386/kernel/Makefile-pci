#
# Main RTLinux system.
#
# Copyright (C) 1999 FSMLabs
# Written by Cort Dougan <cort@fsmlabs.com>, Victor Yodaiken <yodaiken@fsmlabs.com>
# 
#

include ../../../rtl.mk
CPPFLAGS  = -I$(RTL_DIR)/include

all: setup.o malloc.o page.o rtl_time_8254.o processor.o hypercallEntry.o hypercall.o direct.o
	#$(AS) -o hypercallEntry.o hypercallEntry.S
	mv -f *.o $(OBJ_DIR)

main: all

hypercallEntry.o: hypercallEntry.s
	$(AS) -o $@ $<

hypercallEntry.s: hypercallEntry.S
	$(CPP) $(CPPFLAGS) -traditional $< -o $@

clean:
	rm -f hypercallEntry.s
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

