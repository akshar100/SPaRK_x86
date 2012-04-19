/*
 * malloc.c
 * This code is developed in IITB
 */

#include <arch/page.h>
#include <rtl_malloc.h>
#include <rtl_conf.h>
#include <ctype.h>
#include <arch/memory.h>

/*******************************************************************/
extern char _init_allocspace;
static char *endmem=(char *) &_init_allocspace;
/*******************Our simple 1 page malloc************************/
char	*freePages_CurrAddr, *freePages_MaxLimit;

void	init_page_pool(char *start_addr, unsigned long size)
{
	freePages_CurrAddr = start_addr;
	freePages_MaxLimit = freePages_CurrAddr + size;
	if (MEMORY_MAX < freePages_MaxLimit )
		freePages_MaxLimit = MEMORY_MAX;
}

char*	kGetPage(void)
{
	char	*curr_page;
	if ( (freePages_CurrAddr + RTL_PAGE_SIZE) > freePages_MaxLimit )
		return NULL;

	curr_page = freePages_CurrAddr;
	freePages_CurrAddr += RTL_PAGE_SIZE;

	return	curr_page;
}
/***********************************************************************/
void init_memory(void)
{
	if( endmem < 0x100000 )
		endmem = 0x100000;
	init_page_pool( endmem, MEMORY_MAX );
}
/***********************************************************************/
