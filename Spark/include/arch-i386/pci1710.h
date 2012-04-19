/*
 * pci1710.h 
 *
 * This file has several functions from deblib project.
 * Thanks to Vicente Esteve
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) April, 2003 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

#ifndef _PCI1710_H_
#define _PCI1710_H_

#include <rtl_core.h>

//#define PCI_DEBUG
#ifdef PCI_DEBUG
#define pci_debugpr(format, args...)        rtl_printf(format, ## args)
#else
#define pci_debugpr(format, args...)        do { ; } while (0)
#endif

/* DS - - For testing only */
//#define PCI_DO
#define PCI_DI
//#define PCI_AO
//#define PCI_AI


/* max length of device and driver names */
#define PCI1710_NAMELEN 20

/* hardware type of the card */
#define TYPE_PCI1710    0

#define IORANGE_1710    32


#define PCI1710_AD_DATA  0              /* R:   A/D data */
#define PCI1710_SOFTTRG  0              /* W:   soft trigger for A/D */
#define PCI1710_RANGE    2              /* W:   A/D gain/range register */
#define PCI1710_MUX      4              /* W:   A/D multiplexor control */
#define PCI1710_STATUS   6              /* R:   status register */
#define PCI1710_CONTROL  6              /* W:   control register */
#define PCI1710_CLRINT   8              /* W:   clear interrupts request */
#define PCI1710_CLRFIFO  9              /* W:   clear FIFO */
#define PCI1710_DA1     10              /* W:   D/A register */
#define PCI1710_DA2     12              /* W:   D/A register */
#define PCI1710_DAREF   14              /* W:   D/A reference control */
#define PCI1710_DI      16              /* R:   digi inputs */
#define PCI1710_DO      16              /* W:   digi outputs */

#define PCI1710_CNTCTRL 30              /* W:   8254 counter control */

/* bits from control register (PCI171x_CONTROL) */
#define Control_CNT0    0x0040          /* 1=CNT0 have external source
					 * 0=have internal 100kHz source 
					 */
#define Control_SW      0x0001          /* 1=enable software trigger source */

#define Status_FE       0x0100          /* 1=FIFO is empty */
#define Status_FH       0x0200          /* 1=FIFO is half full */
#define Status_FF       0x0400          /* 1=FIFO is full, fatal error */

/* subdevice flags */

#define SDF_BUSY        0x0001          /* device is busy */
#define SDF_BUSY_OWNER  0x0002          /* device is busy with your job */
#define SDF_LOCKED      0x0004          /* subdevice is locked */
#define SDF_LOCK_OWNER  0x0008          /* you own lock */
#define SDF_MAXDATA     0x0010          /* maxdata depends on channel */
#define SDF_FLAGS       0x0020          /* flags depend on channel */
#define SDF_RANGETYPE   0x0040          /* range type depends on channel */
#define SDF_MODE0       0x0080          /* can do mode 0 */
#define SDF_MODE1       0x0100          /* can do mode 1 */
#define SDF_MODE2       0x0200          /* can do mode 2 */
#define SDF_MODE3       0x0400          /* can do mode 3 */
#define SDF_MODE4       0x0800          /* can do mode 4 */
#define SDF_CMD         0x1000          /* can do commands */

#define SDF_READABLE    0x00010000      /* subdevice can be read (e.g. analog input) */
#define SDF_WRITEABLE   0x00020000      /* subdevice can be written (e.g. analog output) */
#define SDF_INTERNAL    0x00040000      /* subdevice does not have externally visible lines */
#define SDF_RT          0x00080000      /* subdevice is RT capable */
#define SDF_GROUND      0x00100000      /* can do aref=ground */
#define SDF_COMMON      0x00200000      /* can do aref=common */
#define SDF_DIFF        0x00400000      /* can do aref=diff */
#define SDF_OTHER       0x00800000      /* can do aref=other */
#define SDF_DITHER      0x01000000      /* can do dithering */
#define SDF_DEGLITCH    0x02000000      /* can do deglitching */
#define SDF_MMAP        0x04000000      /* can do mmap() */
#define SDF_RUNNING     0x08000000      /* subdevice is acquiring data */
#define SDF_LSAMPL      0x10000000      /* subdevice uses 32-bit samples */
#define SDF_PACKED      0x20000000      /* subdevice can do packed DIO */

/* subdevice number */
/* Currently we support only 4 device types AI, AO, DI and DO and we do not
 * use malloc(). Hence assigned a fixed number of subdevices below as 4
 * In order to add more devices like timer and counter, change the define below
 */
