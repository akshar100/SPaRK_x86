/*************************************
*
*   Developed and Written at IIT Bombay
*
**************************************/

#include <guests.h>

typedef unsigned int uint;

#define MaxGuestSize 0x20000

uint *offset1 = (uint *)0x30000;
uint *offset2 = (uint *)0x50000;
uint *offset3 = (uint *)0x70000;

void copyGuestOS()
{
	uint *guest1Loc = (uint *)MEMORY_ALLOTED_TO_SPARK;
	uint *guest2Loc, *guest3Loc; 
	uint i;
	
	guest2Loc = (char *)guest1Loc + MEMORY_ALLOTED_TO_EACH_GUEST;
	guest3Loc = (char *)guest2Loc + MEMORY_ALLOTED_TO_EACH_GUEST;

	for (i =0 ; i< MaxGuestSize; i++)
	{
		*guest1Loc++ = *offset1++;
		*guest2Loc++ = *offset2++;
		*guest3Loc++ = *offset3++;
	}
}
