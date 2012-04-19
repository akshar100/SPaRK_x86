#ifndef __RTL_ARCH_TIME_H__
#define __RTL_ARCH_TIME_H__

/*
 * rtl_time.h
 *
 * x86 hardware clock interface
 *
 * Copyright (C) 1999 Michael Barabanov
 * Released under the terms of the GPL
 *
 */

/* begin of what each arch should declare */

typedef long long hrtime_t; /* high-resolution time type (signed 64-bit) */


extern inline hrtime_t timespec_to_ns (const struct timespec *ts)
{
	long long t;

	__asm__("imull %%edx\n"
		"add %%ebx, %%eax\n\t"
		"adc $0, %%edx\n\t"
		:"=A" (t)
		: "a" (ts->tv_sec), "d" (NSECS_PER_SEC), "b" (ts->tv_nsec)
		);
	return t;
}

extern inline struct timespec timespec_from_ns (hrtime_t t)
{
	struct timespec ts;
	__asm__("idivl %%ecx\n\t"
		:"=a" (ts.tv_sec), "=d" (ts.tv_nsec)
		: "A" (t), "c" (NSECS_PER_SEC)
		);
	if (ts.tv_nsec < 0) {
		ts.tv_nsec += NSECS_PER_SEC;
		ts.tv_sec --;
	}
	return ts;
}

#ifdef __KERNEL__
#include <rtl_core.h>

extern inline hrtime_t hrt_to_8254 (hrtime_t t)
{
	__asm__("mov %%eax, %%ecx\n\t"
		"mov %%edx, %%eax\n\t"
		"xorl %%edx, %%edx\n\t"
		"idivl %%ebx\n\t"
		"xchg %%eax, %%ecx\n\t"
		"divl %%ebx\n\t"
		"mov %%ecx, %%edx\n\t"
		:"=A" (t) :"0" (t), "b" (838)
		:"cx");
	return t;
}

extern inline hrtime_t _8254_to_hrt (hrtime_t t)
{
	__asm__(
		"mov %%eax, %%ecx\n\t"
		"mov %%edx, %%eax\n\t"
		"mul %%ebx\n\t" 
		"xchg %%eax, %%ecx\n\t"
		"mul %%ebx\n\t"
		"add %%ecx, %%edx\n\t"
		:"=A" (t) : "b" (838), "0" (t): "cx");
	return t;
}

#define HRT_FROM_8254(x) (_8254_to_hrt(x))
#define HRT_TO_8254(x) (hrt_to_8254(x))

extern hrtime_t gethrtime(void); /* time in nanoseconds since bootup */
extern hrtime_t gethrtimeres(void); /* resolution of gethrtime() in ns */

struct rtl_clock_arch {
	int istimerset;
	int count_irqs;
	int apic_cpu;
	hrtime_t linux_time;
};

#define RTL_CLOCK_ARCH_INITIALIZER { 0, 0, 0, 0 }


/* end of public arch declarations */



extern struct rtl_clock _i8254_clock __attribute__ ((aligned (64)));
#define CLOCK_8254 (&_i8254_clock)

#ifdef CONFIG_X86_LOCAL_APIC
extern struct rtl_clock _apic_clock[RTL_NR_CPUS] __attribute__ ((aligned (64)));
#define CLOCK_APIC (&_apic_clock[rtl_getcpuid()])
#endif


/* TODO
extern clockid_t CLOCK_RTC;
 */

extern __inline__ unsigned long muldiv(unsigned long a, unsigned long mul, unsigned long div)
{
	int temp;
       __asm__("mull %2 ; divl %3"
               :"=a#" (a), "=d" (temp)
               :"1" (mul),
                "c" (div),
                "0" (a)
               );
       return a;
}

static inline void rtl_delay(long delay)
{
	hrtime_t t = gethrtime() + delay;
	while (gethrtime() < t);
}

extern int I8253_channel2_free(void);

#endif



/***********************************************************************/
/* Structure and variable required for system calls included just for demo.
 * These are not reuired for working of timer.
 * They are just additional utilities to added for demo.
 */
#define MAX_NO_OF_READINGS 50
struct part_switch_perf
{
   hrtime_t t1;
   hrtime_t t2;
   hrtime_t old_avg;
   hrtime_t elements;
   hrtime_t readings[MAX_NO_OF_READINGS];
};

struct sched_decision_perf
{
   hrtime_t t1;
   hrtime_t t2;
   hrtime_t old_avg;
   hrtime_t elements;
   hrtime_t readings[MAX_NO_OF_READINGS];
};

struct run_time_schedule
{
   hrtime_t start_time;
   unsigned long gos_id;
   hrtime_t end_time;
};

struct schedule_over_eta
{
    struct run_time_schedule schedule_over_eta[MAX_NO_OF_SWITCHES];
    unsigned int index;
    unsigned int no_of_switches;
};

#endif

