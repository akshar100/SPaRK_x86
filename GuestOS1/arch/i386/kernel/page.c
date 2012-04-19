/*
 * page.c
 * This code is developed in IITB
 */

#include <arch/page.h>
#include <arch/mprot.h>
#include <rtl_conf.h>
#include <arch/memory.h>
#include <arch/context.h>

extern pthread_t tcb[2];

extern char _init_kernel, _end_kernel, _init_tskcontext, _end_tskcontext, _init_allocspace; 

page_entry pde[1024] __attribute__ ((__section__ (".pagetable")));

/*******************************************************************/
void map_kernelspace(void)
{
	void *ptr = (void *) &_init_kernel, *ulEndMemory;
	page_entry *pte;

	ulEndMemory = (void *) ((unsigned long) &_end_kernel);
	// ulEndMemory = MEMORY_MAX;
	// map the kernel text + data + bss
	while (ptr < ulEndMemory)
	{
		pte = pde[address2pdeindex(ptr)] & 0xFFFFF000;
		if (0 == pte)
		{
			pte = (page_entry *) kGetPage();
			memset(pte, 0, (NUM_ENTRIES_PER_PAGE*sizeof(int)));
			pde[address2pdeindex(ptr)] = ((unsigned long) pte & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
		}

		pte[address2pteindex(ptr)] = ((unsigned long) ptr & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
		ptr = (void *) (((unsigned long) ptr) + RTL_PAGE_SIZE);
	}

	// map the heap area	
	ptr = (void *)&_init_allocspace; 
	ulEndMemory = MEMORY_MAX;

	while (ptr < ulEndMemory)
	{
		pte = pde[address2pdeindex(ptr)] & 0xFFFFF000;
		if (0 == pte)
		{
			pte = (page_entry *) kGetPage();
			memset(pte, 0, (NUM_ENTRIES_PER_PAGE*sizeof(int)));
			pde[address2pdeindex(ptr)] = ((unsigned long) pte & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
		}

		pte[address2pteindex(ptr)] = ((unsigned long) ptr & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
		ptr = (void *) (((unsigned long) ptr) + RTL_PAGE_SIZE);
	}

	return 0;
}

/*******************************************************************
  maps and registers all user tasks with the spark kernel
 ********************************************************************/
//struct task_data td;

void	map_userspace(void)
{
	void *ptr = 0 , *ulEndMemory = 0;
	int iCount ;
	page_entry *pte;

	for (iCount = 0 ; iCount < NUM_CONTEXTS ; iCount++)
	{
		// map the user space
		ptr = (void *)context[iCount].base;
		ulEndMemory = (void *)context[iCount].end;
		while (ptr < ulEndMemory)
		{
			pte = pde[address2pdeindex(ptr)] & 0xFFFFF000;
			if (0 == pte)
			{
				pte = (page_entry *) kGetPage();
				memset(pte, 0, (NUM_ENTRIES_PER_PAGE*sizeof(int)));
				pde[address2pdeindex(ptr)] = ((unsigned long) pte & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
			}
			pte[address2pteindex(ptr)] = ((unsigned long) ptr & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
			ptr = (void *) (((unsigned long) ptr) + RTL_PAGE_SIZE);
		}

		//tcb[iCount]->cr3 = pde;
		// register the tasks page tables
		//td.cr3 = tcb[iCount]->cr3;
		//td.esp2 = tcb[iCount]->top_of_stack; 
		spark_registerPages(iCount,pde);
		//spark_registerPages(iCount,(unsigned long)&td);

		// unmap the tasks page tables
		ptr = (void *)context[iCount].base;
		ulEndMemory = (void *)context[iCount].end;

		while (ptr < ulEndMemory)
		{
			pte = pde[address2pdeindex(ptr)] & 0xFFFFF000;
			pte[address2pteindex(ptr)] = 0;
			ptr = (void *) (((unsigned long) ptr) + RTL_PAGE_SIZE);
		}
	}

	return 0;
}
/*******************************************************************/
void init_page(void)
{
	map_kernelspace();
	map_userspace();
}
/*******************************************************************/
