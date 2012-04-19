#
# RTLinux Lib System.
#
#

include ../../rtl.mk

all: rand.o rand48.o rand_r.o
	@$(AR) -r librand.a *.o
	@mv *.a $(LIB_DIR)

main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

