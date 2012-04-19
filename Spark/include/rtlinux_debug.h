/*
 * RTLinux debug routines
 *
 * Written by Michael Barabanov
 * Copyright (C) Finite State Machine Labs Inc., 1998-1999
 * Released under the terms of the GPL Version 2
 *
 * Idea for debugpr from include/linux/sunrpc/debug.h
 * Copyright (C) 1996, Olaf Kirch <okir@monad.swb.de>
 *
 */

#ifndef __RTLINUX_DEBUG__
#define __RTLINUX_DEBUG__


#define do_first(x) if (({static int __count=(x); __count-- > 0;}))

#define do_every(x) if (({static unsigned __count=x; !(__count++ % x); }))

#ifdef RTL_DEBUG_PRINT
#define debugpr(format, args...)	rtl_cprintf(format, ## args)
#else
#define debugpr(format, args...)	do { ; } while (0)
#endif

#ifdef __i386__
#define BREAKPOINT() asm("   int $3");
#elif defined(__powerpc__)
#define BREAKPOINT() asm(".long 0x7d821008");
#else
#define BREAKPOINT() do { ; } while (0)
#endif

/* This will generate a breakpoint exception.  It can be used at the
   beginning of a program to sync up with a debugger and can be used
   otherwise as a quick means to stop program execution and "break" into
   the debugger. */

int rtl_debug_initialized;
#define breakpoint() do { if (rtl_debug_initialized) BREAKPOINT(); } while (0)

#endif /* __RTL_DEBUG__ */

