#ifndef __RTL_PCI_H
#define __RTL_PCI_H

#include <sys/io.h>

#define PCI_CLASS_DEVICE        0x0a    /* Device class */
#define PCI_VENDOR_ID           0x00    /* 16 bits */
#define PCI_DEVICE_ID           0x02    /* 16 bits */
#define PCI_COMMAND             0x04    /* 16 bits */
#define PCI_REV_AND_CLASS       0x08    /* 32 bits */

#define PCI_CLASS_BRIDGE_HOST           0x0600
#define PCI_CLASS_DISPLAY_VGA           0x0300

#define PCI_VENDOR_ID_INTEL             0x8086
#define PCI_VENDOR_ID_COMPAQ            0x0e11
#define PCI_CLASS_BRIDGE_PCI            0x0604
#define ETHRNET_CNTRL_DEV_ID		0x108B

/* For DLink ethernet controller card */
#define ETH_PCI_VIA_VENDOR_ID		0x1106
#define ETH_PCI_VIA_DEV_ID              0x3106
#define ETH_PCI_INTEL_VENDOR_ID		0x8086
#define ETH_PCI_INTEL_DEV_ID            0x108B

#define PCI_PRIMARY_BUS         0x18    /* Primary bus number */
#define PCI_SECONDARY_BUS       0x19    /* Secondary bus number */
#define PCI_SUBORDINATE_BUS     0x1a    /* Highest bus number behind the bridge */

#define PCI_IO_BASE             0x1c    /* I/O range behind the bridge */
#define PCI_IO_LIMIT            0x1d
#define PCI_MEMORY_BASE         0x20    /* Memory range behind */
#define PCI_MEMORY_LIMIT        0x22
#define PCI_PREF_MEMORY_BASE    0x24    /* Prefetchable memory range behind */
#define PCI_PREF_MEMORY_LIMIT   0x26

#define PCI_BASE_ADDRESS_0      0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1      0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2      0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3      0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4      0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5      0x24    /* 32 bits */
#define PCI_INTR_REG 		0x3C
#define  PCI_BASE_ADDRESS_SPACE         0x01    /* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO      0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY  0x00
#define  PCI_BASE_ADDRESS_MEM_PREFETCH  0x08    /* prefetchable? */

#define SPARK_PCI_ETH_DEVICE

struct pci_bridge {
int devfn;
int primary;
int secondary;
int subordinate;
unsigned long iobase;
unsigned long iolimit;
unsigned long membase;
unsigned long memlimit;
unsigned long pmembase;
unsigned long pmemlimit;
};
#define PCI_BRIDGES_MAX 10 
//currently facility for storing info about 5 devices
#define MAX_PCI_DEVICE_FUNCTIONS 16
//indexed by secondary bus number
struct pci_bridge bridge_list[PCI_BRIDGES_MAX];

#define ADVANTECH_VENDOR_ID 0x13fe
#define INTEL_VENDOR_ID 0x8086
#define ADVANTECH_DEV_ID 0x1710

//currently using irq info obtained from bios for our motherboard
#define ADVANTECH_INTR_LINE 0x0b
#define ADVANTECH_INTR_PIN 0x01 //intr A
//kernel data structure
struct pci_dev_info
{
    int bus_no;
    int devfn;
    int vendor_id;
    int devid;
    //No.of memory regions
    int num_mem;
    //No.of prefetchable memory regions
    int num_pmem;
    //No.of io regions
    int num_io;
    //Upto 6 mem or io regions possible
    unsigned long mem_addr[6];
    unsigned long pmem_addr[6];
    //Size of memory address space
    unsigned long mem_size[6];
    unsigned long pmem_size[6];
    unsigned int io_addr[6];
    //Size of io address space
    unsigned int io_size[6];
    unsigned char intr_line;
    unsigned char intr_pin;
};
//Data structure to be used by driver
struct pci_device_info
{
    int device_status;
    //No.of memory regions
    int num_mem;
    //No.of prefetchable memory regions
    int num_pmem;
    //No.of io regions
    int num_io;
    //Upto 6 mem or io regions possible
    unsigned long mem_addr[6];
    unsigned long pmem_addr[6];
    unsigned long pmem_size[6];
    //Size of memory address space
    unsigned long mem_size[6];
    unsigned int io_addr[6];
    //Size of io address space
    unsigned int io_size[6];
    unsigned char intr_line;
    unsigned char intr_pin;
    unsigned int bus;
    unsigned int devfn;
};
int pci_direct_init(void);
int pci_scan(void);
int print_pci_info(void);
int get_device_info(int vid,int devid,int num,struct pci_device_info *devinfo);
int find_num_devices(int vid,int devid);

#endif
