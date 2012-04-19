/*
 *  i386_pci1710.c
 *
 */

#include <rtl_conf.h>

#include <rtl_devices.h>
#include <rtl_posixio.h>
#include <rtl_sync.h>
#include <arch/pci1710.h>
#include <rtl_pci.h>
#include <arch/rtl_io.h>
#include <arch/vga.h>
#include <arch/rtl_screen.h>
#include <arch/hw_irq.h>
#include <rtl_time.h>
#include <pthread.h>
#include <errno.h>
#include <rtl_printf.h>
#include <dietlibc/sys/io.h>


#define devpriv ((pci1710_private *)pci1710_dev.private)
#define this_board ((boardtypes *)&boardtype)

/* DS For testing only */
#ifdef PCI_DO
#define TEST_DEVICE SUBD_DO
#endif 
#ifdef PCI_DI
#define TEST_DEVICE SUBD_DI
#endif 
#ifdef PCI_AO
#define TEST_DEVICE SUBD_AO
#endif 
#ifdef PCI_AI
#define TEST_DEVICE SUBD_AI
#endif 

struct pci_device_info devinfo[1];

/* PCI 1710 device structure */
struct pci1710_device_struct pci1710_dev;

//static boardtypes boardtype =
boardtypes boardtype =
{
    "pci1710", ADVANTECH_VENDOR_ID, 0x1710,
    IORANGE_1710, 1, TYPE_PCI1710,
    16, 8, 2, 16, 16, 1, 1, 0x0fff, 0x0fff,
    10000, 2048 
};


ssize_t pci1710_dev_read (struct rtl_file *file, char *buffer, size_t size, loff_t *offset)
{
        int ret = 0;

        pci1710_subdevice *s;

	pci_debugpr("Entering %s\n", __FUNCTION__);
	switch(pci1710_dev.minor)
	{
		case SUBD_AI:
			pci_debugpr("%s: Read for AI\n", __FUNCTION__);
			break;
		case SUBD_AO:
			pci_debugpr("%s: Read for AO\n", __FUNCTION__);
			break;
		case SUBD_DI:
			pci_debugpr("%s: Read for DI\n", __FUNCTION__);
			break;
		case SUBD_DO:
			pci_debugpr("%s: Read for DO\n", __FUNCTION__);
			break;
		default:
			pci_debugpr("Unknown subdevice for read\n");
			return -1;
	}

	/* Make s point to appropriate subdevice */
	s = &pci1710_dev.subdevices[pci1710_dev.minor-1];
	ret=s->pci1710_read(s,size,(unsigned int*)buffer);
	if (ret == -1)
	{
		pci_debugpr("%s: Error in read\n", __FUNCTION__);
	}
	return ret;
}


ssize_t pci1710_dev_write (struct rtl_file *file, const char* buffer, 
		size_t size, loff_t *offset)
{
        int ret = 0;
	unsigned int buf = *(unsigned int*)buffer;

        pci1710_subdevice *s;

	pci_debugpr("Entering %s\n", __FUNCTION__);
	switch(pci1710_dev.minor)
	{
		case SUBD_AO:
			pci_debugpr("%s: Write for AO\n", __FUNCTION__);
			break;
		case SUBD_DO:
			pci_debugpr("%s: Write for DO\n", __FUNCTION__);
			break;
		default:
			pci_debugpr("Unknown subdevice for write\n");
			return -1;
	}
	s = &pci1710_dev.subdevices[pci1710_dev.minor - 1];
	ret = s->pci1710_write(s,size,buf);
	if (ret == -1)
	{
		pci_debugpr("%s: Error in write\n", __FUNCTION__);
	}
	return ret;
}

static struct rtl_file_operations rtl_pci1710_fops = {
	NULL,
	pci1710_dev_read,
	pci1710_dev_write,
	NULL,
	NULL,
	NULL,
	NULL
};


/* 
 pci1710_reset ()
 Function:    the pci1710 card reset the function
 Argument:
              dev             the operated Comedi device
 Return value:
              int             wether error occured
 */
