#
# Main RTLinux system.
#
# Copyright (C) 1999 FSMLabs
# Written by Cort Dougan <cort@fsmlabs.com>, Victor Yodaiken <yodaiken@fsmlabs.com>
# 
#

include ../rtl.mk

all: moveBinary.S utility.o circqueue.o  
#all: utility.o 
	$(AS) -o moveBinary.o moveBinary.S
	mv -f *.o $(OBJ_DIR)

clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

