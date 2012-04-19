/*
 * SParK scheduler
 * Developed at IIT Bombay
 *
 */

#ifndef __RTL__SCHED__
#define __RTL__SCHED__

#include <rtl_conf.h>
//#include <signal.h>

extern	int	iCurrGuestOsIndex; 	// MM070914 Def in processor.h
extern	int	spark_schedule(void);
extern  int     init_sched(void); 

extern unsigned int gCurrEtaCount;



#endif
