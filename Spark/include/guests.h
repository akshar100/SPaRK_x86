#ifndef _SPARK_GUESTS_H
#define _SPARK_GUESTS_H

#define	NUM_OF_GUESTOS			3		// total number of guest os
#define	MEMORY_ALLOTED_TO_EACH_GUEST	0x2000000	// 32MB
#define	MEMORY_ALLOTED_TO_SPARK		0x2000000	// 32MB
#define	MEMORY_PER_PDEENTRY		0x400000	// 4MB
#define	MAX_TASK_PER_GUEST		2		// max tasks per guest os

/*
typedef struct GUESTTIMERS
{
	int	iCurrTicks;
	int	iTicks;
	unsigned int	iHandler;
}GUESTTIMERS;
extern	GUESTTIMERS	guestTimers[NUM_OF_GUESTOS+1];
*/
extern	int	iCurrGuestOsIndex;

#endif
