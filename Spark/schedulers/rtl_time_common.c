/*
 * RTLinux architecture-independent clock support
 *
 * 1999 Michael Barabanov <baraban@fsmlabs.com>
 *
 * Copyright (C) Finite State Machine Labs Inc., 1999
 */

#include <rtl_conf.h>
#include <rtl_time.h>
#include <rtl_core.h>

/* This is not really the right place but...*/
int *(*__rtl_errno_location)(void);

static int default_errno;
int *default_errno_location(void)
{
  return &default_errno;
}

int rtl_setclockhandler (rtl_clockid_t h, clock_irq_handler_t fn)
{
  
  h->handler = fn;
  
  return 0;
}

int rtl_unsetclockhandler (rtl_clockid_t h)
{
  if (h->handler == RTL_CLOCK_DEFAULTS.handler) 
  {
    return -EINVAL;
  }
  
  h-> handler = RTL_CLOCK_DEFAULTS.handler;
  
  return 0;
}

static int definit (struct rtl_clock *c) 
{ 
  return 0; 
}

static void defuninit (struct rtl_clock *c) 
{ 
  return; 
}

static hrtime_t defgethrtime (struct rtl_clock *c)
{
  return (hrtime_t) -1;
}

static int defsethrtime(struct rtl_clock *c, hrtime_t t)
{
  return -EINVAL;
}
static int defsettimer(struct rtl_clock *c, hrtime_t interval)
{
  return -1;
}
static int defsettimermode (struct rtl_clock *c, int mode)
{
  return -1;
}

static void default_handler( struct pt_regs *regs) {
		//_i8254_clock.settimer (&_i8254_clock, 5000000 );
}


#define RTL_CLOCK_INITIALIZER { \
	definit, \
	defuninit, \
	defgethrtime, \
	defsethrtime, \
	defsettimer, \
	defsettimermode, \
	default_handler, \
	RTL_CLOCK_MODE_UNINITIALIZED, \
	0, \
	0, \
	0, \
	PTHREAD_SPINLOCK_INITIALIZER, \
	RTL_CLOCK_ARCH_INITIALIZER};


struct rtl_clock RTL_CLOCK_DEFAULTS = RTL_CLOCK_INITIALIZER;

static struct rtl_clock clock_ust = RTL_CLOCK_INITIALIZER;

hrtime_t ust_gethrtime(struct rtl_clock *c)
{
  return gethrtime();
}

#include <rtl_debug.h>
#include <rtl_core.h>

rtl_clockid_t CLOCK_UST = &clock_ust;


int rtl_init_standard_clocks(void)
{
  __rtl_errno_location = &default_errno_location;
  clock_ust.gethrtime = &ust_gethrtime;
  clock_ust.resolution = gethrtimeres();
  return 0;
}

void rtl_cleanup_standard_clocks(void)
{
}
