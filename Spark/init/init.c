/*
 * Developed at IITB
*/

#include <rtl_tasks.h>
#include <rtl_sched.h>
#include <rtl_time.h>
#include <rtl_debug.h>
#include <rtl_posixio.h>
#include <rtl_conf.h>
#include <rtl_printf.h>
#include <sys/io.h>
//PCI_DRIVER
#include <arch/rtl_io.h>
#include <rtl_pci.h>
#include <arch/pci1710.h>
#include <arch/pci82573v.h>
//PCI_DRIVER ~

struct sk_buff ip_packet;

unsigned char icmp_data[] = {
                               0x00, 0x11, 0x11, 0x6c, 0x71, 0x6c,
                               0x00, 0x1c, 0xf0, 0xa6, 0x27, 0xe6,
                               0x08, 0x00,
                               0x45, 0x00,
                               0x00, 0x54, 0x00, 0x00, 0x40, 0x00, 0x40, 0x01,
                               0x0e, 0x89,
                               0x0a, 0x81, 0x0b, 0xa3,
                               0x0a, 0x81, 0x0b, 0xa2,
0x08, 0x00, 0x7f, 0x37, 0xad, 0x0c,
0x00, 0x01, 0xbc, 0xc1, 0x49, 0x49,
0xd6, 0xac,  0x04, 0x00, 0x08, 0x09,
0x0a, 0x0b, 0x0c, 0x0d,  0x0e, 0x0f,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15,  0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,  0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24,0x25,  0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,  0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,  0x36, 0x37   };

extern unsigned int rhine_intr(unsigned int irq,struct pt_regs *r); 

int init_devices(void)
{
#if DEVICE_I386_TERMINAL 
	init_i386_terminal();
	close(STDOUT);
	STDOUT = open("/dev/rt_tty",0);
#endif
	init_i386_serial();
	return 0;
}

extern void start_kernel(void)
{
	int	i,j,k, dummy =0;

	// interrupt disabled temporarily
	rtl_stop_interrupts();
	setup_arch();   // PIC programming.
	// Mask all interrupts.
	// Interrupts are mapped in 0x20-0x30 IDT entries
#if _RTL_STARTBREAKPOINT 
	BREAKPOINT();
#endif
	init_clocks(); // 8254 currently in periodic mode
#if _RTL_POSIX_IO
	init_posixio();
#endif

	init_devices();
	cpu_init();

	rt_terminal_putstring(" Spark V3 Running     \n",23);
	rt_terminal_putstring(" 08-12-2008 on : Ethernet\n",28);
	copyGuestOS();
//PCI_DEVICE DETECTION
        pci_direct_init();
        //Gives PCI bus architecture and discovers non bridge devices
        pci_scan();
//      print_pci_info(); //For Debugging
//PCI_DEVICE DETECTION ~

	/* Initialize the PCI 1710 Card */
        init_i386_pci1710();

#ifdef SPARK_PCI_ETH_DEVICE
        /* Initialize the PCI 82573v ethernet card */
        init_i386_pci82573v();
        i = rhine_reset_configure();
        if(i != 0)  
        {
            rtl_printf("Ethernet Card not found. \n"); 
        }
        else
        {
            rtl_printf("Ethernet Card Found.\n"); 
            i = rhine_initdevice();
            if(i != 0)
            {
               rtl_printf("%d rhine_initdevice failed\n",i);
            }
        }
#endif
	// init_memory defines the heap area to internal memory allocator
	// heap can be used to construct page tables for SParK 
	init_memory();
	init_page();
	init_sched();
	_i8254_clock.init(&_i8254_clock); // register timer
	rtl_allow_interrupts();

	while(1)
	{
		for (k=0; k< 1; k++)
		{	
			for( i = 0 ; i < 100000; i++){
				for(j = 0 ; j < 100000; j++){
					dummy += 5;
					if(dummy> 50000)
						dummy = 0;	
				}
			}
		}
	}
}
