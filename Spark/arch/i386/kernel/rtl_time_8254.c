/*
 * rtl_time.c
 * Developed at IITB 
 * Reference from RTLinux
*/

#include <rtl_conf.h>
#include <arch/hw_irq.h>
#include <arch/page.h>
#include <arch/timer.h>
#include <arch/rtl_io.h>
#include <rtl_core.h>
#include <rtl_debug.h>
#include <rtl_sync.h>
#include <rtl_time.h>
#include <rtl_printf.h>
#include <guests.h>
#include <rtl_sched.h>
#include <arch/processor.h>

//        Structs and Vars
//        




#define	schedule_handler(iHandler) \
do{ \
	__asm__ __volatile__("movl %%eax,0x30(%%esp) " : "=a" (iHandler)); \
} while(0)

	
#define rdtscll(val) \
	__asm__ __volatile__("rdtsc" : "=A" (val))

#define LATCH_CNT2() rtl_outb(0xd8,0x43);

#define READ_CNT2(var) \
do {var = rtl_inb(0x42); var |= (rtl_inb(0x42) << 8 );} while (0);

#define WRITE_COUNTER_ZERO16(x) do { \
	rtl_outb(x&0xff,0x40); rtl_outb((x>>8)&0xff,0x40);\
      	clock_counter =x; \
} while (0)

#define WRITE_COUNTER_ZERO_ONESHOT(x) WRITE_COUNTER_ZERO16(x)

#define wait_value(x) do {; } while ((rtl_inb(0x61) & 0x20) != (x))
#define wait_cycle() do { wait_value(0); wait_value(0x20); } while (0)

#define CLATCH (1024 * 32)
#define LATCH2 0x8000
#define NLOOPS 50
unsigned long scaler_8254_to_hrtime;
unsigned long scaler_hrtime_to_8254;
/* getting global time from Pentium TSC */
unsigned long scaler_pentium_to_hrtime = 0;
int can_change_latch2;

static volatile int last_c2;
/*static */spinlock_t lock8254;
struct rtl_clock _i8254_clock;
static hrtime_t (*rtl_do_get_time)(void);
static hrtime_t hrtime_resolution;
static long latch_ns;
static long max_latch_oneshot;
static unsigned int clock_counter; /* current latch value */
hrtime_t base_time;
hrtime_t last_8254_time;
long offset_time;
int iCallOn = 0;

#ifdef PERF_ANALYSIS_ON
/* Code inserted for demo START */
struct part_switch_perf partition_switch_performace;
struct sched_decision_perf schedule_decision_performace;
struct schedule_over_eta current_schedule[2];
unsigned long c_s_run_no = 0;
/* Code inserted for demo END */        
#endif
 