int pci1710_reset()
{
	pci_debugpr("Entering %s\n", __FUNCTION__);

        outw(0x30, pci1710_dev.iobase + PCI1710_CNTCTRL);
        devpriv->CntrlReg=Control_SW;   // Software trigger, CNT0=100kHz
        outw(devpriv->CntrlReg, pci1710_dev.iobase+PCI1710_CONTROL);   // reset any operations
        outb(0, pci1710_dev.iobase + PCI1710_CLRFIFO);         // clear FIFO
        outb(0, pci1710_dev.iobase + PCI1710_CLRINT);          // clear INT request
        devpriv->da_ranges=0;
        if (this_board->n_aochan) {
                outb(devpriv->da_ranges, pci1710_dev.iobase+PCI1710_DAREF); // set DACs to 0..5V
                outw(0, pci1710_dev.iobase+PCI1710_DA1);               // set DA outputs to 0V
                devpriv->ao_data[0]=0x0000;
                if (this_board->n_aochan>1) {
                        outw(0, pci1710_dev.iobase+PCI1710_DA2);
                        devpriv->ao_data[1]=0x0000;
                }
        }
        outw(0, pci1710_dev.iobase + PCI1710_DO);              // digital outputs to 0
        outb(0, pci1710_dev.iobase + PCI1710_CLRFIFO);         // clear FIFO
        outb(0, pci1710_dev.iobase + PCI1710_CLRINT);          // clear INT request

	pci_debugpr("Exiting %s\n", __FUNCTION__);
        return 0;

}


/*
 pci1710_ai_cancel ()
 Function:    this function cancle a running AI data sample
 Argument:
              s                       the operated subdevice
 Return value:
              int             wether error occured
 */

int pci1710_ai_cancel(pci1710_subdevice * s)
{
	pci_debugpr("Entering %s\n", __FUNCTION__);

	devpriv->CntrlReg&=Control_CNT0;
	devpriv->CntrlReg|=Control_SW;

	outw(devpriv->CntrlReg, pci1710_dev.iobase+PCI1710_CONTROL);   // reset any operations
	outb(0,pci1710_dev.iobase + PCI1710_CLRFIFO);
	outb(0,pci1710_dev.iobase + PCI1710_CLRINT);

        devpriv->ai_do=0;
        devpriv->ai_act_scan=0;
        devpriv->ai_buf_ptr=0;
        devpriv->neverending_ai=0;

	pci_debugpr("Exiting %s\n", __FUNCTION__);
        return 0;
}

/*
 pci1710_ead_ai ()
 Function:    this function provide a method to read from an AI subdevice
 Argument:
              s               the operated subdevice
              data:   the data read from the subdevice
 Return value:
              type:           int
              description:    the number of data read from the subdevice
 */
int pci1710_read_ai(pci1710_subdevice * s, size_t size, unsigned int *data)
{
        int n,timeout;

	pci_debugpr("Entering %s\n", __FUNCTION__);
        devpriv->CntrlReg&=Control_CNT0;
        devpriv->CntrlReg|=Control_SW;  // set software trigger
        outw(devpriv->CntrlReg, pci1710_dev.iobase+PCI1710_CONTROL);
	pci_debugpr("%s: Control reg configured, for CNT0 and SW interrupt\n", __FUNCTION__);
        outb(0,pci1710_dev.iobase + PCI1710_CLRFIFO);
        outb(0,pci1710_dev.iobase + PCI1710_CLRINT);

	outw(0, pci1710_dev.iobase+PCI1710_SOFTTRG); /* start conversion */
	pci_debugpr("%s: SW trigger provided\n",__FUNCTION__);
	rtl_delay(1000);
	timeout=100;
	while (timeout--) {
		if (!(inw(pci1710_dev.iobase+PCI1710_STATUS) & Status_FE)) goto conv_finish;
		rtl_delay(1000);
	}
	pci_debugpr("%s: A/D instruction timeout\n", __FUNCTION__);
	outb(0,pci1710_dev.iobase + PCI1710_CLRFIFO);
	outb(0,pci1710_dev.iobase + PCI1710_CLRINT);
	*data=0;
	return -ETIME;

conv_finish:
	pci_debugpr("%s: No A/D timeout\n", __FUNCTION__);
	*data = inw(pci1710_dev.iobase+PCI1710_AD_DATA) & 0x0fff;

        outb(0,pci1710_dev.iobase + PCI1710_CLRFIFO);
        outb(0,pci1710_dev.iobase + PCI1710_CLRINT);

	pci_debugpr("Exiting %s\n", __FUNCTION__);
        return 1;
}

