/*
 * RTLinux POSIX IO level
 *
 *  Copyright (C) 1999 FSM Labs (http://www.fsmlabs.com/)
 *  Written by Michael Barabanov <baraban@fsmlabs.com>
 */

#ifndef __RTL_POSIXIO_H__
#define __RTL_POSIXIO_H__

#include <rtl_conf.h>
#include <sys/types.h>
#include <stdio.h>

extern int STDIN;
extern int STDOUT;
extern int STDERR;
extern int STDMOUSE;

#define MAX_CHRDEV  255

// may disappear in future versions
extern int rtl_get_minor(int fd);


/* typedef int RTL_INODE; */
/* we only need the minor number */
#define RTL_MINOR_FROM_FILEPTR(fptr) ((fptr)->f_minor)

struct rtl_file_operations;

struct rtl_file {
	struct rtl_file_operations	*f_op;
	int f_minor;
	int f_flags;
	loff_t                  f_pos;
};

struct rtl_file_operations {
	loff_t (*llseek) (struct rtl_file *, loff_t, int);
	ssize_t (*read) (struct rtl_file *, char *, size_t, loff_t *);
	ssize_t (*write) (struct rtl_file *, const char *, size_t, loff_t *);
	int (*ioctl) (struct rtl_file *, unsigned int, unsigned long);
	int (*mmap) (struct rtl_file *, void  *start,  size_t length, int prot , int flags, loff_t offset);
	int (*open) (struct rtl_file *);
	int (*release) (struct rtl_file *);
};

extern int rtl_register_rtldev(unsigned int, const char *, struct rtl_file_operations *);
extern int rtl_unregister_rtldev(unsigned int major, const char * name);

extern int init_posixio(void);

/* compatibility */
#define rtl_register_chrdev rtl_register_rtldev
#define rtl_unregister_chrdev rtl_unregister_rtldev


#endif

