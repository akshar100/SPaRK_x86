#
# RTLinux Lib System.
#
#

include ../../rtl.mk

all: __dtostr.o __lltostr.o __v_printf.o __isinf.o __isnan.o __ltostr.o puts.o vprintf.o printf.o assert_fail.o  vsprintf.o vsnprintf.o fprintf.o 
	@$(AR) -r libstdio.a *.o
	@mv *.a $(LIB_DIR)

main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

