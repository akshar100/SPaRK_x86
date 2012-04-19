#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include <errno.h>
#define SPIN_LOCK_UNLOCKED 0

typedef unsigned long spinlock_t;

static inline int spin_lock(spinlock_t *lock)
{
  asm volatile
    ("\n"
     "1:\n\t"
     "lock; decl %0\n\t"
     "js 2f\n\t"
     ".section .text.spinlock,\"ax\"\n"
     "2:\n\t"
     "cmpl $0,%0\n\t"
     "rep; nop\n\t"
     "jle 2b\n\t"
     "jmp 1b\n\t"
     ".previous"
     : "=m" (*lock));
  return 0;
}

static inline int spin_trylock(spinlock_t *lock)
{
  int oldval;

  asm volatile
    ("xchgl %0,%1"
     : "=r" (oldval), "=m" (*lock)
     : "0" (0));
  return oldval > 0 ? 0 : EBUSY;
}


static inline int spin_unlock (spinlock_t *lock)
{
  asm volatile
    ("movl $1,%0"
     : "=m" (*lock));
  return 0;
}


static inline int spin_lock_init (spinlock_t *lock)
{
  /* We can ignore the `pshared' parameter.  Since we are busy-waiting
     all processes which can access the memory location `lock' points
     to can use the spinlock.  */
  *lock = 1;
  return 0;
}


static inline int spin_destroy (spinlock_t *lock)
{
  /* Nothing to do.  */
  return 0;
}

#endif //_SPINLOCKS_H_

