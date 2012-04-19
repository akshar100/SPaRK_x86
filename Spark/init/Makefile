#
# Main RTLinux system.
#
# Copyright (C) 1999 FSMLabs
# Written by Cort Dougan <cort@fsmlabs.com>, Victor Yodaiken <yodaiken@fsmlabs.com>
# 
#

all: init.o video.o
	@mv -f *.o $(OBJ_DIR)
main: all

include ../rtl.mk


clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

