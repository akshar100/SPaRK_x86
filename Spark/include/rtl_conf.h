
#define RTLINUX_V3 1
#define _RTL_POSIX_THREADS 1
#define _RTL_POSIX_THREAD_SAFE_FUNCTIONS 1
#define _RTL_POSIX_THREAD_PRIORITY_SCHEDULING 1
#define _RTL_POSIX_THREAD_PROCESS_SHARED 1
#define _RTL_POSIX_THREAD_ATTR_STACKADDR 1
#define _RTL_POSIX_THREAD_ATTR_STACKSIZE 1
#define _RTL_POSIX_SEMAPHORES 1
#define _RTL_POSIX_MONOTONIC_CLOCK 1
#define _RTL_POSIX_SPIN_LOCKS 1
#define _RTL_POSIX_TIMEOUTS 1
#define RTL_PTHREAD_KEYS_MAX (20)
#define RTL_PTHREAD_THREADS_MAX (128)
#define RTL_PTHREAD_MUTEX_MAX (128)
#define RTL_PTHREAD_COND_MAX (128)
#define RTL_SEM_NSEMS_MAX (128)
#define _RTL_POSIX_THREAD_THREADS_MAX (128)


#define _USE_SEGMENTATION_FOR_PROT_  1  


/*
 * Processor type and features
 */
#define CONFIG_I386 1
#define CONFIG_RTL_FP_SUPPORT 1

/*
 * Scheduler
 */
#define CONFIG_RTL_SCHED_RM 1
#undef  CONFIG_RTL_SCHED_EDF

/*
 * Posix options
 */
#define _RTL_POSIX_IO 1
#define _RTL_POSIX_SIGNALS 1
#undef CONFIG_OC_PBARRIERS 
#undef  CONFIG_RTL_SRP

/*
 * Debug options
 */
/*
#define CONFIG_RTL_GDBAGENT 1
#define CONFIG_RTL_DEBUG 1
#define  CONFIG_RTL_BAKERTEST 1 
#define  CONFIG_RTL_TRACER 1
#define  _RTL_STARTBREAKPOINT 1
#define CONFIG_RTL_TRACER_EVENTS (200)
*/

//#undef  CONFIG_RTL_BAKERTEST 
//#undef  CONFIG_RTL_TRACER
//#undef  _RTL_STARTBREAKPOINT



/*
 * Drivers
 */
#define DEVICE_I386_TERMINAL 1
#define DEVICE_I386_KBD 1
#define DEVICE_I386_PS2_MOUSE 1
#undef  DEVICE_I386_SERIAL
#undef  DEVICE_I386_SERIAL_MOUSE

/*
 * Non Real Time
 */
#undef  CONFIG_RT_SERIALIZER


/* Scheduler related */
/* The number of switches amongst the GOSs during one cycle
 * is not fix and varies depending on the offlines scheduler's output
 * Assuming we don't have very large schedules
 */
#define MAX_NO_OF_SWITCHES              10 

/* This is to conditionally include or exclude the
 * performance analysis code
 */
#define PERF_ANALYSIS_ON                1