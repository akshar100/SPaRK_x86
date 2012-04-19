/*
 * page.c
 * Developed at IITB
 * Reference from :- RTLinux  
 *
 */

#include <arch/page.h>
#include <arch/mprot.h>
#include <rtl_conf.h>
#include <arch/memory.h>
#include <rtl_printf.h>
#include <rtl_pci.h>
#include <arch/processor.h>

extern char _init_kernel, _end_kernel, _init_tskcontext, _end_tskcontext, _init_allocspace; 

page_entry pde[1024] __attribute__ ((__section__ (".pagetable")));

#ifdef SPARK_PCI_ETH_DEVICE 
extern struct pci_device_info ethdevinfo[1];
#endif
/****************************************************************
  map_kernelspace
 ****************************************************************/
void map_kernelspace(void) {
	void *ptr = (void *) &_init_kernel, *ulEndMemory;
	page_entry *pte;
	ptr = 0;

	if( (unsigned long) &_end_kernel < 0x100000 )
		ulEndMemory = (void *) ((unsigned long) 0x100000 + KERNEL_HEAP_SIZE);
	else
		ulEndMemory = (void *) ((unsigned long) &_end_kernel + KERNEL_HEAP_SIZE);
	while (ptr < ulEndMemory)
	{
		pte = pde[address2pdeindex(ptr)] & 0xFFFFF000;
		if (0 == pte) 
		{
//DEEPTI                      
			pte = (page_entry *) kGetPage();
			memset(pte, 0, (NUM_ENTRIES_PER_PAGE*sizeof(int)));
			pde[address2pdeindex(ptr)] = ((unsigned long) pte & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
		}
		pte[address2pteindex(ptr)] = ((unsigned long) ptr & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
		ptr = (void *) (((unsigned long) ptr) + RTL_PAGE_SIZE);
	}

#ifdef SPARK_PCI_ETH_DEVICE
        ptr = (void *)((ethdevinfo[0].mem_addr[0]) & 0xFFFFF000);
	pte = pde[address2pdeindex(ptr)] & 0xFFFFF000;
	if (0 == pte) 
	{
		pte = ptr;
		pde[address2pdeindex(ptr)] = ((unsigned long) pte & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
	}
	pte[address2pteindex(ptr)] = ((unsigned long) ptr & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
#endif
	return;
}
/****************************************************************
  map_guestOSes
 ****************************************************************/
void map_guestOSes(void) {
	void *ptr, *ulEndMemory;
	page_entry *guest_pde, *pte;
	int i,j;

	for(i=0; i< NUM_OF_GUESTOS; i++ ) {
		guest_pde = kGetPage();
		memset(guest_pde, 0, (NUM_ENTRIES_PER_PAGE*sizeof(int)));
		ptr = MEMORY_ALLOTED_TO_SPARK + i*MEMORY_ALLOTED_TO_EACH_GUEST;
		ulEndMemory = (unsigned long)ptr +	MEMORY_ALLOTED_TO_EACH_GUEST;

		while (ptr < ulEndMemory)
		{
			pte = guest_pde[address2pdeindex(ptr)] & 0xFFFFF000;
			if (0 == pte) 
			{
				pte = (page_entry *) kGetPage();
				memset(pte, 0, (NUM_ENTRIES_PER_PAGE*sizeof(int)));
				guest_pde[address2pdeindex(ptr)] = ((unsigned long) pte & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
			}
			pte[address2pteindex(ptr)] = ((unsigned long) ptr & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
			ptr = (void *) (((unsigned long) ptr) + RTL_PAGE_SIZE);
		}

		// map spark kernel
		for (j = 0 ; j < MEMORY_ALLOTED_TO_SPARK/MEMORY_PER_PDEENTRY ; j++)	
		{
			guest_pde[j] = pde[j];
		}

		guestOS_thread[i].pde[0] = guest_pde;
	}

	return;
}

void enable_pagebit(void) 
{
	SET_CR3(pde);
	ENABLE_PAGE();
}

unsigned long GET_CR3()
{
    unsigned long value_cr3;
    __asm__ __volatile__ ("movl %%cr3,%%eax \n":"=a" (value_cr3):);
    return value_cr3;
}



void init_page(void)
{
	map_kernelspace();
	map_guestOSes();
	enable_pagebit();
}
/*
 *
 */
int	verifyAndCopyPages (int iProcessId , void * baseAddr)
{
	int iCount, iCurrEntry = 0;
	page_entry *guest_pde = (page_entry *)baseAddr;
	page_entry *spark_pde, *guest_pte , *spark_pte;
	unsigned long uMappedMemory = 0;
	int iPdeBase = guestOS_thread[iCurrGuestOsIndex].phyOffset/MEMORY_PER_PDEENTRY;
	spark_pde = (page_entry *) kGetPage();
	memset(spark_pde, 0, (NUM_ENTRIES_PER_PAGE*sizeof(int)));

	// verify guest os pages..and copy into spark area if valid
	for (iCount = 0 ; iCount < NUM_ENTRIES_PER_PAGE ; iCount++)
	{
		if ( guest_pde[iCount] != 0 )
		{
			spark_pte = (page_entry *) kGetPage();
			memset(spark_pte, 0, (NUM_ENTRIES_PER_PAGE*sizeof(int)));
			spark_pde[iCount + iPdeBase] = ((unsigned long) spark_pte & 0xFFFFF000) | (guest_pde[iCount] & 0x00000FFF ) ;// PAGE_PRESENT | PAGE_RW ;
			guest_pte = (page_entry *)(guest_pde[iCount] & 0xFFFFF000);
			guest_pte = (page_entry *) ((unsigned long)guest_pte + guestOS_thread[iCurrGuestOsIndex].phyOffset);
			iCurrEntry = 0;

			while ( iCurrEntry < NUM_ENTRIES_PER_PAGE ) {
				if (guest_pte[iCurrEntry] != 0) {
					if( (guest_pte[iCurrEntry] & 0xFFFFF000) <= MEMORY_ALLOTED_TO_EACH_GUEST - RTL_PAGE_SIZE) {
						uMappedMemory = (guest_pte[iCurrEntry] & 0xFFFFF000) + guestOS_thread[iCurrGuestOsIndex].phyOffset;
						spark_pte[iCurrEntry] = ((unsigned long) uMappedMemory & 0xFFFFF000) | (guest_pte[iCurrEntry] & 0x00000FFF);
					}
					else {
						rtl_printf("!!!!got a bad pte entry %d\t of 0x%x!!!!!!!\n" , iCurrEntry, guest_pte[iCurrEntry]);
						return 1;
					}
				}
				iCurrEntry++;
			}
		}
	}
	// map spark kernel
	for (iCount = 0 ; iCount < MEMORY_ALLOTED_TO_SPARK/MEMORY_PER_PDEENTRY ; iCount++)	{
		spark_pde[iCount] = pde[iCount];
	}

	guestOS_thread[iCurrGuestOsIndex].pde[iProcessId] = spark_pde;
	return 0;
}
/*
 *
 */
int	load_pde(int	iProcessId)
{
	SET_CR3(guestOS_thread[iCurrGuestOsIndex].pde[iProcessId]);
	return 0;
}
