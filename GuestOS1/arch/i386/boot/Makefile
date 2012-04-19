#
# arch/i386/boot/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1994 by Linus Torvalds
#

RTL       = $(shell cd ../../../;pwd)

include $(RTL)/rtl.mk

CPPFLAGS  = -I$(RTL)/include
TOPDIR    = $(RTL)
BOOTIMAGE = bzImage
OBJCOPY   = objcopy -O binary -R .note -R .comment -R .debug -S


HEAD = head.o

CFLAGS = $(CPPFLAGS) -O2 -DSTDC_HEADERS
ZLDFLAGS = -e startup_32
#ZIMAGE_OFFSET = 0x8000000 # 128MB mark
ZIMAGE_OFFSET = 0x0 

#ZLINKFLAGS = -Ttext $(ZIMAGE_OFFSET) $(ZLDFLAGS)
ZLINKFLAGS = -T sa-rtl-cmp.lds


bzImage: $(CONFIGURE) head.o 
ifdef CONFIG_RTL_FP_SUPPORT	
	$(LD) -L/usr/lib $(ZLINKFLAGS) -o rtvic head.o $(OBJ_DIR)/*.o $(STATIC_LIBS)  -static $(shell cc -print-libgcc-file-name)  
else	
	$(LD) -L/usr/lib $(ZLINKFLAGS) -o rtvic head.o $(OBJ_DIR)/*.o $(STATIC_LIBS)  -static $(shell cc -print-libgcc-file-name)  
endif	
	$(OBJCOPY) rtvic  rtvic.out
	mv rtvic.out bzImage
	cp -f bzImage $(IMAGE_DIR)/$(TARGET_NAME)

head.o: head.S
	$(CC) $(AFLAGS) -traditional -c head.S

dep:

clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;
	rm -f *.out
	rm -f *.s
	rm -f bzImage
	rm -f rtvic
	@cd ..
	rm -f bzImage 
