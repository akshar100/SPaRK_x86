/*
* Developed at IITB, ERTS LAB
*
*/

#include <rtl_conf.h>
#include <arch/errno.h>
#include <arch/linkage.h>
#include <rtl_printf.h>
#include <guests.h>
#include <rtl_sched.h>
#include <arch/processor.h>
#include <arch/hw_irq.h>
#include <sys/io.h>
#include <arch/serial.h>
#include <arch/kbd.h>
#include <rtl_mq.h>
#include <rtl_time.h>
#include <rtl_printf.h>
//PCI_DRIVER
#include <rtl_posixio.h>
#include <arch/pci1710.h>
#include <arch/pci82573v.h>
/* PCI 1710 device structure */
extern struct pci1710_device_struct pci1710_dev;
//PCI_DRIVER ~

unsigned char stackdata[1540];

//extern int iTick;
extern void timer_handler( struct pt_regs *regs);
extern void enable_pagebit(void); 
/**********************************************************************/
asmlinkage long spark_ni_hypercall(void)
{
	rtl_printf("Hyper call not implemented\n");
	return -ENOSYS;
}
/**********************************************************************/

asmlinkage long spark_printLong(unsigned long l)
{
	//rtl_printf("spark_printLong:%u\t0x%x\n" , l , l);
	rtl_printf("%u\t0x%x\n" , l , l);
	return 0;	
}
/**********************************************************************/

asmlinkage long spark_print(char *str)
{
#ifdef _USE_SEGMENTATION_FOR_PROT_  
	str = str + guestOS_thread[iCurrGuestOsIndex].phyOffset;
#endif
	rtl_printf("%s",str);
	return 0;
}
/**********************************************************************/
asmlinkage long spark_registerTimer(int iPeriod , int iHandler, unsigned long puRealTime)
{
	guestOS_thread[iCurrGuestOsIndex].iHandler = iHandler;
	guestOS_thread[iCurrGuestOsIndex].iTicks = iPeriod / TIMER_PERIOD;
	guestOS_thread[iCurrGuestOsIndex].iCurrTicks = 0;
	guestOS_thread[iCurrGuestOsIndex].pllRealTime = puRealTime + guestOS_thread[iCurrGuestOsIndex].phyOffset;
	return 0;
}

/**********************************************************************/

asmlinkage long spark_registerSysCallHandler(int *handler_address  , int intr_number)
{
	// set_system_gate3(SYSCALL_VECTOR,handler_address);
	set_system_gate3( intr_number, handler_address);
	return 0;
}


/**********************************************************************/

asmlinkage long spark_registerPages(int iProcessId , void *baseAddr ) 
//asmlinkage long spark_registerPages(int iProcessId , struct task_data *baseAddr ) 
{
	if (iProcessId >= MAX_TASK_PER_GUEST)
		return EINVAL;

	baseAddr = baseAddr + guestOS_thread[iCurrGuestOsIndex].phyOffset;
	//guestOS_thread[iCurrGuestOsIndex].stack_start_level2 =  baseAddr->esp2; 
	return	verifyAndCopyPages(iProcessId , baseAddr);
}
/**********************************************************************/

asmlinkage long spark_loadPDE(int iProcessId)
{
	if (iProcessId >= MAX_TASK_PER_GUEST)
		return EINVAL;

	guestOS_thread[iCurrGuestOsIndex].iCurrTaskId = iProcessId;
	load_pde(iProcessId);	
	return 0;
}

/**********************************************************************/
asmlinkage long spark_setLevel2Stack(unsigned long l2Stack)
{
	guestOS_thread[iCurrGuestOsIndex].stack_start_level2 = l2Stack;// + guestOS_thread[iCurrGuestOsIndex].phyOffset;

	//rtl_printf("stack_start_level2: 0x%x\n" , l2Stack);
	fLoadEsp2();

}


/**********************************************************************/
// pointer to 
int *kbd_data_gos;
int kbd_guest_os = -1;
//unsigned int *kbd_gos_isr;

asmlinkage long spark_register_interrupt(int int_num, int loc)
{
	switch	(int_num)
	{

		case 1: //keyboard interrupt
			kbd_data_gos = (int *)(loc + guestOS_thread[iCurrGuestOsIndex].phyOffset);
			//kbd_gos_isr = int_handler;
			init_i386_kbd();
			kbd_guest_os = iCurrGuestOsIndex;
			break;			

		case 4:	// serial interrupt

			break;

		default:
			;
	

	}
}


