
#include <rtl_conf.h>
#if DEVICE_I386_TERMINAL
#include <rtl_posixio.h>
#include <rtl_screen.h>
#include <rtl_devices.h>
#include <arch/vga.h>
#include <arch/kbd.h>
#include <errno.h>

//////////////////////////////////
//// POSIX TERMINAL FUNCTIONS ////
//////////////////////////////////

// This function can be implemented using a proccess running on user mode
// which read from keyboard

static ssize_t tty_read (struct rtl_file *file, char *buffer, size_t size, 
		         loff_t *s){
  
  int n, temp;
  for (n = 0; n < size; n++) {
//    temp = (char) rt_terminal_getchar ();
    if (temp == -1)
      return n;
    buffer [n] = temp;
  }
  return size;
}

static ssize_t tty_write (struct rtl_file *file, const char *buffer, 
		          size_t size, loff_t *s){
  return rt_terminal_putstring ((char *)buffer, size);  
}

static int tty_open (struct rtl_file *file){
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

int init_i386_terminal(void){

  if (rtl_register_chrdev (I386_TERMINAL_MAJOR, "rt_tty", &rtl_tty_fops)) {
    return -EIO;
  }

  if (init_vga () != 0) return -1;
  if (init_rtl_screen ()!= 0) return -1;
//  if (init_kbd ()!= 0) return -1;

  return 0;
}

void cleanup_i386_terminal(void){
  close_rtl_screen ();
//  close_kbd ();
  rtl_unregister_chrdev(I386_TERMINAL_MAJOR, "rt_tty");
}
#endif //DEVICE_I386_TERMINAL
