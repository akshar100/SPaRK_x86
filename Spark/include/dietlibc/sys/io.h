#ifndef _SYS_IO_H
#define _SYS_IO_H

#include <sys/cdefs.h>


#define PARALLEL_PORT		0x378  /* Enter Your Port Address Here */
#define PARALLEL_DATA		PARALLEL_PORT+0
#define PARALLEL_STATUS		PARALLEL_PORT+1
#define PARALLEL_CONTROL	PARALLEL_PORT+2

__BEGIN_DECLS

extern int ioperm(unsigned long from, unsigned long num, int turn_on) __THROW;
extern int iopl(int level) __THROW;

#ifndef __STRICT_ANSI__
/* anyone have a cleaner solution for this mess? */
#ifdef __i386__
static inline unsigned char inb (unsigned short int port) {
  unsigned char _v;
  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static inline unsigned short inw (unsigned short int port) {
  unsigned short _v;
  __asm__ __volatile__ ("inw %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static inline unsigned int inl (unsigned short int port) {
  unsigned int _v;
  __asm__ __volatile__ ("inl %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static inline void outb (unsigned char value, unsigned short int port) {
  __asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}

static inline void outw (unsigned short value, unsigned short int port) {
  __asm__ __volatile__ ("outw %w0,%w1": :"a" (value), "Nd" (port));
}

static inline void outl (unsigned int value, unsigned short int port) {
  __asm__ __volatile__ ("outl %0,%w1": :"a" (value), "Nd" (port));
}
#endif
#endif

__END_DECLS

#endif