/***********************************************************************/
void timer_handler( struct pt_regs *regs);
/**************************************************************************/
void timer_handler( struct pt_regs *regs) {
	unsigned long *ptr;
	gCurrEtaCount++;
	if(iCallOn == 1) {
		//rtl_printf("In Icall on... returning.....\n");
		return;
	}

#ifdef PERF_ANALYSIS_ON
        /* Code inserted for demo START */
        schedule_decision_performace.t1 = gethrtime();         
        partition_switch_performace.t1 = schedule_decision_performace.t1; 
        /* Code inserted for demo END */        
#endif

	spark_schedule();
	
	// if the guest os has registered timer interrupt
	if (guestOS_thread[iCurrGuestOsIndex].iTicks)
	{
		guestOS_thread[iCurrGuestOsIndex].iCurrTicks++;
		// Here, if it is the time to give virtual timer interrupt to guest os 
		// then we need to do some more work
		if( guestOS_thread[iCurrGuestOsIndex].iCurrTicks == guestOS_thread[iCurrGuestOsIndex].iTicks)
		{
			guestOS_thread[iCurrGuestOsIndex].iCurrTicks = 0;

			// if the guest os interrupted when guest os kernel is running
			if (__GUESTOS_DS == regs->xss)
			{
				// need to give control to the Guest os Timer interrupt
				// and after the timer interrupt is being served in guest os 
				// the control should directly return to the point in the kernel from
				// where it is interrupted
				ptr = (unsigned long *)(regs->esp + guestOS_thread[iCurrGuestOsIndex].phyOffset);
				*(--ptr) = regs->eflags;
				*(--ptr) = regs->xcs;
				*(--ptr) = regs->eip;	
				// the next line should not be required but before removing that, it has to be tested
				regs->esp = (unsigned long)ptr - guestOS_thread[iCurrGuestOsIndex].phyOffset;
				regs->eip = guestOS_thread[iCurrGuestOsIndex].iHandler;

			}
			// if the guest os is interrupted when it is running in user mode,
			// or a user task is running when the guest os is interrupted
			else
			{
				ptr = (unsigned long *)(guestOS_thread[iCurrGuestOsIndex].stack_start_level2 + guestOS_thread[iCurrGuestOsIndex].phyOffset);
				*(--ptr) = regs->xss;
				*(--ptr) = regs->esp;
				*(--ptr) = regs->eflags;
				*(--ptr) = regs->xcs;
				*(--ptr) = regs->eip;
				regs->xss = __GUESTOS_DS;
				regs->xcs = __GUESTOS_CS;
				regs->esp = (unsigned long)ptr - guestOS_thread[iCurrGuestOsIndex].phyOffset;
				regs->eip = guestOS_thread[iCurrGuestOsIndex].iHandler;
				// rtl_printf("IRQ0 from level 3\n");
			}

		}
	}
#ifdef PERF_ANALYSIS_ON
        /* Code inserted for demo START */
        partition_switch_performace.t2 = gethrtime();         
        perf_scheduling_decision();
        perf_partition_switch();
        /* Code inserted for demo END */        
#endif
}

/**************************************************************************/
hrtime_t gethrtime(void)
{
  hrtime_t aux;
  aux = rtl_do_get_time();
  return aux;
}

hrtime_t _gethrtime(struct rtl_clock *c)
{
  return gethrtime();
}

hrtime_t gethrtimeres(void)
{
  return hrtime_resolution;
}

hrtime_t pent_gettime(void)
{
	hrtime_t t;
	/* time = counter * scaler_pentium_to_hrtime / 2^32 * 2^5; */
	/* Why 2^5? Because the slowest Pentiums run at 60 MHz */

	__asm__("rdtsc\n\t"
		"mov %%edx, %%ecx\n\t"
		"mul %%ebx\n\t"  	/* multiply the low 32 bits of the counter by the scaler_pentium */
		"mov %%ecx, %%eax\n\t"
		"mov %%edx, %%ecx\n\t"	/* save the high 32 bits of the product */
		"mul %%ebx\n\t" 	/* now the high 32 bits of the counter */
		"add %%ecx, %%eax\n\t"
		"adc $0, %%edx\n\t"
#if HRTICKS_PER_SEC == NSECS_PER_SEC
		"shld $5, %%eax, %%edx\n\t"
		"shl $5, %%eax\n\t"
#endif
		:"=A" (t) : "b" (scaler_pentium_to_hrtime) : "cx");
	return t;
}

hrtime_t global_8254_gettime (void)
{
  register unsigned int c2;
  int flags;
  long t;
  
  rtl_spin_lock_irqsave(&lock8254, flags);

  LATCH_CNT2();
  READ_CNT2(c2);
  offset_time += ((c2 < last_c2) ? (last_c2 - c2) / 2 : (last_c2 - c2 + LATCH2) / 2);
  last_c2 = c2;
  if (offset_time >= CLOCK_TICK_RATE) {
    offset_time -= CLOCK_TICK_RATE;
    base_time += HRTICKS_PER_SEC;
  };
  
#if HRTICKS_PER_SEC != CLOCK_TICK_RATE
  __asm__("shl $10, %%eax\n\t"
          "mul %%ebx\n\t"
          :"=d" (t) : "b" (scaler_8254_to_hrtime), "a" (offset_time));
#else
  t = offset_time;
#endif
  
  last_8254_time = base_time + t;

  rtl_spin_unlock_irqrestore(&lock8254, flags);

  return last_8254_time;
}