/*
 pci1710_write_ao ()
 Function:    this function provide a method to write to an AI subdevice 
 Argument:
              s               the operated subdevice
              data:   the data will be write to the subdevice
 Return value:
              type:           int
              description:    the number of data written
 */
int pci1710_write_ao(pci1710_subdevice * s, size_t size, unsigned int data)
{
        int chan,range,ofs;

	pci_debugpr("Entering %s\n", __FUNCTION__);
        chan=0;		/* Working with channel 0, 1 is also available */
	range = 0; 	/* Check whether internal ref volt is 5 or 10, 0 for 5 */

	devpriv->da_ranges&=0xfe;
	devpriv->da_ranges|=range;
	outw(devpriv->da_ranges, pci1710_dev.iobase+PCI1710_DAREF);
	pci_debugpr("%s: D/A ref register configured for channel 0, range 0-5V\n");
	ofs=PCI1710_DA1;/* D/A register for chan 0 is DA1 */	

	/* Analog output is only 12-bit, hence mask 0x0fff */
	data = data & 0x0fff;
        outw(data, pci1710_dev.iobase + ofs);

        devpriv->ao_data[chan]=data;

	pci_debugpr("Exiting %s\n", __FUNCTION__);
        return 1;
}

/*
 pci1710_read_ao ()
 Function:    this function provide a method to read from an AO subdevice 
 Argument:
              s       the operated subdevice
              data:   the data read from the subdevice
 Return value:
              type:           int
              description:    the number of data read from the subdevice
 */
int pci1710_read_ao(pci1710_subdevice * s, size_t size, unsigned int *data)
{
        int chan;

	pci_debugpr("Entering %s\n", __FUNCTION__);
        chan=0;

        *data=devpriv->ao_data[chan];
 	//pci_debugpr("%s: data = 0x%x\n",__FUNCTION__, *data);
 	//rtl_printf("%s: data = 0x%x\n",__FUNCTION__, *data);

	pci_debugpr("Exiting %s\n", __FUNCTION__);
        return 1;
}



/*
 pci1710_read_di ()
 Function:    this function provide a method to read from an di subdevice 
 Argument:
              s       the operated subdevice
              data:   the data read from the subdevice
 Return value:
              type:           int
              description:    the number of data read from the subdevice
 */
int pci1710_read_di(pci1710_subdevice *s, size_t size, unsigned int *data)
{

	pci_debugpr("Entering %s\n", __FUNCTION__);
        *data = inw(pci1710_dev.iobase + PCI1710_DI);
	//pci_debugpr("%s: data = 0x%x\n",__FUNCTION__, *data);
	//rtl_printf("%s: data = 0x%x\n",__FUNCTION__, *data);

	pci_debugpr("Exiting %s\n", __FUNCTION__);
        return 1;
}



/*
 pci1710_read_do ()
 Function:    this function provide a method to read from an do
                                      subdevice 
 Argument:
              s       the operated subdevice
              data:   the data read from the subdevice
 Return value:
              type:           int
              description:    the number of data read from the subdevice
 */
int pci1710_read_do(pci1710_subdevice *s, size_t size, unsigned int *data)
{

	pci_debugpr("Entering %s\n", __FUNCTION__);
        *data = s->state & 0xffff;
//	pci_debugpr("%s: data = 0x%x\n",__FUNCTION__, *data);
	//rtl_printf("%s: data = 0x%x\n",__FUNCTION__, *data);

	pci_debugpr("Exiting %s\n", __FUNCTION__);
  
        return 1;
}


