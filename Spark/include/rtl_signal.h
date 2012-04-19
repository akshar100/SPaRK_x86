#ifndef RTL_SIGNAL_H
#define RTL_SIGNAL_H

#define RTL_MAX_SIGNAL 31 /* this is max for internal RTLinux signals */
/* these are bit positions */
#define RTL_SIGNAL_NULL 0 /* posix wants signal=0 to simply check */
#define RTL_SIGNAL_WAKEUP 1
#define RTL_SIGNAL_CANCEL 2
#define RTL_SIGNAL_SUSPEND 3
#define RTL_SIGNAL_TIMER 5
#define RTL_SIGNAL_READY 6

/*TODO How will this work on PPC */
#define RTL_LINUX_MIN_SIGNAL 256  /* signals to Linux start here. global then local */
#define RTL_LINUX_MAX_SIGNAL 1024
#define RTL_LINUX_MIN_LOCAL_SIGNAL  512
#define RTL_SIG_NOLINUX "SIGNAL NOLINUX IS UNDEFINED"

#define RTL_TIMED_OUT(x) rtl_sigismember((x), RTL_SIGNAL_TIMER)
#define RTL_SIGINTR(x) (0)


#endif