/**********************************************************************/

asmlinkage long spark_parallel_out(unsigned char bData)
{
	outb(bData , PARALLEL_DATA);
}
/**********************************************************************/

asmlinkage long spark_serial_out(char *str)
{

#ifdef _USE_SEGMENTATION_FOR_PROT_  
	str = str + guestOS_thread[iCurrGuestOsIndex].phyOffset;
#endif

	return	serial_out(str);

#if 0
	int i = 0;
	unsigned char c=0;

#ifdef _USE_SEGMENTATION_FOR_PROT_  
	str = str + guestOS_thread[iCurrGuestOsIndex].phyOffset;
#endif

	for(i=0; str[i] != '\0';)
	{
		// SER_PORT + 5 points line status Register 
		c = inb (SER_PORT + 5);  
		if (c & 0x20) // check empty transmitter bit
		{
			c = 0;
			outb(str[i], SER_PORT);
			i++;
		}
	}

	return 0;
#endif

}

/**********************************************************************/

asmlinkage long spark_initFinish(void)
{
	//rtl_printf("Guest %d: spark_initFinish:iTick %d\n" , (iCurrGuestOsIndex+1) , iTick);
	//iTick = 399;
	// _i8254_clock.handler = timer_handler;
	// spark_schedule();
	return 0;
}

/**********************************************************************/

/*Required for IPC using message queue implementation*/
static int no_init_queue = 0; //counter to track no of queues initialized
MSG_QUEUE msq_queue_arry[MAX_QUEUES];

// ulMemSize will decide the size of buffer
asmlinkage long spark_registerMQ (unsigned long ulChannelID, unsigned long ulMemSize)
{
	int i;

	if(no_init_queue == (MAX_QUEUES - 1))
		rtl_printf("Max No of Queues already initialized \n");

	else
	{
		for (i=0; i<no_init_queue; i++)	
		{
			if (msq_queue_arry[i].ulChannelID == ulChannelID )
			{
				//rtl_printf("channel is already registered\n");
				return 0;
			}
		}	

		//Get a new memory page for this queue
		char *new_page;
		new_page = kGetPage();
		memset(new_page, 0, RTL_PAGE_SIZE);

		//Initialize the page, current read, and current write pointers
		msq_queue_arry[no_init_queue].ulChannelID = ulChannelID;
		msq_queue_arry[no_init_queue].start_buff = new_page;
		msq_queue_arry[no_init_queue].end_buff = new_page + ulMemSize - 1;
		msq_queue_arry[no_init_queue].current_read_ptr = new_page;
		msq_queue_arry[no_init_queue].current_write_ptr = new_page;
		msq_queue_arry[no_init_queue].total_size = ulMemSize;
		msq_queue_arry[no_init_queue].q_filled_bytes = 0;
		//rtl_printf("****Inside mq_register function*****\n");
		//rtl_printf("spark_registerMQ queue no = %d , Channel ID = %u \n", no_init_queue, ulChannelID);
		no_init_queue++;
	}
	return 0;
}

/**********************************************************************/
// IPC routines are tried to be made as generic as possible
// spark_send will write the data in buffer, if it tries to overwrite the data 
// which is not read till now the queue will be reset, this is done because
// we don't want to loose any data and once the write_ptr crosses read ptr
// then there is no point in keeping the old data
// Returns 0 in case of error else number of data written
asmlinkage long spark_send (unsigned long ulChannelID, char *msg, unsigned
		long ulDataSize)
{
	int array_index, diff;
	char *write_ptr;

	for(array_index = 0; msq_queue_arry[array_index].ulChannelID != ulChannelID; array_index++ );

	// check that the size of the data that is to be written should be smaller then the buffer size
	if (ulDataSize > msq_queue_arry[array_index].total_size)
		return 0;

#ifdef _USE_SEGMENTATION_FOR_PROT_
	msg = msg + guestOS_thread[iCurrGuestOsIndex].phyOffset;
#endif
	write_ptr = msq_queue_arry[array_index].current_write_ptr;

	if (msq_queue_arry[array_index].q_filled_bytes + ulDataSize > msq_queue_arry[array_index].total_size)
		write_ptr = msq_queue_arry[array_index].current_read_ptr = msq_queue_arry[array_index].start_buff;

	// space left to write
	diff = msq_queue_arry[array_index].end_buff - write_ptr + 1;
	if (diff >= ulDataSize)
	{
		memcpy(write_ptr, msg, ulDataSize);
		write_ptr += ulDataSize;
		if (write_ptr > msq_queue_arry[array_index].end_buff)
			write_ptr = msq_queue_arry[array_index].start_buff;
	}
	else
	{
		memcpy(write_ptr, msg, diff);
		msg+=diff;
		memcpy(msq_queue_arry[array_index].start_buff, msg, ulDataSize - diff);
		write_ptr = msq_queue_arry[array_index].start_buff + (ulDataSize - diff);
	}

	msq_queue_arry[array_index].current_write_ptr = write_ptr;
	msq_queue_arry[array_index].q_filled_bytes += ulDataSize;
	return ulDataSize;
}