static hrtime_t periodic_gethrtime (struct rtl_clock *c) 
{ 
  return c->value; 
}

static hrtime_t oneshot_gethrtime (struct rtl_clock *c) 
{ 
  return gethrtime();
}

/* the 8254 clock */
static unsigned int _8254_irq(unsigned int irq, struct pt_regs *regs)
{
	int flags;

	rtl_spin_lock_irqsave (&lock8254, flags);
	if (_i8254_clock.mode == RTL_CLOCK_MODE_PERIODIC) {
		_i8254_clock.value += _i8254_clock.resolution;
	} else {
		_i8254_clock.arch.istimerset = 0;
	}

	rtl_spin_unlock_irqrestore (&lock8254, flags);
	_i8254_clock.handler(regs);

	return 0;
}

static inline long RTIME_to_8254_ticks(long t)
{
#if HRTICKS_PER_SEC != CLOCK_TICK_RATE
int dummy;
__asm__("mull %2"
	:"=a" (dummy), "=d" (t)
	:"g" (scaler_hrtime_to_8254), "0" (t)
	);
#endif
	
return (t);
}


static int _8254_setperiodic (rtl_clockid_t c, hrtime_t interval)
{
	long t;
	int flags;

	rtl_spin_lock_irqsave (&lock8254, flags);
	t = RTIME_to_8254_ticks (interval) + 1;
	WRITE_COUNTER_ZERO16 (t);
	_i8254_clock.value = gethrtime();
	_i8254_clock.resolution = interval;
	_i8254_clock.arch.istimerset = 1;
	rtl_spin_unlock_irqrestore(&lock8254, flags);

	return 0;
}

static int _8254_setoneshot (rtl_clockid_t c, hrtime_t interval)
{
	rtl_irqstate_t flags;
	long t;

	rtl_spin_lock_irqsave (&lock8254, flags);
	if (interval > max_latch_oneshot) {
		interval = max_latch_oneshot;
	}
	t = RTIME_to_8254_ticks (interval); 
	if (t < 1) {
		t = 1;
	}
	WRITE_COUNTER_ZERO_ONESHOT(t);
	_i8254_clock.arch.istimerset = 1;
	rtl_spin_unlock_irqrestore(&lock8254, flags);

	return 0;
}



int _8254_settimermode (struct rtl_clock *c, int mode)
{
	if (mode == _i8254_clock.mode) 
	{
		return 0;
	}
	if (mode == RTL_CLOCK_MODE_PERIODIC) 
	{
		rtl_outb_p(0x30, 0x43);
		rtl_outb_p(0x34, 0x43);
		_i8254_clock.mode = mode;
		_i8254_clock.gethrtime = periodic_gethrtime;
		_i8254_clock.settimer = _8254_setperiodic;
		_i8254_clock.arch.count_irqs = 0;
	} 
	else if (mode == RTL_CLOCK_MODE_ONESHOT) {
		rtl_outb_p(0x30, 0x43);
		_i8254_clock.mode = mode;
		_i8254_clock.gethrtime = oneshot_gethrtime;
		_i8254_clock.settimer = _8254_setoneshot;
		_i8254_clock.resolution = HRTICKS_PER_SEC / CLOCK_TICK_RATE;
	} else {
		return -EINVAL;
	}
	return 0;
}

static int _8254_init (rtl_clockid_t clock)
{
	int flags;
	rtl_no_interrupts (flags);
	rtl_do_get_time = pent_gettime;
	rtl_request_global_irq(0, _8254_irq);
	_8254_settimermode (clock, RTL_CLOCK_MODE_PERIODIC);
	_i8254_clock.settimer (clock, TIMER_PERIOD);
	enable_8259_irq(0); 
	rtl_restore_interrupts (flags);
	return 0;
}

