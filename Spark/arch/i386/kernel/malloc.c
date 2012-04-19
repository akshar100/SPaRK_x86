/*
 * Stand-Alone RTLinux Memory Allocator
 * 
 * Written by Vicente Esteve LLoret
 * Released under the terms of the GPL Version 2
 *
 */

#include <arch/e820.h>
#include <arch/mprot.h>
#include <arch/page.h>
#include <rtl_malloc.h>
#include <rtl_conf.h>
#include <ctype.h>
#include <arch/memory.h>
#include <rtl_printf.h>

extern char _init_allocspace;
static char *endmem=(char *) &_init_allocspace;

char	*freePages_CurrAddr, *freePages_MaxLimit;


//***************** LIBC Compatibility **************************

void *malloc(size_t size) {
  return rtl_malloc(size);
};

void *realloc(void *ptr,size_t new_len) {
  return rtl_realloc(ptr,new_len);
};

void *calloc(size_t nelem,size_t elem_size) {
  return rtl_calloc(nelem,elem_size);
};

void free(void *ptr) {
  rtl_free(ptr);	
};

//*****************************************************************
//


/*******************Our simple 1 page malloc****************************/
void	init_page_pool(char *start_addr, unsigned long size)
{
	freePages_CurrAddr = start_addr;
	freePages_MaxLimit = freePages_CurrAddr + size;
}

char*	kGetPage(void)
{
	char	*curr_page;
	if ( (freePages_CurrAddr + RTL_PAGE_SIZE) > freePages_MaxLimit )
	{
		rtl_printf("error:: Heap is emptied, please check");
		return NULL;
	}
	
	curr_page = freePages_CurrAddr;
	freePages_CurrAddr += RTL_PAGE_SIZE;


	return	curr_page;
}
/***********************************************************************/

unsigned long GetMemorySize(void)
{
  return ((*((unsigned long *) 0xf00) + 1024) * 1024 ); 
};


void init_memory(void)
{
	if(endmem < 0x100000 )
		endmem = 0x100000;

	if( (unsigned long)endmem % 0x1000 != 0) {
 		rtl_printf("error:: Value of endmem is not multiple of 0x1000");
 	}

	init_page_pool(endmem,KERNEL_HEAP_SIZE);

}
