#ifndef __RTL_WAIT_H__
#define __RTL_WAIT_H__

#include <rtl_conf.h>
#include <rtl_spinlock.h>

#define RTL_WAIT_MAGIC 0x43743543

struct rtl_thread_struct;

struct rtl_wait_struct {
	int magic;
	struct rtl_thread_struct *waiter;
	struct rtl_wait_struct *next;
};

struct rtl_waitqueue_struct {
	struct rtl_wait_struct *queue;
	spinlock_t *p_lock;
};

typedef struct rtl_waitqueue_struct rtl_wait_t;

#define RTL_WAIT_INITIALIZER { 0 /*, SPIN_LOCK_UNLOCKED */}
#define rtl_wait_init(q) do { (q)->queue = 0; /* spin_lock_init(&((q)->lock)); */ } while (0)



extern int rtl_wait_sleep (rtl_wait_t *wait, spinlock_t *lock);
extern int rtl_wait_wakeup (rtl_wait_t *wait);


#endif //__RTL_WAIT_H__