/**********************************************************************/
// Returns 0 in case does not have enough bytes to read
// else returns ulDataSize
asmlinkage long spark_receive (unsigned long ulChannelID, char *msg, unsigned
		long ulDataSize)
{
	int array_index, diff;
	char *read_ptr;
	for(array_index = 0; msq_queue_arry[array_index].ulChannelID != ulChannelID; array_index++ );

	// check that the size of data that is to be read should be smaller than the buffer size
	if (ulDataSize > msq_queue_arry[array_index].q_filled_bytes)
		return (msq_queue_arry[array_index].q_filled_bytes);

#ifdef _USE_SEGMENTATION_FOR_PROT_
	msg = msg + guestOS_thread[iCurrGuestOsIndex].phyOffset;
#endif
	read_ptr = msq_queue_arry[array_index].current_read_ptr;

	// Bound Check - Check to see if read is beyound page boundary
	diff = msq_queue_arry[array_index].end_buff - read_ptr + 1;
	if (diff >= ulDataSize)
	{
		memcpy(msg, read_ptr, ulDataSize);
		read_ptr += ulDataSize;
		if (read_ptr > msq_queue_arry[array_index].end_buff)
			read_ptr = msq_queue_arry[array_index].start_buff;
	}
	else
	{
		memcpy(msg, read_ptr, diff);
		memcpy(msg+diff, msq_queue_arry[array_index].start_buff, ulDataSize - diff);
		read_ptr = msq_queue_arry[array_index].start_buff + (ulDataSize - diff);
	}
	msq_queue_arry[array_index].q_filled_bytes -= ulDataSize;
	// save read ptr
	msq_queue_arry[array_index].current_read_ptr = read_ptr;
	return ulDataSize;
}
/****************************************************************************/
//ETHERNET_DRIVER
long spark_rhine_get_ip(char *buffer)
{
        return rhine_get_ip((unsigned char *)((unsigned long)buffer + guestOS_thread[iCurrGuestOsIndex].phyOffset));
}

long spark_rhine_transmit(char *buffer, unsigned int size)
{
        int iProcessId;
        memcpy(stackdata, (unsigned char *)((unsigned long)buffer + guestOS_thread[iCurrGuestOsIndex].phyOffset), size);
        enable_pagebit(); 
	rhine_xmit(stackdata, size);
        iProcessId = guestOS_thread[iCurrGuestOsIndex].iCurrTaskId;
        load_pde(iProcessId);
	return 0;
}

long spark_rhine_isready()
{
        return rhine_isready();
}

long spark_rhine_receive(const char* buffer,unsigned long * ethhdr_type)
{
        return rhine_receive(buffer + guestOS_thread[iCurrGuestOsIndex].phyOffset, ((char*)ethhdr_type) + guestOS_thread[iCurrGuestOsIndex].phyOffset);
}

asmlinkage long spark_rhine_poll(void)
{
/* If the device is working with interrupts then this function should
 * not do anything 
 */
#if ETH_INTR_BASED
        return 0;
/* If the device is working with interrupts then this function should
 * call the rhine_intr routing explicitly so that the data from hardware
 * buffer gets copied into the software buffer 
 */
#else
        return rhine_intr();
#endif
/* Note : Actually in interrupt mode the function should not get called at all.
 * But then that would require keeping such a marco in guest OS also.
 * wanted to avoid that. So did the implementation this way
 */
}

