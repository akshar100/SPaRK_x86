#
# Main RTLinux system.
#
# Copyright (C) 1999 FSMLabs
# Written by Cort Dougan <cort@fsmlabs.com>, Victor Yodaiken <yodaiken@fsmlabs.com>
# 
#

include ../rtl.mk

all:
	make -C librand
	make -C libstdio
	make -C libstrings
	make -C libmem
	make -C libis
	make -C liberrno
	make -C libmath
main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

