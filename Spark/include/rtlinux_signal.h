/*
 * (C) Finite State Machine Labs Inc. 2000 business@fsmlabs.com
 *
 * Released under the terms of GPL 2.
 * Open RTLinux makes use of a patented process described in
 * US Patent 5,995,745. Use of this process is governed
 * by the Open RTLinux Patent License which can be obtained from
 * www.fsmlabs.com/PATENT or by sending email to
 * licensequestions@fsmlabs.com
 */

/*
 * RTLinux signal and the user-level RTLinux signals conflict.
 * This is ok since users should NOT be making RTLinux calls
 * from normal user code outside of PSC.  -- Cort
 */
#ifdef __RTL_SIGNAL_H__
#error rtlinux_signal.h cannot be included with RTLinux posix/signal.h
#endif

#ifndef _RTLINUX_SIGNAL_H
#define _RTLINUX_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include <unistd.h>
#include <sys/types.h>
#ifndef __KERNEL__
#define __KERNEL__
#include <asm/irq.h>
#undef __KERNEL__
#else
#include <asm/irq.h>
#endif
#include <rtl_time.h>
/* handlers */
#define RTLINUX_SIG_IGN 	0
#define RTLINUX_SIG_DFL 	0
/* signals */
#define RTLINUX_SIGIRQ0		0
#define RTLINUX_SIGTIMER0	(NR_IRQS)
#define RTLINUX_SIGTIMER1	(RTLINUX_SIGTIMER0+1)
#define RTLINUX_SIGTIMER2	(RTLINUX_SIGTIMER0+2)
#define RTLINUX_SIGTIMER3	(RTLINUX_SIGTIMER0+3)
#define RTLINUX_SIGTIMER4	(RTLINUX_SIGTIMER0+4)
#define RTLINUX_SIGTIMER5	(RTLINUX_SIGTIMER0+5)
#define RTLINUX_SIGTIMER6	(RTLINUX_SIGTIMER0+6)
#define RTLINUX_SIGTIMER7	(RTLINUX_SIGTIMER0+7)
#define RTLINUX_SIGTIMER8	(RTLINUX_SIGTIMER0+8)
#define RTLINUX_SIGTIMER9	(RTLINUX_SIGTIMER0+9)
#define RTLINUX_SIGTIMER10	(RTLINUX_SIGTIMER0+10)
#define RTLINUX_SIGTIMER11	(RTLINUX_SIGTIMER0+11)
#define RTLINUX_SIGTIMER12	(RTLINUX_SIGTIMER0+12)
#define RTLINUX_SIGTIMER13	(RTLINUX_SIGTIMER0+13)
#define RTLINUX_SIGTIMER14	(RTLINUX_SIGTIMER0+14)
#define RTLINUX_SIGTIMER15	(RTLINUX_SIGTIMER0+15)
#define RTLINUX_SIGUSR0		(RTLINUX_SIGTIMER15+16)
#define RTLINUX_SIGUSR1		(RTLINUX_SIGUSR0+1)
#define RTLINUX_SIGUSR2		(RTLINUX_SIGUSR1+1)
#define RTLINUX_SIGUSR3		(RTLINUX_SIGUSR2+1)
#define RTLINUX_SIGMAX		(RTLINUX_SIGUSR3)
/* flags */
#define RTLINUX_SA_ONESHOT	1
#define RTLINUX_SA_RESETHAND	1
#define RTLINUX_SA_PERIODIC	0
/* sigprocmask modes */
#define RTLINUX_SIG_BLOCK 	0xa00
#define RTLINUX_SIG_UNBLOCK 	0xb00
#define RTLINUX_SIG_SETMASK	0xc00
#define RTLINUX_SIGNWORDS	(RTLINUX_SIGMAX / (8 * sizeof (unsigned long int))) + 1
	typedef struct {
		unsigned long int __val[RTLINUX_SIGNWORDS];
	} rtlinux_sigset_t;

	struct rtlinux_sigaction {
		int sa_signal;
		void (*sa_handler) (int);
		rtlinux_sigset_t sa_mask;
		int sa_flags;
		hrtime_t sa_period;
	};

	extern rtlinux_sigset_t rtlinux_blocked;
	extern hrtime_t gethrtime(void);

	int rtlinux_sigaction(int, struct rtlinux_sigaction *,
			      struct rtlinux_sigaction *);
	int rtlinux_sigprocmask(int, rtlinux_sigset_t *,
				rtlinux_sigset_t *);
	int rtlinux_sigemptyset(rtlinux_sigset_t *);
	int rtlinux_sigfillset(rtlinux_sigset_t *);
	int rtlinux_sigaddset(rtlinux_sigset_t *, int);
	int rtlinux_sigdelset(rtlinux_sigset_t *, int);
	int rtlinux_sigismember(const rtlinux_sigset_t *, int);

	int rtf_create(unsigned int fifo, int size);
	int rtf_destroy(unsigned int fifo);
	extern int rtf_put(unsigned int fifo, void *buf, int count);
	extern int rtf_get(unsigned int fifo, void *buf, int count);


#ifdef __cplusplus
}
#endif
#endif				/* _RTLINUX_SIGNAL_H */
