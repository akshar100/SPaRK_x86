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

GUESTOS1  = $(TOPDIR)/guestOSes/GuestOS1
GUESTOS2  = $(TOPDIR)/guestOSes/GuestOS2
GUESTOS3  = $(TOPDIR)/guestOSes/GuestOS3

HEAD = head.o
OBJECTS = $(HEAD) misc.o

CFLAGS = $(CPPFLAGS) -O2 -DSTDC_HEADERS
ZLDFLAGS = -e startup_32
#ZIMAGE_OFFSET = 0x1000

#ZLINKFLAGS = -Ttext $(ZIMAGE_OFFSET) $(ZLDFLAGS)
ZLINKFLAGS = -Tsa-rtl-kmp.lds

bzImage: $(CONFIGURE) head.o bbootsect bsetup tools/build
ifdef CONFIG_RTL_FP_SUPPORT	
	$(LD) -L/usr/lib $(ZLINKFLAGS) -o rtvic head.o $(OBJ_DIR)/*.o $(STATIC_LIBS)  -static $(shell cc -print-libgcc-file-name)  
else	
	$(LD) -L/usr/lib $(ZLINKFLAGS) -o rtvic head.o $(OBJ_DIR)/*.o $(STATIC_LIBS)  -static $(shell cc -print-libgcc-file-name)  
endif	
	$(OBJCOPY) rtvic  rtvic.out
	$(OBJCOPY) head.o  head.out
#	tools/attach 2 rtvic.out $(GUESTOS1) $(GUESTOS2) $(GUESTOS3) > SparkImage
#	tools/build -b bbootsect bsetup  SparkImage > bzImage
#	cp -f bzImage $(IMAGE_DIR)/$(TARGET_NAME)

tools/build:
	gcc tools/build.c -o tools/build          	
	gcc tools/attach.c -o tools/attach          	

bootsect: bootsect.o
	$(LD) -Ttext 0x0 -s --oformat binary -o $@ $<

bootsect.o: bootsect.s
	$(AS) -o $@ $<

bootsect.s: bootsect.S Makefile $(BOOT_INCL) 
	$(CPP) $(CPPFLAGS) -traditional $(SVGA_MODE) $(RAMDISK) $< -o $@

bbootsect: bbootsect.o
	$(LD) -Ttext 0x0 -s --oformat binary $< -o $@

bbootsect.o: bbootsect.s
	$(AS) -o $@ $<

bbootsect.s: bootsect.S Makefile $(BOOT_INCL) 
	$(CPP) $(CPPFLAGS) -D__BIG_KERNEL__  -traditional $(SVGA_MODE) $(RAMDISK) $< -o $@

setup: setup.o
	$(LD) -Ttext 0x0 -s --oformat binary -e begtext -o $@ $<

setup.o: setup.s
	$(AS) -o $@ $<

setup.s: setup.S video.S Makefile 
	$(CPP) $(CPPFLAGS) -traditional $(SVGA_MODE) $(RAMDISK) $< -o $@

bsetup: bsetup.o
	$(LD) -Ttext 0x0 -s --oformat binary -e begtext -o $@ $<

bsetup.o: bsetup.s
	$(AS) -o $@ $<

bsetup.s: setup.S video.S Makefile 
	$(CPP) $(CPPFLAGS) -D__BIG_KERNEL__ -traditional $(SVGA_MODE) $(RAMDISK) $< -o $@

head.o: head.S
	$(CC) $(AFLAGS) -traditional -c head.S

dep:

clean:
	find . \( -name '*~' -o -name '*.o' -o -name core \) -exec /bin/rm -r '{}' \;
	rm -f *.out
	rm -f *.s
	rm -f rtvic
	rm -f bzImage
	rm -f tools/build
	rm -f tools/attach
	rm -f SparkImage
	rm -f setup bootsect zImage 
	@cd ..
	rm -f bsetup bbootsect bzImage 
