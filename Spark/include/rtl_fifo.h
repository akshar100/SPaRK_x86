/*
 * RTLinux FIFOs
 *
 *  Copyright (C) 1999  Finite State Machine Labs Inc. (http://www.fsmlabs.com/)
 *  Written by Michael Barabanov <baraban@fsmlabs.com>, 1995-2000
 */

#ifndef __RTF__
#define __RTF__

#include <rtl_conf.h>

#define RTF_NO (CONFIG_RTL_NFIFOS)
#define RTL_MAX_USER_FIFO RTF_NO

#define RTF_MAX_FIFO (RTL_MAX_USER_FIFO + 8)
#define RTF_SETSIZE 1

#ifdef __KERNEL__
extern int rtf_init(void);

extern int rtf_create_handler(unsigned int fifo,
		int (*handler)(unsigned int fifo));


extern int rtf_create_rt_handler(unsigned int fifo,
		int (*handler)(unsigned int fifo));


struct module;
extern struct module __this_module;
extern int __rtf_create(unsigned int fifo, int size, struct module *mod);

#define rtf_create(fifo, size) __rtf_create(fifo, size, &__this_module)

/* bidirectional FIFOs:
 * fifo is for rtf_get (user-space write),
 * fifo_put is for rtf_put (user-space read)
 */ 
extern int rtf_make_user_pair (unsigned int fifo, unsigned int fifo_put);


extern int rtf_link_user_ioctl (unsigned int fifo,
		int (*handler)(unsigned int fifo, unsigned int cmd, unsigned long arg));

extern int rtf_destroy(unsigned int fifo);

extern int rtf_flush(unsigned int fifo);

extern int rtf_isempty(unsigned int fifo);

extern int rtf_isused(unsigned int fifo);

extern int rtf_resize(unsigned int minor, int size);

extern int rtf_put(unsigned int fifo, void * buf, int count);

extern int rtf_get(unsigned int fifo, void * buf, int count);


#define RTFSETSIZE 0
#define RTFMMAPABLE 1

#endif

#endif
