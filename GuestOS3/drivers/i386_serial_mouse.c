
#include <rtl_conf.h>
#ifdef DEVICE_I386_SERIAL_MOUSE
#include <rtl_posixio.h>
#include <rtl_devices.h>
#include <arch/rtl_io.h>
#include <errno.h>

#define COMPORT       0x3f8
#define LSR  (COMPORT + 5)
#define TBE 0x20
#define RDA 0x01
#define RT_COM_RXB 0x00
#define RT_COM_TXB 0x00
#define RT_COM_IER 0x01
#define RT_COM_IIR 0x02
#define RT_COM_FCR 0x02
#define RT_COM_LCR 0x03
#define RT_COM_MCR 0x04
#define RT_COM_LSR 0x05
#define RT_COM_MSR 0x06
#define RT_COM_DLL 0x00
#define RT_COM_DLM 0x01


//////////////////////////////////
//// POSIX TERMINAL FUNCTIONS ////
//////////////////////////////////


// This function can be implemented using a proccess running on user mode
// which read from keyboard

static ssize_t tty_read (struct rtl_file *file, char *buffer, size_t size, 
		         loff_t *s){
  size_t nbytes=0;
  while (((rtl_inb(LSR) & RDA) == RDA) && (nbytes<3)){ //while teclas 
    buffer[nbytes]=rtl_inb(COMPORT);
    nbytes ++;
  };
  return nbytes;
}

static ssize_t tty_write (struct rtl_file *file, const char *buffer, 
		          size_t size, loff_t *s){
  return 0;
}

static int tty_open (struct rtl_file *file){
   int aux;
   __asm("cli\n");
   rtl_outb(0,COMPORT +1);   // Turn off interrupts
   rtl_outb(0x80,COMPORT+3); // enable DLAB 
   rtl_outb(96,COMPORT+0); // Set divisor LSB
   rtl_outb(0x00,COMPORT+1); // Set divisor MSB
   rtl_outb(0x02,COMPORT+3); // 7 bits no parity 1 stop bit
//   outb(0xc7,COMPORT+2); // Enable fifo 
 //  outb(0,COMPORT+4);
   for (aux=0;aux<=0xffff;aux++) {};
   rtl_outb(0x0b,COMPORT+4); // Turn On DTR,DTS and out2
//   outb(0x01,COMPORT+1); // Interrupt when data received
   __asm("sti\n");
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

int init_i386_serial_mouse(void){

  if (rtl_register_chrdev (I386_SERIAL_MOUSE_MAJOR, "rt_serial_mouse", &rtl_tty_fops)) {
    return -EIO;
  }

  return 0;
}

void cleanup_i386_serial_mouse(void){
  rtl_unregister_chrdev(I386_SERIAL_MOUSE_MAJOR, "rt_serial_mouse");
}

#endif //DEVICE_I386_SERIAL_MOUSE