/*
 pci170_pci1710_write_do ()
 Function:    this function provide a method to write to an do
                                      subdevice 
 Argument:
              s       the operated subdevice
              data:   the data will be write
 Return value:
              type:           int
              description:    the number of data write to the subdevice
*/
int pci1710_write_do(pci1710_subdevice *s, size_t size, unsigned int data)
{

	pci_debugpr("Entering %s\n", __FUNCTION__);
//	pci_debugpr("%s: data = 0x%x\n",__FUNCTION__, data);
	//rtl_printf("%s: data = 0x%x\n",__FUNCTION__, data);

        s->state = data& 0xffff;
        outw(s->state, pci1710_dev.iobase + PCI1710_DO);
	pci_debugpr("%s: s->state = 0x%x\n",__FUNCTION__, s->state);
        
	pci_debugpr("Exiting %s\n", __FUNCTION__);
        return 1;
}


int pci1710_attach()
{
        pci1710_subdevice *s;
        int ret,subdev,i,j;
	unsigned int boardid;
        unsigned int io_addr[6];
        unsigned int iobase;

	pci_debugpr("Entering %s\n", __FUNCTION__);
        pci_debugpr("%s: board=%s",__FUNCTION__,this_board->name);

        ret = get_device_info(ADVANTECH_VENDOR_ID, this_board->device_id,
				1,&devinfo[0]);
        pci_debugpr("%s: Driver: pci info of ADVANTEC device 0:\n", __FUNCTION__);
        pci_debugpr("%s: Num of mem regions %d\n", __FUNCTION__,devinfo[0].num_mem);
        pci_debugpr("%s: Num of io regions %d\n", __FUNCTION__,devinfo[0].num_io);
        for(j=0; j < devinfo[0].num_mem; j++)
        {
                pci_debugpr("%s: mem region at 0x%x of size 0x%x\n", __FUNCTION__,devinfo[0].mem_addr[j],devinfo[0].mem_size[j]);
        }
        for(j=0; j < devinfo[0].num_io ; j++)
        {
                pci_debugpr("%s: io region at 0x%x of size 0x%x\n", __FUNCTION__,devinfo[0].io_addr[j],devinfo[0].io_size[j]);
        }
        pci_debugpr("%s: Inter line 0x%x\n", __FUNCTION__,devinfo[0].intr_line);
        pci_debugpr("%s: Inter pin 0x%x\n", __FUNCTION__,devinfo[0].intr_pin);


        iobase=devinfo[0].io_addr[0];
        pci1710_dev.iobase=iobase-1;

//        rtl_printf("\nDS ************ iobase=0x%x\n",pci1710_dev.iobase);
	boardid = inw(pci1710_dev.iobase + 20);
  //      rtl_printf("DS ************ board id=0x%x\n",boardid);
 	
        iobase=devinfo[0].io_addr[1];
        pci1710_dev.iobase=iobase-1;

    //    rtl_printf("\nDS ************ iobase=0x%x\n",pci1710_dev.iobase);
	boardid = inw(pci1710_dev.iobase + 20);
      //  rtl_printf("DS ************ board id=0x%x\n",boardid);
 	
        pci1710_dev.board_name = this_board->name;
        pci1710_dev.n_subdevices = 0;
        if (this_board->n_aichan) pci1710_dev.n_subdevices++;
        if (this_board->n_aochan) pci1710_dev.n_subdevices++;
        if (this_board->n_dichan) pci1710_dev.n_subdevices++;
        if (this_board->n_dochan) pci1710_dev.n_subdevices++;
        
	/* This board has AI channels
	 *
	 */
        if (this_board->n_aichan) {
                s = &pci1710_dev.subdevices[SUBD_AI-1];
                s->type = SUBD_AI;
                s->subdev_flags = SDF_READABLE|SDF_COMMON|SDF_GROUND;
                if (this_board->n_aichand) s->subdev_flags |= SDF_DIFF;
                s->n_chan = this_board->n_aichan;
                s->maxdata = this_board->ai_maxdata;
                s->len_chanlist = this_board->n_aichan;
                s->cancel=pci1710_ai_cancel;
                s->pci1710_read=pci1710_read_ai;
                devpriv->i8254_osc_base=100;    // 100ns=10MHz
        }


        if (this_board->n_aochan) {
                s = &pci1710_dev.subdevices[SUBD_AO -1];
                s->type = SUBD_AO;
                s->subdev_flags = SDF_WRITEABLE|SDF_GROUND|SDF_COMMON;
                s->n_chan = this_board->n_aochan;
                s->maxdata = this_board->ao_maxdata;
                s->len_chanlist = this_board->n_aochan;
                s->pci1710_write=pci1710_write_ao;
                s->pci1710_read=pci1710_read_ao;
        }

        if (this_board->n_dichan) {
                s = &pci1710_dev.subdevices[SUBD_DI - 1];
                s->type = SUBD_DI;
                s->subdev_flags = SDF_READABLE|SDF_GROUND|SDF_COMMON;
                s->n_chan = this_board->n_dichan;
                s->maxdata = 65535;
                s->len_chanlist = this_board->n_dichan;
                s->io_bits=0;           /* all bits input */
                s->pci1710_read=pci1710_read_di;
        }

        if (this_board->n_dochan) {
                s = &pci1710_dev.subdevices[SUBD_DO - 1];
                s->type = SUBD_DO;
                s->subdev_flags = SDF_WRITEABLE|SDF_GROUND|SDF_COMMON;
                s->n_chan = this_board->n_dochan;
                s->maxdata = 65535;
                s->len_chanlist = this_board->n_dochan;
                s->io_bits=(1 << this_board->n_dochan)-1;       /* all bits output */
                s->state=0;
                s->pci1710_read=pci1710_read_do;
                s->pci1710_write=pci1710_write_do;
        }

        devpriv->valid=1;

        pci1710_reset();
	pci_debugpr("Exiting %s\n", __FUNCTION__);

        return 0;
}

