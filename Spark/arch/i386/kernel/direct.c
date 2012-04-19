/*
 * direct.c - Low-level direct PCI config space access
 */

#include <rtl_pci.h>
#include <rtl_printf.h>
#include <sys/io.h>

unsigned int last_discovered_bus = 0;
struct pci_dev_info pci_devices[MAX_PCI_DEVICE_FUNCTIONS];
int num_devices_discovered=0;
int num_bridges_discovered=0;
int num_buses_discovered=0;
/*
 * Functions for accessing PCI configuration space with type 1 accesses
 */

#define PCI_CONF1_ADDRESS(bus, devfn, reg) \
	(0x80000000 | (bus << 16) | (devfn << 8) | (reg & ~3))

/*Reads data from configuration space*/
int pci_conf1_read(unsigned int seg, unsigned int bus,
			  unsigned int devfn, int reg, int len, unsigned long *value)
{
	unsigned long flags;

	if (!value || (bus > 255) || (devfn > 255) || (reg > 255))
	{
		rtl_printf("PCI:--*inv*--\n");
		return -1;
	}

	outl(PCI_CONF1_ADDRESS(bus, devfn, reg), 0xCF8);

	switch (len) {
	case 1:
		*value = inb(0xCFC + (reg & 3));
		break;
	case 2:
		*value = inw(0xCFC + (reg & 2));
		break;
	case 4:
		*value = inl(0xCFC);
		break;
	}
	return 0;
}
/*Writes data to configuration space*/

int pci_conf1_write(unsigned int seg, unsigned int bus,
			   unsigned int devfn, int reg, int len, unsigned long value)
{
	unsigned long flags;

	if ((bus > 255) || (devfn > 255) || (reg > 255)) 
		return -1;

	outl(PCI_CONF1_ADDRESS(bus, devfn, reg), 0xCF8);
	switch (len) {
	case 1:
		outb((unsigned char)value, 0xCFC + (reg & 3));
		break;
	case 2:
		outw((unsigned short)value, 0xCFC + (reg & 2));
		break;
	case 4:
		outl((unsigned int)value, 0xCFC);
		break;
	}
	return 0;
}

/*Checks if configuration space of devices on PCI bus is accessible*/
static int pci_sanity_check(void)
{
	unsigned long x = 0;
	int devfn;
	for (devfn = 0; devfn < 0x100; devfn++) {
		if (pci_conf1_read(0, 0, devfn, PCI_CLASS_DEVICE, 2, &x))
				continue;
		if (x == PCI_CLASS_BRIDGE_HOST || x == PCI_CLASS_DISPLAY_VGA)
		{
			return 1;
		}

		if (pci_conf1_read(0, 0, devfn, PCI_VENDOR_ID, 2, &x))
			continue;
		if (x == PCI_VENDOR_ID_INTEL || x == PCI_VENDOR_ID_COMPAQ)
		{
			return 1;
		}
	}
	return 0;
}

/* Checks if PCI devices can be addressed by type 1 access*/
int pci_check_type1(void)
{
	unsigned long flags;
	unsigned int tmp;
	int works = 0;

	outb(0x01, 0xCFB);
	tmp = inl(0xCF8);
	outl(0x80000000, 0xCF8);
	if (inl(0xCF8) == 0x80000000 && pci_sanity_check()) {
		works = 1;
	}
	outl(tmp, 0xCF8);

	return works;
}
/* Checks if PCI devices can be addressed by type 1 access and 
*does sanity check
*/
int pci_direct_init(void)
{
	if (pci_check_type1()) {
		return 0;
	}
	else
	{
		rtl_printf("PCI ERROR : Can't initialize PCI Subsystem\n");
		return 1;
	}
}