/**********************************************************************/

//PCI_DRIVER ~

// MODBUS..

//gk080327..
typedef struct serIn
{
	unsigned char	cData;
	hrtime_t	tStamp; // long long
}serIn;
#define	MAX_SMOD_BUFF	8
#define QLEN		8

#define	BUFFER_READY	0
#define	BUFFER_BUSY	1

extern serIn sInBuff[QLEN];
extern	int	bModFlag;

//gk080327_

/**********************************************************************/
// piSize : pointer which needs to be updated with size of filled array
asmlinkage long spark_readModbus (void *szRData, unsigned int *piSize)
{
#if 0
	int i=0;
	serIn* szRData1;
	
// add guest offset to SParK's address
#ifdef _USE_SEGMENTATION_FOR_PROT_  
	szRData = (unsigned long)szRData + guestOS_thread[iCurrGuestOsIndex].phyOffset;
	piSize = (unsigned long)piSize + guestOS_thread[iCurrGuestOsIndex].phyOffset;
#endif
	szRData1 = (serIn*)szRData;

	if (BUFFER_BUSY == bModFlag)
	{
		for (i=0; i<QLEN; i++) 
		{
			szRData1[i].cData = sInBuff[i].cData; //Modbus Query
			szRData1[i].tStamp = sInBuff[i].tStamp; //Clock Tick at recv
		}
		*piSize = QLEN;
		bModFlag = BUFFER_READY;
	}
	else
		*piSize = 0;
#endif
	return 0;
}

/**********************************************************************/
asmlinkage long spark_writeModbus (void *szWData, unsigned int iSize)
{
#if 0
	unsigned char	*szWData1;
	int	i = 0;
#ifdef _USE_SEGMENTATION_FOR_PROT_  
	szWData1 = (unsigned char *)((unsigned long)szWData + guestOS_thread[iCurrGuestOsIndex].phyOffset);
#endif
	//gk080407
	//rtl_printf("String received : %s\n", szWData1);
	//rtl_printf("\nWdata=%x\t%d\t iSize=%d\n", szWData1[0],szWData1[0],iSize);
	serial_outModbus(szWData1, iSize); //write character to serial port
#endif
	return 0;
}
/**********************************************************************/
extern	unsigned long scaler_pentium_to_hrtime;
asmlinkage long spark_enableModbusTimer (unsigned long *ptrScaler)
{
#if 0
#ifdef _USE_SEGMENTATION_FOR_PROT_  
	ptrScaler = (unsigned long *)((unsigned long)ptrScaler + guestOS_thread[iCurrGuestOsIndex].phyOffset);
#endif
	*ptrScaler = scaler_pentium_to_hrtime;
#endif
	return 0;
}
/**********************************************************************/
#ifdef PERF_ANALYSIS_ON
/* Code inserted for demo START */
extern struct part_switch_perf partition_switch_performace;
extern struct sched_decision_perf schedule_decision_performace;
/* Code inserted for demo END */        
#endif
 
asmlinkage long spark_perf_measurement(unsigned long parameter, unsigned long preading)
{
        unsigned long temp = 0;
#ifdef PERF_ANALYSIS_ON
/* Code inserted for demo START */
        switch(parameter)
        {
             case 0x11:
             *((hrtime_t *)(((unsigned long)preading+ guestOS_thread[iCurrGuestOsIndex].phyOffset))) = partition_switch_performace.old_avg;
             break;
             case 0x22:
             *((hrtime_t *)(((unsigned long)preading+ guestOS_thread[iCurrGuestOsIndex].phyOffset))) = schedule_decision_performace.old_avg;
             break;
             case 0x33:
             temp = *(unsigned long *)((unsigned long)preading+ guestOS_thread[iCurrGuestOsIndex].phyOffset);
             temp = kill_gos(temp);
             break;
             case 0x44:
             temp = current_gos_schedule(((unsigned long)preading+ guestOS_thread[iCurrGuestOsIndex].phyOffset));
             break;
             default:
             *((hrtime_t *)(((unsigned long)preading+ guestOS_thread[iCurrGuestOsIndex].phyOffset))) = 0xFFFFFFFF;
             break;
        }
#endif
	return temp;
}

/**********************************************************************/
// MODBUS~
