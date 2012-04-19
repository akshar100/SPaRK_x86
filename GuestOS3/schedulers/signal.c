 /*
 * RTLinux signal support
 *
 * Written by Michael Barabanov
 * Copyright (C) Finite State Machine Labs Inc., 2000
 * Released under the terms of the GPL Version 2
 *
 *  Added user signals support - Dec, 2002 Josep Vidal <jvidal@disca.upv.es> (OCERA)
 *
 * StandAlone RTLinux integration 
 * written by Vicente Esteve LLoret <viesllo@inf.upv.es> 
 * Copyright (C) Feb, 2003 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * 
 */
#include <arch/mprot.h>
#include <arch/rtl_irqs.h>
#include <signal.h>
#include <errno.h>
#include <rtl_core.h>
#include <rtl_sched.h>

/* Thread signals */ /* F. González & J. Vidal */
/* Array of sigactions. Thread signals from (7..31) */

struct sigaction rtl_sigact [RTL_SIGIRQMAX];

/* for compatibility issues */
#define sigact rtl_sigact

unsigned int rtl_sig_interrupt (unsigned int irq, struct pt_regs *regs)
{
  int sig = irq + RTL_SIGIRQMIN;
  if (sigact[sig].sa_handler != SIG_IGN && sigact[sig].sa_handler != SIG_DFL) 
  {
    sigact[sig].sa_sigaction(sig, NULL, NULL);
  }
	/* shall we reenable here? */
  return 0;
}

/* Thread signals added. */ 
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
  int flags;
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif
  int irq = -1;

  STARTKERNELCODE(mprot);

  if (act && (act->sa_flags & SA_IRQ)) 
  {
    irq = sig;
  } 
  else 
  {
    /* for now, only hard global interrupts 
       & thread signals from RTL_SIGNAL_READY+1 to  
       RTL_MAX_SIGNAL 
     */
    if (sig <= RTL_SIGNAL_READY || (sig > RTL_MAX_SIGNAL 
      && sig < RTL_SIGIRQMIN) || sig >= RTL_SIGIRQMIN + NR_IRQS) 
    {
      errno = EINVAL;
      ENDKERNELCODE(mprot);
      return -1;
    }
    irq = sig - RTL_SIGIRQMIN;
  }
  
  if (oact) 
  {
    *oact = sigact[sig];
    /*hard global interrupts */
    if (sig > RTL_MAX_SIGNAL) 
    {
//      rtl_irq_set_affinity(irq, NULL, &oact->sa_focus);
    };
  }
  if (!act) 
  
  {
    return 0;
  }

	/*hard global interrupts */
  if (sig > RTL_MAX_SIGNAL)
  {
    if (sigact[sig].sa_handler != act->sa_handler) 
    {
      /* free old irq first if needed */
      if (sigact[sig].sa_handler != SIG_IGN
          && sigact[sig].sa_handler != SIG_DFL) 
      {
//         rtl_free_global_irq (irq);
      }
      /* now request */
      if (act->sa_handler != SIG_IGN && act->sa_handler != SIG_DFL) 
      {
        sigact[sig] = *act;
//        rtl_request_global_irq (irq, rtl_sig_interrupt);
      }
    }

    if ( act->sa_flags & SA_FOCUS) 
    {
//      rtl_irq_set_affinity (irq, &act->sa_focus, NULL);
    }
  }	/*End hard global interrupts */

  rtl_no_interrupts(flags);
  sigact[sig].sa_handler = act->sa_handler;

  if (sig > RTL_SIGNAL_READY && sig <= RTL_MAX_SIGNAL )
  {
    if (sigact[sig].sa_handler == SIG_IGN
        || sigact[sig].sa_handler == SIG_DFL)
    {
      sigact[sig].sa_handler = NULL;
    }
  }
  sigact[sig].sa_flags = act->sa_flags;
  sigact[sig].sa_focus = act->sa_focus;
  sigact[sig].sa_mask = act->sa_mask;
  sigact[sig].owner = pthread_self();

  rtl_restore_interrupts(flags);
  ENDKERNELCODE(mprot);
  
  return 0;
}

int pthread_sigmask(int how, const rtl_sigset_t *set, rtl_sigset_t *oset)
{
 pthread_t self=pthread_self();
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  
  int err=0,flags;
  
  STARTKERNELCODE(mprot);

  if (oset) *oset=self->blocked;

  if (!set) {
    ENDKERNELCODE(mprot);
    return EFAULT;
  };
 
  /*
    With pthread_sigmask RTLINUX scheduler signals can't be blocked
    or unblocked. For this reason the RTL_THREAD_SIGNALS_MASK is 
    applied.
  */

  rtl_no_interrupts(flags);

  switch (how){
  case SIG_SETMASK:
    self->blocked=( self->blocked & ~RTL_THREAD_SIGNALS_MASK) | 
      (RTL_THREAD_SIGNALS_MASK & *set);
    break;
  case SIG_BLOCK:
    self->blocked=( self->blocked & ~RTL_THREAD_SIGNALS_MASK) | 
      (RTL_THREAD_SIGNALS_MASK & (self->blocked | *set));
    break;
  case SIG_UNBLOCK:
    self->blocked=( self->blocked & ~RTL_THREAD_SIGNALS_MASK) | 
      (RTL_THREAD_SIGNALS_MASK & (self->blocked & ~(*set)));
    break;
  default:
    rtl_restore_interrupts(flags);
    ENDKERNELCODE(mprot);
    return EINVAL;
  }

  rtl_restore_interrupts(flags);
  ENDKERNELCODE(mprot);
  return err;
} 


int sigsuspend(const rtl_sigset_t *sigmask){
  rtl_sigset_t oset;
  int flags;  
#if CONFIG_KERNEL_MEMORYPROT
  mprot_t mprot;
#endif  
  pthread_t self=pthread_self();
 
  STARTKERNELCODE(mprot);
  
  if (!sigmask) {
    errno=EFAULT;
    ENDKERNELCODE(mprot);
    return -1;
  }

  rtl_no_interrupts(flags);

  // Backup old mask.
  oset=self->blocked;

  /*
    Instaure new.

    With pthread_sigmask RTLINUX scheduler signals can't be blocked
    or unblocked.     
    0 .. 6 from oset, 7 .. 31 to 0 OR 
    0 .. 6 to 0, 7 .. 31 from sigmask    
  */
  self->blocked= (oset & ~RTL_THREAD_SIGNALS_MASK) | (*sigmask & RTL_THREAD_SIGNALS_MASK) ;
  // Mark as interrumpible by a signal.
  set_bit(RTL_THREAD_SIGNAL_INTERRUMPIBLE,&self->threadflags);
  // Suspend calling thread until a signal arrives.
  pthread_suspend_np(self);  
  
  // do_user_signal clears the bit RTL_THREAD_SIGNAL_INTERRUMPIBLE from threadflags.
  if (!test_and_clear_bit(RTL_THREAD_SIGNAL_INTERRUMPIBLE,&self->threadflags)){
    errno=EINTR;
  }

  // Restore thread blocked set.
  self->blocked=oset;
  rtl_restore_interrupts(flags);
 
  ENDKERNELCODE(mprot);
  
  return -1;
}

int sigpending(rtl_sigset_t *set)
{
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  

  STARTKERNELCODE(mprot);

  if (set)
  {
    *set=pthread_self()->pending;
    ENDKERNELCODE(mprot);
    return 0;
  } 
  else 
  {
    errno=EFAULT;
    ENDKERNELCODE(mprot);
    return -1;
  }
    
}





































































