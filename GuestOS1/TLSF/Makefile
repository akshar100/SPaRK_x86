//CFLAGS = -Wall -O2 -fomit-frame-pointer
include ../rtl.mk

all: libTLSF.a
	cp libTLSF.a $(LIB_DIR)

LIBTLSF_DEP = TLSF_initialisation.o TLSF_malloc.o TLSF_free.o \
	TLSF_destructor.o TLSF_addblock.o TLSF_calloc.o TLSF_realloc.o 

dep:
	ln -sf generic-arch arch  

libTLSF.a: dep $(LIBTLSF_DEP)
	@$(AR) -r libTLSF.a $(LIBTLSF_DEP)

clean:
	$(RM) -rf *.o *~ libTLSF.a regression_tests arch
