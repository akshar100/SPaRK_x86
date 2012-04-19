
#include <rtl_conf.h>
#ifdef DEVICE_I386_PS2_MOUSE
#include <rtl_posixio.h>
#include <rtl_devices.h>
#include <arch/rtl_io.h>
#include <errno.h>

#define KBD_PORT	0x60
#define MOUSE_PORT	0x64




//////////////////////////////////
//// POSIX TERMINAL FUNCTIONS ////
//////////////////////////////////


// This function can be implemented using a proccess running on user mode
// which read from keyboard

void disable_kbd_ps2(void) {
       rtl_outb(0xad,MOUSE_PORT);
       while ((rtl_inb(MOUSE_PORT)&0x2)==02) {};
};

void enable_kbd_ps2(void) {
       rtl_outb(0xae,MOUSE_PORT);
       while ((rtl_inb(MOUSE_PORT)&0x2)==02) {};
};

int check_mouse(void) {
    int repeat=1000;
    while (repeat) {
	    if ((rtl_inb(0x64)&0x1)==0x1) return 1;
	    repeat--;
    };
    return 0;
};
static ssize_t tty_read (struct rtl_file *file, char *buffer, size_t size, 
		         loff_t *s){
  size_t nbytes=0;
  
  while ((nbytes<3) && (check_mouse())){ //while teclas 
    disable_kbd_ps2();
    buffer[nbytes]=rtl_inb(0x60);
    nbytes ++;
    enable_kbd_ps2();
  };
  return nbytes;
}

static ssize_t tty_write (struct rtl_file *file, const char *buffer, 
		          size_t size, loff_t *s){
  return 0;
}

static int tty_open (struct rtl_file *file){
   int aux;
  return 0;
}

static int tty_release (struct rtl_file *file){
  return 0;
}

static struct rtl_file_operations rtl_tty_fops = {
  NULL,
  tty_read,
  tty_write,
  NULL,
  NULL,
  tty_open,
  tty_release
};

/////////////////////////////////////////////////////

int init_i386_ps2_mouse(void){
 unsigned char *ch=0;	
  
  rtl_outb(0xa8,MOUSE_PORT); // Enable mouse port
  while ((rtl_inb(MOUSE_PORT)&0x2)==02) {};
  rtl_outb(0xd4,MOUSE_PORT); //Write mouse device instead of keyboard
  while ((rtl_inb(MOUSE_PORT)&0x2)==02) {};
  rtl_outb(0xf4,KBD_PORT); // Activate mouse (Stream Mode);
  while ((rtl_inb(MOUSE_PORT)&0x2)==02) {};
  while ((rtl_inb(MOUSE_PORT)&0x1)!=01) {};
  disable_kbd_ps2();
  rtl_inb(0x60);
  enable_kbd_ps2();
  
  if (rtl_register_chrdev (I386_PS2_MOUSE_MAJOR, "rt_ps_mouse", &rtl_tty_fops)) {
    return -EIO;
  }

  return 0;
}

void cleanup_i386_ps2_mouse(void){
  rtl_unregister_chrdev(I386_PS2_MOUSE_MAJOR, "rt_ps_mouse");
}

#endif //DEVICE_I386_PS2_MOUSE