/*
*Find architecture of PCI subsystem for current hardware
*Fill all the information found in  bridge_list data structure
*/
int pci_scan(void)
{
	int idx=0,devfn,num_bus=0,bus; 
	unsigned long x;
	unsigned long temp;
	for (num_bus = 0; num_bus < 0x100; num_bus++)
	{ 
		for (devfn = 0; devfn < 0x100; devfn++) 
		{
			if (pci_conf1_read(0, num_bus, devfn, PCI_CLASS_DEVICE, 2, &x))
				continue;
			if(x == PCI_CLASS_BRIDGE_PCI)
			{
				bridge_list[idx].devfn=devfn;
				//Read primary, secondary and subordinate busnumbers
				pci_conf1_read(0, 0, devfn,PCI_PRIMARY_BUS, 4,&temp);
				bridge_list[idx].primary=temp & 0xff;
				temp = temp >> 8;
				bridge_list[idx].secondary=temp & 0xff;
				temp = temp >> 8;
				bridge_list[idx].subordinate=temp & 0xff;
				pci_conf1_read(0, 0, devfn,PCI_IO_BASE, 4,&temp);
				bridge_list[idx].iobase=temp & 0xff;
				temp = temp >> 8;
				bridge_list[idx].iolimit=temp & 0xff;
				pci_conf1_read(0, 0, devfn,PCI_MEMORY_BASE, 4,&temp);
				//Upper 12 bits of mem base
				bridge_list[idx].membase=((temp & 0xffff) << 16) | 0x0;
				//Upper 12 bits of mem limit
				temp = temp >> 16;
				bridge_list[idx].memlimit=((temp & 0xffff) << 16) | 0xfffff;
				pci_conf1_read(0, 0, devfn,PCI_PREF_MEMORY_BASE, 4,&temp);
				//Upper 12 bits of prefetchable mem base
				bridge_list[idx].pmembase=((temp & 0xffff) << 16) | 0x0;
				//Upper 12 bits of prefetchable mem limit
				temp = temp >> 16;
				bridge_list[idx].pmemlimit=((temp & 0xffff) << 16) | 0xfffff;
				idx++;

				if(idx == PCI_BRIDGES_MAX)
				{
					rtl_printf("\n*Scan stopped as max bridges reached..May contain more bridges");
				}
				
			}
		}
	}
	num_bridges_discovered=idx;
	num_buses_discovered=idx+1;

	//Scan all the buses for finding pci devices
	get_pci_devicelist();
	//print_pci_info();	//shenai
	return 0;
}


/* This function is currently scanning all devices but adding them to kernel data structure only if they are advantech devices. To make this routine generic replace the check
if (z == ADVANTECH_VENDOR_ID && y == ADVANTECH_DEV_ID) by
if (z != 0xffff && y != 0xffff). However in that case number of devices will be unpedictable so we will need to use malloc to get memory space for the data structure corresponding to each device*/

