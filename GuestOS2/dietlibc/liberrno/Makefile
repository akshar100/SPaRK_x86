#
# RTLinux Lib System.
#
#

include ../../rtl.mk

all: errno.o errno_location.o
	@$(AR) -r liberrno.a *.o
	@mv *.a $(LIB_DIR)

main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

