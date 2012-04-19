/*
 This code is developed in IITB
*/

#include <rtl_tasks.h>
#include <rtl_sched.h>
#include <rtl_time.h>
#include <rtl_conf.h>
#include <arch/context.h>
#include <arch/page.h>
#include <arch/linkage.h>

asmlinkage int system_call(void);

extern void start_kernel(void)
{
	// char	*ptr = 0x2000001;
	init_memory();
	init_page();
	init_sched();
	init_tasks();
	init_itc();
	init_clocks();
	spark_registerSysCallHandler(&system_call, 0x90);
	//spark_print(" ========== Guest 1 :: STARTED\n");


	while(1) {
		int	i,j,k, dummy =0;

		for (k=0; k< 5; k++)
		{
			for( i = 0 ; i < 10000; i++){
				for(j = 0 ; j < 1000; j++){
					
	//				dummy += 5;
	//				if(dummy > 500)
	//					dummy = 0;
				}
			}
		}
	}
}
