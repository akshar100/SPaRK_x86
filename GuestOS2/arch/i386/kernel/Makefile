#
# Main RTLinux system.
#
# Copyright (C) 1999 FSMLabs
# Written by Cort Dougan <cort@fsmlabs.com>, Victor Yodaiken <yodaiken@fsmlabs.com>
# 
#

include ../../../rtl.mk
CPPFLAGS  = -I$(RTL_DIR)/include

all: malloc.o context.o page.o hypercall_lib.o systemcallEntry.o systemcall.o zcircqueue.o  
	mv -f *.o $(OBJ_DIR)

main: all


systemcallEntry.o: systemcallEntry.s
	$(AS) -o $@ $<

systemcallEntry.s: systemcallEntry.S
	$(CPP) $(CPPFLAGS) -traditional $< -o $@


clean:
	rm -f systemcallEntry.s
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

