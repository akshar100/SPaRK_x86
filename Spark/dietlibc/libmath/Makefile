#
# RTLinux Lib System.
#
#

include ../../rtl.mk
OBJECTS = abs.o
#if CONFIG_I386
OBJECTS += cos.o sin.o tan.o acos.o asin.o atan.o pow.o fabs.o sqrt.o floor.o ceil.o exp.o fmod.o
#endif
all: $(OBJECTS) 
	@$(AR) -r libmath.a *.o
	@mv *.a $(LIB_DIR)

main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