int get_pci_devicelist(void)
{
	int bus,devfn;
	unsigned long vid,devid;
	unsigned int pos,reg,idx;
	unsigned int temp;
	unsigned long y=0,size=0,command_reg;
	char flag = 0;
	idx = num_devices_discovered;
	for (bus = 0; bus < num_buses_discovered; bus++) 
	{
		for (devfn = 0; devfn < 0x100; devfn++) 
		{
			pci_conf1_read(0,bus,devfn,PCI_VENDOR_ID, 4, &vid);
			devid=(vid >> 16) & 0x0ffff;
			vid=vid & 0xffff;
		
			if ((vid == ADVANTECH_VENDOR_ID && devid == ADVANTECH_DEV_ID) || (vid == ETH_PCI_VIA_VENDOR_ID && devid == ETH_PCI_VIA_DEV_ID)) //shenai
			{
				//rtl_printf("\nidx %d, vid 0x%x, devid 0x%x",idx,vid,devid);
				pci_devices[idx].vendor_id=vid;
				pci_devices[idx].devid=devid;
				pci_devices[idx].bus_no=bus;
				pci_devices[idx].devfn=devfn;
				//Find how many memory & io regions device has
				//and what are the sizes of regions
				pci_devices[idx].num_mem=0;
				pci_devices[idx].num_pmem=0;
				pci_devices[idx].num_io=0;
				pci_conf1_read(0,bus,devfn,PCI_COMMAND, 4, &command_reg);
				y = command_reg & 0xfffffffc;
				//disable command register
				pci_conf1_write(0,bus,devfn,PCI_COMMAND,4,y);
				//check all the 6 base address registers
				for(pos=0; pos<6; pos++ )
				{
					reg = PCI_BASE_ADDRESS_0 + (pos << 2);
					//don't forget to assign value to Advantech variables in previous function
					pci_conf1_read(0,bus,devfn,reg,4, &y);
					pci_conf1_write(0,bus,devfn,reg,4, ~0);
					pci_conf1_read(0,bus,devfn,reg,4,&size);
					pci_conf1_write(0,bus,devfn,reg,4,y);
					if (!size || size == 0xffffffff)
                        			continue;
					if((y & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY)
					{
						size = size & 0xfffffff0;
						size = (size & ~(size-1)) - 1;
					if((y & PCI_BASE_ADDRESS_MEM_PREFETCH ) == PCI_BASE_ADDRESS_MEM_PREFETCH )
					{
						pci_devices[idx].pmem_addr[pci_devices[idx].num_mem] = y;
						pci_devices[idx].pmem_size[pci_devices[idx].num_mem] = size;
						pci_devices[idx].num_pmem++;

					}
					else
					{
						pci_devices[idx].mem_addr[pci_devices[idx].num_mem] = y;
						pci_devices[idx].mem_size[pci_devices[idx].num_mem] = size;
						pci_devices[idx].num_mem++;

					}

											}
					else
					{
						size = size & 0xfffffff0;
						size = (size & ~(size-1)) - 1;
						pci_devices[idx].io_addr[pci_devices[idx].num_io] = y;
						pci_devices[idx].io_size[pci_devices[idx].num_io] = size;
						pci_devices[idx].num_io++;
					}
				}
				//Restore command reg
				pci_conf1_write(0,bus,devfn,PCI_COMMAND, 4,command_reg);
				pci_conf1_read(0,bus,devfn,PCI_INTR_REG,4,&y);
				pci_devices[idx].intr_line = y & 0xff;
				y = y >> 8;
				pci_devices[idx].intr_pin = y & 0xff;
				idx++;
				if(idx == MAX_PCI_DEVICE_FUNCTIONS)
				{
					rtl_printf("\n*Scan stopped as max devices reached..May contain more devices");
				}

			}
		}
		num_devices_discovered = idx;
	}
	return 0;
}
/*
*Debug function to print values obtained in kernel data structures for bridges and devices.
*Make sure this is called after pci_scan().
*/

int print_pci_info()
{
	int idx,i,j;
	rtl_printf("\nFound %d bridges and %d pci buses",num_bridges_discovered,num_buses_discovered);
	unsigned int * temp_address;
#if 0
// Do not need all this right now
	for(idx=0;idx < num_bridges_discovered;idx++)
	{
		rtl_printf("\nBridge: %d ",idx+1);
		rtl_printf("Prim 0x%x ",bridge_list[idx].primary);
		rtl_printf("Sec 0x%x ",bridge_list[idx].secondary);
		rtl_printf("Sub 0x%x ",bridge_list[idx].subordinate);
		rtl_printf("iobase 0x%x ",bridge_list[idx].iobase);
		rtl_printf("iolim 0x%x ",bridge_list[idx].iolimit);
		rtl_printf("mbase 0x%x ",bridge_list[idx].membase);
		rtl_printf("mlimit 0x%x ",bridge_list[idx].memlimit);
		rtl_printf("Pref mbase 0x%x ",bridge_list[idx].pmembase);
		rtl_printf("Pref mlimit 0x%x ",bridge_list[idx].pmemlimit);
	}
#endif 

//	rtl_printf("\n\nFound %d Advantech Devices",num_devices_discovered);
//	for(idx=0;idx < num_devices_discovered;idx++)
	idx = 0;
	if (idx == 0) //For proper display during demo
	{
		
		rtl_printf("\nDevice %d: \n",idx);
		rtl_printf("Addr (0x%x:0x%x) \n",pci_devices[idx].bus_no,pci_devices[idx].devfn);
		for(i=0; i<pci_devices[idx].num_mem; i++)
		{
			rtl_printf("Mregion %d at 0x%x of size 0x%x \n",i,pci_devices[idx].mem_addr[i],pci_devices[idx].mem_size[i]);

		}
		for(i=0; i<pci_devices[idx].num_pmem; i++)
		{
			rtl_printf("Prefetchable Mregion %d at 0x%x of size 0x%x \n",i,pci_devices[idx].pmem_addr[i],pci_devices[idx].pmem_size[i]);
		}

		for(i=0; i<pci_devices[idx].num_io; i++)
		{
			rtl_printf("IOregion %d at 0x%x of size 0x%x \n",i,pci_devices[idx].io_addr[i],pci_devices[idx].io_size[i]);

		}
		rtl_printf("Int line 0x%x, Int pin 0x%x \n",pci_devices[idx].intr_line,pci_devices[idx].intr_pin);
	}
}

/* 
I/P: bus number, byte formed by 5 bit device address and 3 bit function address
*Debug function to read 64 bytes of PCI Configuration space
*Should be called after the bridges have been configured
*/
int read_pci_configSpace(int bus,int devfn)
{
	int i;
	unsigned long y;
	rtl_printf("\n reading bus 0x%x devfn 0x%x",bus,devfn);
	for(i=0; i<16; i++ )
	{
		pci_conf1_read(0, bus, devfn,i*4, 4, &y);
		rtl_printf("\naddr: 0x%x val: 0x%x",i*4,y);
	}
	return 0;
}
/*
*Interface function for driver.
*I/P: vendor id, device id of the devices to be found
*Return Value: number of devices having matching devid and vendor id*/ 
int find_num_devices(int vid,int devid)
{
	int i,num = 0;
	for(i=0;i < num_devices_discovered;i++)
	{
		if(pci_devices[i].vendor_id == vid && pci_devices[i].devid == devid)
		{
			num++;
		}
		
	}
	return num;
}

/*
*Interface function for driver.
*I/P: vendor id, device id,number of devices,pointer to array of 
 type struct pci_device_info (size of array should match number of devices)
*Return Value : Actual number of devices filled in array.
*This function would fill the array with information obtained during subsystem scan.If number asked is less than actual devices then info about only first discovered devices is returned. 
*/ 
int get_device_info(int vid,int devid,int num,struct pci_device_info *devinfo)
{
	int i,n_d = 0,j;
	if (num <= 0)
		return 0;
	for(i=0;i < num_devices_discovered;i++)
	{
		if(pci_devices[i].vendor_id == vid && pci_devices[i].devid == devid)
		{
	                
			devinfo[n_d].bus   = pci_devices[i].bus_no;
			devinfo[n_d].devfn = pci_devices[i].devfn;
			devinfo[n_d].num_mem = pci_devices[i].num_mem;
			devinfo[n_d].num_pmem = pci_devices[i].num_pmem;
			devinfo[n_d].num_io = pci_devices[i].num_io; 
			for(j=0; j<pci_devices[i].num_mem; j++)
			{
				devinfo[n_d].mem_addr[j] = pci_devices[i].mem_addr[j]; 
				devinfo[n_d].mem_size[j] = pci_devices[i].mem_size[j]; 
			}
			for(j=0; j<pci_devices[i].num_pmem; j++)
			{
				devinfo[n_d].pmem_addr[j] = pci_devices[i].pmem_addr[j]; 
				devinfo[n_d].pmem_size[j] = pci_devices[i].pmem_size[j]; 
			}

			for(j=0; j<pci_devices[i].num_io; j++)
			{
				devinfo[n_d].io_addr[j] = pci_devices[i].io_addr[j]; 
				devinfo[n_d].io_size[j] = pci_devices[i].io_size[j]; 
			}
			devinfo[n_d].intr_line = pci_devices[i].intr_line; 
			devinfo[n_d].intr_pin = pci_devices[i].intr_pin; 
			n_d++;
			//There may be more devices but driver asked only for num
			if(n_d >= num)
				return num;
		}
		
	}
	return n_d;
}