static void _8254_uninit (rtl_clockid_t clock)
{
}

/* sort of a constructor */
int rtl_create_clock_8254(void)
{
	_i8254_clock = RTL_CLOCK_DEFAULTS;
	_i8254_clock.init = _8254_init;
	_i8254_clock.uninit = _8254_uninit;
	_i8254_clock.settimermode = _8254_settimermode;
	_i8254_clock.value = 0;
	_i8254_clock.handler = timer_handler;
	return 0;
}

/* returns a pointer to the clock structure of the best controlling hw clock 
 * for this CPU */
rtl_clockid_t rtl_getbestclock (unsigned int cpu)
{
	return &_i8254_clock;
}

/* scaler_pentium ==  2^32 / (2^5 * (cpu clocks per ns)) */
static void do_calibration(int do_tsc)
{
	long long t1 = 0;
	long long t2 = 0;
	long pps;
	int j;
	long result = 0;

	rtl_irqstate_t flags;
	rtl_no_interrupts(flags);
	rtl_outb((rtl_inb(0x61) & ~0x02) | 0x01, 0x61);
	rtl_outb_p(0xb6, 0x43);     /* binary, mode 3, LSB/MSB, ch 2 */
	rtl_outb(CLATCH & 0xff, 0x42);	/* LSB of count */
	rtl_outb(CLATCH >> 8, 0x42);	/* MSB of count */
	wait_cycle();
	if (do_tsc)
		rdtscll(t1);

	for (j = 0; j < NLOOPS; j++) {
		wait_cycle();
	}
	if (do_tsc)
		rdtscll(t2);
	if (do_tsc)
		result = t2 - t1;

	rtl_restore_interrupts(flags);

	if (do_tsc) {
		pps = muldiv (result, CLOCK_TICK_RATE, CLATCH * NLOOPS);
#if HRTICKS_PER_SEC == NSECS_PER_SEC
		scaler_pentium_to_hrtime = muldiv (1 << 27, HRTICKS_PER_SEC, pps);
#else
		scaler_pentium_to_hrtime = muldiv (1 << 31, HRTICKS_PER_SEC * 2, pps);
#endif
	} else  {
		scaler_pentium_to_hrtime = 0;
	}
}

static void init_hrtime (void)
{
	int flags;

	rtl_do_get_time = global_8254_gettime;
	hrtime_resolution = HRTICKS_PER_SEC / CLOCK_TICK_RATE;
#if HRTICKS_PER_SEC != CLOCK_TICK_RATE
	scaler_8254_to_hrtime = muldiv (HRTICKS_PER_SEC, 1 << 22, CLOCK_TICK_RATE);
	scaler_hrtime_to_8254 = muldiv (CLOCK_TICK_RATE, 1 << 31, HRTICKS_PER_SEC / 2);
	latch_ns = muldiv (LATCH, HRTICKS_PER_SEC, CLOCK_TICK_RATE);
#else
	latch_ns = LATCH;
#endif
	max_latch_oneshot = latch_ns * 3 / 4;
	rtl_no_interrupts(flags);

	/* program channel 2 of the 8254 chip for periodic counting */
	rtl_outb_p(0xb6, 0x43);     /* binary, mode 3, LSB/MSB, ch 2 */
	rtl_outb_p(LATCH2 & 0xff, 0x42);
	rtl_outb_p((LATCH2 >> 8) & 0xff, 0x42);
	rtl_outb_p((rtl_inb_p(0x61) & 0xfd) | 1, 0x61); /* shut up the speaker and enable counting */
	LATCH_CNT2();
	READ_CNT2(last_c2);
	offset_time = 0;
	base_time = 0;
	rtl_do_get_time = global_8254_gettime;
	hrtime_resolution = HRTICKS_PER_SEC / CLOCK_TICK_RATE;
	do_calibration(1);
	rtl_do_get_time = pent_gettime;
	hrtime_resolution = 32;
	rtl_restore_interrupts(flags);
}

