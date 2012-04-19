#include <rtl_sync.h>
#include <rtl_core.h>
#include <rtl_printf.h>
#include <arch/serial.h>
#include <arch/hw_irq.h>
// #include <arch/rtl_io.h>
#include <sys/io.h>
#include <rtl_spinlock.h>
#include <rtl_time.h>

// MODBUS..
// gk080426..

struct serIn
{
	unsigned char	cData;
	hrtime_t	tStamp; // long long
};

#define	BUFFER_READY	0
#define	BUFFER_BUSY	1

#define MODRTU		1
#define	MAX_SMOD_BUFF	8
struct serIn sInBuff[MAX_SMOD_BUFF];
struct serIn sCurrBuff[MAX_SMOD_BUFF];

int	bModFlag = BUFFER_READY;

long serial_outModbus(unsigned char *str, unsigned char size);

// gk080426~
// MODBUS~


static unsigned int _serial_irq(unsigned int, struct pt_regs *);

static spinlock_t lock_serial;

// #define	MODBUS_DEBUG

int	init_i386_serial(void)
{
 	int flags;

	rtl_no_interrupts (flags);
	rtl_spin_lock_init (&lock_serial);
	rtl_request_global_irq(4, _serial_irq);
  	enable_8259_irq(4);
	rtl_restore_interrupts (flags);

	outb (0x83, SER_PORT1 + 3);
	// outb (0x0c, SER_PORT1 + 0); /* Set Baud rate-Divisor latch low byte */
	outb (0x02, SER_PORT1 + 0); // for baud 57600
	// outb (0x00, SER_PORT1 + 1); // for baud 19200
	outb (0x03, SER_PORT1 + 3); /* 8 bits, no parity, 1 stop bit */
	outb (0x08, SER_PORT1 + 4); /* Set OUT2 bit */
	outb (0x01, SER_PORT1 + 1); /* Interrupt when data received */

#ifdef	MODBUS_DEBUG
	sInBuff[0].cData = 0x01;
	sInBuff[0].tStamp = gethrtime();
	sInBuff[1].cData = 0x04;
	sInBuff[1].tStamp = gethrtime();
	sInBuff[2].cData = 0x00;
	sInBuff[2].tStamp = gethrtime();
	sInBuff[3].cData = 0x01;
	sInBuff[3].tStamp = gethrtime();
	sInBuff[4].cData = 0x00;
	sInBuff[4].tStamp = gethrtime();
	sInBuff[5].cData = 0x02;
	sInBuff[5].tStamp = gethrtime();
	sInBuff[6].cData = 0x0b;
	sInBuff[6].tStamp = gethrtime();
	sInBuff[7].cData = 0x20;
	sInBuff[7].tStamp = gethrtime();
#endif
	return 0;
}

long serial_out(char *str)
{
	int i = 0;
	unsigned char c=0;

	for(i=0; str[i] != '\0';)
	{
		// SER_PORT1 + 5 points line status Register 
		c = inb (SER_PORT1 + 5);  
		if (c & 0x20) // check empty transmitter bit
		{
			c = 0;
			outb(str[i], SER_PORT1);
			i++;
		}
	}

	return 0;
}

static unsigned int _serial_irq(unsigned int irq, struct pt_regs *regs)
{
	int flags;
	static int iCount=0;
	int	j=0;
	unsigned char c=0;

	// rtl_spin_lock_irqsave(&lock_serial, flags);

	// handle interrupts here
	c = inb (SER_PORT1 + 5);
	if (c & 0x1) // check receive bit
	{
#if	MODRTU
		sCurrBuff[iCount].cData= inb(SER_PORT1);
		sCurrBuff[iCount].tStamp= gethrtime(); // _i8254_clock.value;

		iCount=iCount+1;

		if (iCount>=MAX_SMOD_BUFF) {
			// rtl_printf("SParK: Caught 8 bytes!\n");
			if( BUFFER_READY == bModFlag )
			{
				for(j=0;j<MAX_SMOD_BUFF;j++)
				{
					sInBuff[j].cData = sCurrBuff[j].cData;
					sInBuff[j].tStamp = sCurrBuff[j].tStamp;	
				}
				bModFlag = BUFFER_BUSY;
			}
			iCount=0;
		}
#else
		c = inb(SER_PORT1);
		rtl_printf("SParK: Caught serial INT with data %d!!\n",c);
#endif
	}
	else
		rtl_printf("SParK: Caught serial interrupt no data!!\n");
		
	// rtl_spin_unlock_irqrestore(&lock_serial, flags);

	return 0;
}

// MODBUS..
//gk 080407
long serial_outModbus(unsigned char *str, unsigned char size)
{
	int i = 0;
	unsigned char c=0;

	for(i=0; i<size; i++) 
	{
		do{
			c = 0;
			c = inb (SER_PORT1 + 5); //Read LSR (Adr=Base+5)
			c=(c&0x20); //>>6; //check empty Tx buffer
		}while(!c);
  		
		//rtl_printf("\n");
		rtl_printf("\tSer data out=%x\t",str[i]);
		outb(str[i], SER_PORT1); //send Data
	}

	return 0;
}
// MODBUS~
//gk080407
