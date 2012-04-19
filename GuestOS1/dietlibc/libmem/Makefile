#
# RTLinux Lib System.
#
#

include ../../rtl.mk

all: memccpy.o memchr.o memcmp.o memmem.o memmove.o memrchr.o memset.o  memcpy.o
	@$(AR) -r libmem.a *.o
	@mv *.a $(LIB_DIR)

main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