void demo_init()
{
#if 0
    int i;
    partition_switch_performace.t1 = 0;
    partition_switch_performace.t2 = 0;
    partition_switch_performace.old_avg = 0;
    partition_switch_performace.elements = 0;
    for(i=0; i<MAX_NO_OF_READINGS; i++)
        partition_switch_performace.readings[i] = 0;

    schedule_decision_performace.t1 = 0;
    schedule_decision_performace.t2 = 0;
    schedule_decision_performace.old_avg = 0;
    schedule_decision_performace.elements = 0;
    for(i=0; i<MAX_NO_OF_READINGS; i++)
        schedule_decision_performace.readings[i] = 0;

    c_s_run_no = 0;
    while(c_s_run_no != 1)
    {
        current_schedule[c_s_run_no].index = 0;
        for(i=0; i<MAX_NO_OF_SWITCHES; i++)
        {
            current_schedule[c_s_run_no].schedule_over_eta[i].start_time = 0;
            current_schedule[c_s_run_no].schedule_over_eta[i].end_time = 0;
        }
        c_s_run_no++;
    }
    c_s_run_no = 0;
#endif
}

void perf_scheduling_decision()
{
    hrtime_t new_avg = 0;  
    hrtime_t old_value = 0;  
    hrtime_t new_value = 0;  
    if(schedule_decision_performace.t2  > schedule_decision_performace.t1)
    new_value = schedule_decision_performace.t2 - schedule_decision_performace.t1;
    else
    new_value = schedule_decision_performace.t1 - schedule_decision_performace.t2;

    if(schedule_decision_performace.elements < MAX_NO_OF_READINGS)
    {
         schedule_decision_performace.readings[schedule_decision_performace.elements] = new_value;
         new_avg = (  schedule_decision_performace.old_avg 
                    * schedule_decision_performace.elements 
                    + new_value)
                    / (schedule_decision_performace.elements + 1);
    }
    else
    {
         old_value = schedule_decision_performace.readings[schedule_decision_performace.elements];
         schedule_decision_performace.readings[schedule_decision_performace.elements] = new_value;
         
         new_avg = schedule_decision_performace.old_avg - old_value + new_value; 
    }
    schedule_decision_performace.old_avg = new_avg;
    schedule_decision_performace.elements = (schedule_decision_performace.elements + 1) % MAX_NO_OF_READINGS;

    return;
}

void perf_partition_switch()
{
    hrtime_t new_avg = 0;  
    hrtime_t old_value = 0;  
    hrtime_t new_value = 0;  
    if(partition_switch_performace.t2  > partition_switch_performace.t1)
    new_value = partition_switch_performace.t2 - partition_switch_performace.t1;
    else
    new_value = partition_switch_performace.t1 - partition_switch_performace.t2;

    if(partition_switch_performace.elements < MAX_NO_OF_READINGS)
    {
         partition_switch_performace.readings[partition_switch_performace.elements] = new_value;
         new_avg = (  partition_switch_performace.old_avg 
                    * partition_switch_performace.elements 
                    + new_value)
                    / (partition_switch_performace.elements + 1);
    }
    else
    {
         old_value = partition_switch_performace.readings[partition_switch_performace.elements];
         partition_switch_performace.readings[partition_switch_performace.elements] = new_value;
         
         new_avg = partition_switch_performace.old_avg + new_value - old_value;
    }
    partition_switch_performace.old_avg = new_avg;
    partition_switch_performace.elements = (partition_switch_performace.elements + 1) % MAX_NO_OF_READINGS;

    return;
}



int init_clocks (void)
{
	int flags;

	rtl_spin_lock_init (&lock8254);
	init_hrtime();
	rtl_no_interrupts(flags);
	rtl_create_clock_8254();
	rtl_init_standard_clocks();
	rtl_restore_interrupts(flags);
        demo_init();
	return 0;
}