int pci1710_detach()
{
	pci_debugpr("Entering %s\n", __FUNCTION__);
        if (devpriv->valid) pci1710_reset();

	pci_debugpr("Exiting %s\n", __FUNCTION__);
        return 0;
}


int init_i386_pci1710 (void)
{

        int rc;
        int minor;
        int use_count;

	pci_debugpr("Entering %s\n", __FUNCTION__);
        if (rtl_register_chrdev (I386_PCI1710_MAJOR, I386_PCI1710_DEVICE_NAME,
                                &rtl_pci1710_fops))
        {
                pci_debugpr("%s : i386_pci1710: unable to get major %d\n",
                                __FUNCTION__, I386_PCI1710_MAJOR);
                return -EIO;
        }
        memset(&pci1710_dev,0,sizeof(pci1710_device));

        if(pci1710_dev.attached)
                return -EBUSY;

        minor = pci1710_dev.minor;
        memset(&pci1710_dev,0,sizeof(pci1710_device));
        pci1710_dev.minor = minor;

	/* Invoke the device attach() function here */
	rc = pci1710_attach();
	if (rc<0)
	{
		pci1710_detach();
		return rc;
	}

	/* board_name should get filled in attach() call */
        if(!pci1710_dev.board_name){
                pci_debugpr("BUG: pci1710_dev.board_name=<%p>\n",
					pci1710_dev.board_name);
                pci1710_dev.board_name="BUG";
        }

	pci1710_dev.attached=1;

	pci_debugpr("Exiting %s\n", __FUNCTION__);
	return 0;
}

void close_i386_pci1710 (void) 
{
	pci_debugpr("Entering %s\n", __FUNCTION__);
//	rtl_stop_interrupts ();

        rtl_unregister_chrdev(I386_PCI1710_MAJOR,I386_PCI1710_DEVICE_NAME);
	pci_debugpr("%s: Unregistered pci1710 driver\n");

	if(pci1710_dev.attached)
		pci1710_detach();

	pci_debugpr("Exiting %s\n", __FUNCTION__);
//	rtl_allow_interrupts ();
}



