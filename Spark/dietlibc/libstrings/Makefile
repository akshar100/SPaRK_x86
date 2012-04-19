#
# RTLinux Lib System.
#
#

include ../../rtl.mk

all: strcasecmp.o strcat.o strchr.o strcmp.o strcpy.o strcspn.o strdup.o strlcat.o strncat.o strncmp.o strncpy.o strpbrk.o strrchr.o strsep.o strspn.o strstr.o strtod.o strtof.o strtok.o strtok_r.o strtol.o strtold.o strtoll.o strtoul.o strtoull.o strxfrm.o strlen.o   
	@$(AR) -r libstrings.a *.o
	@mv *.a $(LIB_DIR)

main: all



clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;

.PHONY: dummy