#define SUBD_NUM 		 4	 /* number of subdevices supported */

/* subdevice types */

#define SUBD_UNUSED              0       /* unused */
#define SUBD_AI                  1       /* analog input */
#define SUBD_AO                  2       /* analog output */
#define SUBD_DI                  3       /* digital input */
#define SUBD_DO                  4       /* digital output */
#define SUBD_DIO                 5       /* digital input/output */
#define SUBD_COUNTER             6       /* counter */
#define SUBD_TIMER               7       /* timer */
#define SUBD_MEMORY              8       /* memory, EEPROM, DPRAM */
#define SUBD_CALIB               9       /* calibration DACs */
#define SUBD_PROC                10      /* processor, DSP */
#define SUBD_FAKE                15      /* fake subd for di interrupt*/


typedef struct pci1710_device_struct pci1710_device;
typedef struct pci1710_subdevice_struct pci1710_subdevice;
typedef struct pci1710_lrange_struct pci1710_lrange;

typedef struct pci_device_struct pci_device;
struct pci_device_struct{
        unsigned int    io_addr[5];
        unsigned int    irq;
};	

struct pci1710_subdevice_struct
{
        int type;
        int n_chan;
        int subdev_flags;
        int len_chanlist;             /* maximum length of channel/gain list */

        void *private;

        void *lock;
        void *busy;
        unsigned int runflags;

        int io_bits;

        unsigned int maxdata;               /* if maxdata==0, use list */
        unsigned int *maxdata_list;         /* list is channel specific */

        unsigned int flags;
        unsigned int *flaglist;

        pci1710_lrange *range_table;
        pci1710_lrange **range_table_list;

        unsigned int *chanlist;         /* driver-owned chanlist (not used) */

        int (*pci1710_read)(pci1710_subdevice *,size_t,unsigned int *);
        int (*pci1710_write)(pci1710_subdevice *,size_t,unsigned int);
        int (*cancel)(pci1710_subdevice *);

        int (*buf_change)(pci1710_device *,pci1710_subdevice *s);

        unsigned int state;
};


struct pci1710_device_struct{
        void *private;
        int minor;
        char *board_name;
        int attached;

        int n_subdevices;
        pci1710_subdevice subdevices[SUBD_NUM];

        unsigned int iobase;
        
};

typedef struct {
        char            *name;          // driver name
        int             vendor_id;      // PCI vendor a device ID of card
        int             device_id;
        int             iorange;        // I/O range len
        char            have_irq;       // 1=card support IRQ
        char            cardtype;       // 0=1710& co. 2=1713, ...
        int             n_aichan;       // num of A/D chans
        int             n_aichand;      // num of A/D chans in diff mode
        int             n_aochan;       // num of D/A chans
        int             n_dichan;       // num of DI chans
        int             n_dochan;       // num of DO chans
        int             n_cntchan;      // num of couter chans
        int             n_tmrchan;      // num of timer chans
        int             ai_maxdata;     // resolution of A/D
        int             ao_maxdata;     // resolution of D/A
        unsigned int    ai_ns_min;      // max sample speed of card v ns
        unsigned int    fifo_half_size; // size of FIFO/2
} boardtypes;

typedef struct{
        char                    valid;          // card is usable
        char                    neverending_ai; // we do unlimited AI
        unsigned int            CntrlReg;       // Control register
        unsigned int            i8254_osc_base; // frequence of onboard oscilator
        unsigned int            ai_do;          // what do AI? 0=nothing, 1 to 4 mode
        unsigned int            ai_act_scan;// how many scans we finished
        unsigned int            ai_act_chan;// actual position in actual scan
        unsigned int            ai_buf_ptr;     // data buffer ptr in samples
        unsigned char           ai_eos;         // 1=EOS wake up
        unsigned char           da_ranges;      // copy of D/A outpit range register
        unsigned int            ai_scans;       // len of scanlist
        unsigned int            ai_n_chan;      // how many channels is measured
        unsigned int            *ai_chanlist;   // actaul chanlist
        unsigned int            ai_flags;       // flaglist
        unsigned int            ai_data_len;    // len of data buffer
        unsigned short		*ai_data;       // data buffer
        unsigned short		ao_data[4];     // data output buffer
} pci1710_private;

int init_i386_pci1710 (void);
void close_pci1710 (void);
int pci1710_dev_open (struct rtl_file *file);
int pci1710_dev_release (struct rtl_file *file);
int pci1710_read_do(pci1710_subdevice *s, size_t size, unsigned int *data);



#endif /* _PCI1710_H_ */

