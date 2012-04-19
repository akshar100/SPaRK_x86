#ifndef __RTL_POSIX_H__
#define __RTL_POSIX_H__

struct rtl_posix_thread_struct
{
	struct rtl_thread_struct *joining_thread;
	void *retval;
	void *joined_thread_retval;
	pthread_spinlock_t exitlock;
};

extern void rtl_posix_cleanup(void *retval);
extern void rtl_posix_init (pthread_t th);
extern void rtl_posix_on_delete(pthread_t th);
extern int pthread_detach(pthread_t thread);

extern int pthread_join(pthread_t thread, void **value_ptr);

#endif
