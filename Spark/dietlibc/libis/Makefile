#
# RTLinux Lib System.
#
#

include ../../rtl.mk

all: isalnum.o isalpha.o isascii.o isblank.o iscntrl.o isdigit.o isgraph.o islower.o isprint.o ispunct.o isspace.o isupper.o isxdigit.o
	@$(AR) -r libis.a *.o
	@mv *.a $(LIB_DIR)

main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

