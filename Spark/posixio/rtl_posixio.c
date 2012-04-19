/*
 * (C) Finite State Machine Labs Inc. 1999-2001 <business@fsmlabs.com>
 *
 * Released under the terms of GPL 2.
 * Open RTLinux makes use of a patented process described in
 * US Patent 5,995,745. Use of this process is governed
 * by the Open RTLinux Patent License which can be obtained from
 * www.fsmlabs.com/PATENT or by sending email to
 * licensequestions@fsmlabs.com
 */
/*
 *  Originally derived from the Linux VFS code
 *  Copyright (C) 1991-1993  Linus Torvalds
 */

#include <rtl_conf.h>
#include <arch/mprot.h>
#include <rtl_time.h>
#include <rtl_posixio.h>
#include <rtl_devices.h>

#include <sys/mman.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#if _RTL_POSIX_IO

//
// First 4 descriptors reserved for Standard Devices in/out/err/mouse
//
//
// IN and MOUSE devices must perform polling reads to avoid lock Microwindows Interface
//

int STDIN;
int STDOUT;
int STDERR;
int STDMOUSE;

static struct rtl_file rtl_files [MAX_RTL_FILES] /* TODO __attribute__ ((aligned (64))) */ = {
	{ NULL, 0 },
};


struct device_struct {
  const char * name;
  struct rtl_file_operations * fops;
};

static struct device_struct rtldevs[MAX_CHRDEV] = {
  { NULL, NULL },
};

int rtl_register_rtldev(unsigned int major, const char * name, struct rtl_file_operations *fops)
{
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  
  STARTKERNELCODE(mprot);
  
  if (major >= MAX_CHRDEV)
    return -EINVAL;
  if (rtldevs[major].fops && rtldevs[major].fops != fops) {
    return -EBUSY;
  }
  rtldevs[major].name = name;
  rtldevs[major].fops = fops;
  ENDKERNELCODE(mprot);
  
  return 0;
}

int rtl_unregister_rtldev(unsigned int major, const char * name)
{
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  

  STARTKERNELCODE(mprot);
  
  if (major >= MAX_CHRDEV)
    return -EINVAL;
  if (!rtldevs[major].fops)
    return -EINVAL;
  if (strcmp(rtldevs[major].name, name))
    return -EINVAL;
  rtldevs[major].name = NULL;
  rtldevs[major].fops = NULL;

  ENDKERNELCODE(mprot);
  
  return 0;
}



/* we assume all RTLinux file names are of the form /dev/devicename<number> */
int open(const char *pathname, int flags, ...)
{
  int i;
  int minor;
  int major;
  char devname[200];
  char *p;
  int ret;
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  

  STARTKERNELCODE(mprot);

  if (strncmp(pathname, "/dev/", 5)) {
    __set_errno(ENOENT);
    ENDKERNELCODE(mprot);
    return -1; /* here we can use some other name resolution scheme */
  }
  i = 0;
  while (i < sizeof(devname) - 1 && *(pathname + 5 + i) && 
	 !isdigit(*(pathname + 5 + i))) {
            devname[i] = *(pathname + 5 + i);
	    i++;
  }
  devname[i] = 0;
  if (isdigit(*(pathname + 5 + i))) {
    minor = strtoul (pathname + 5 + i, &p, 10);
  } else if (!*(pathname + 5 + i)) {
	    minor = 0; /* like /dev/mem */
	 } else {
	          __set_errno(ENOENT);
                  ENDKERNELCODE(mprot);
		  return -1;
             	}
	
  for (i = 0; i < MAX_CHRDEV; i ++) {
    if (rtldevs[i].name && !strcmp(rtldevs[i].name, devname)) {
      goto found_major;
    }
  }
  
  __set_errno(ENOENT);
  ENDKERNELCODE(mprot);
  return -1;
found_major:
  major = i;

  for (i = 0; i < MAX_RTL_FILES; i ++) {
    if (!rtl_files[i].f_op) {
      goto found_free;
    }
  }
  __set_errno(ENOMEM);
  ENDKERNELCODE(mprot);
  return -1;
found_free:
  rtl_files[i].f_op = rtldevs[major].fops;
  rtl_files[i].f_minor = minor;
  rtl_files[i].f_flags = flags;

  ret = rtl_files[i].f_op->open(&rtl_files[i]);
  if (ret < 0) {
    __set_errno(-ret);
    ENDKERNELCODE(mprot);
    return -1;
  }
  ENDKERNELCODE(mprot);
  return i;
}


#define CHECKFD(fd) \
	if ((unsigned int) (fd) >= MAX_RTL_FILES || !rtl_files[fd].f_op) { \
          __set_errno(EBADF); \
	  return -1; \
	}

extern int rtl_get_minor(int fd)
{
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  
  int     minor;

  STARTKERNELCODE(mprot);

  CHECKFD(fd);
  minor = RTL_MINOR_FROM_FILEPTR(&rtl_files[fd]);
  
  ENDKERNELCODE(mprot);

  return minor;
}

int close(int fd)
{
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  
  STARTKERNELCODE(mprot);
  CHECKFD(fd);
  rtl_files[fd].f_op->release(&rtl_files[fd]);
  rtl_files[fd].f_op = NULL;
  ENDKERNELCODE(mprot);
  return 0;
}

int write(int fd, const void *buf, size_t count)
{
  int ret;
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  
  STARTKERNELCODE(mprot);
  CHECKFD(fd);
  ret = rtl_files[fd] . f_op -> write (&rtl_files[fd], buf, count, &rtl_files[fd].f_pos);
  if (ret < 0) {
    __set_errno(-ret);
    ENDKERNELCODE(mprot);
    return -1;
  }
  ENDKERNELCODE(mprot);
  return ret;
}

int read(int fd, void *buf, size_t count)
{
  int ret;
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  
  STARTKERNELCODE(mprot);
  CHECKFD(fd);
  ret = rtl_files[fd] . f_op -> read (&rtl_files[fd], buf, count, &rtl_files[fd].f_pos);
  if (ret < 0) {
    __set_errno(-ret);
    ENDKERNELCODE(mprot);
    return -1;
  }
  ENDKERNELCODE(mprot);
  return ret;
}


/* TODO lseek */
off_t lseek(int fd,off_t offset,int whence)
{
#if CONFIG_KERNEL_MEMORYPROT  
 mprot_t mprot;
#endif 
 STARTKERNELCODE(mprot);
 CHECKFD(fd);
// ret = (off_t) rtl_files[fd].f_op->llseek(fd,loffset,whence);

 rtl_files[fd].f_pos = offset;
 ENDKERNELCODE(mprot);
 return offset;
}

void *mmap(void  *start,  size_t length, int prot , int flags, int fd, off_t offset)
{
  int ret;
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  
  STARTKERNELCODE(mprot);
  if ((unsigned int) (fd) >= MAX_RTL_FILES || !rtl_files[fd].f_op) {
    __set_errno(EBADF);
    ENDKERNELCODE(mprot);
    return (void *) -1;
  }
  if (! rtl_files[fd] . f_op -> mmap) {
    __set_errno(EINVAL);
    ENDKERNELCODE(mprot);
    return (void *) -1;
  }

  ret = rtl_files[fd] . f_op -> mmap (&rtl_files[fd], start, length, prot, flags, offset);

  if (ret != 0) {
    __set_errno(-ret);
    ENDKERNELCODE(mprot);
    return (void *) -1;
  }
  ENDKERNELCODE(mprot);
  return (void *) 0;
}


int munmap(void *start, size_t length)
{
//iounmap (start);
  return 0;
}


int ioctl(int fd, int request, ...)
{
  int ret;
  va_list list;
  unsigned long arg;
#if CONFIG_KERNEL_MEMORYPROT  
  mprot_t mprot;
#endif  
  
  STARTKERNELCODE(mprot);
  va_start (list, request);
  arg = va_arg(list, unsigned long);
  va_end (list);
	
  CHECKFD(fd);
  ret = rtl_files[fd] . f_op -> ioctl (&rtl_files[fd], request, arg);
  if (ret < 0) {
    __set_errno(-ret);
    ENDKERNELCODE(mprot);
    return -1;
  }
  ENDKERNELCODE(mprot);
  return 0;
}


#ifdef CONFIG_RTL_DEVMEM_SUPPORT

static int rtl_mem_open (struct rtl_file *filp)
{
  return 0;
}

static int rtl_mem_release (struct rtl_file *filp)
{
  filp->f_pos = 0;
  return 0;
}


static ssize_t rtl_mem_write(struct rtl_file *filp, const char *buf, size_t count, loff_t* ppos)
{
  memcpy(((char *) ((long) *ppos)),buf,count);
  *ppos += count;
  return count;

}

static ssize_t rtl_mem_read(struct rtl_file *filp, char *buf, size_t count, loff_t* ppos)
{
//  memcpy_fromio (buf, (long) *ppos, count);
  memcpy(buf,((char *) ((long) *ppos)),count);
  *ppos += count;
  return count;
}


static int rtl_mem_mmap (struct rtl_file *file, void  *start,  size_t length, int prot , int flags, off_t offset)
{
	/* TODO need to fail if MAP_FIXED was specified etc */
//  *result = ioremap (offset, length);
//  if (!*result) {
//    return -EINVAL;
//  }
  return 0;
}


static loff_t rtl_mem_llseek(struct rtl_file *file, loff_t offset, int origin)
{
  if (origin != SEEK_SET) {
    return -EINVAL;
  }
  return file->f_pos = offset;
}

static struct rtl_file_operations rtl_mem_fops = {
	rtl_mem_llseek,
	rtl_mem_read,
	rtl_mem_write,
	NULL,
	rtl_mem_mmap,
	rtl_mem_open,
	rtl_mem_release
};

#endif // CONFIG_RTL_DEVMEM_SUPPORT

static int rtl_null_open (struct rtl_file *filp)
{
  return 0;
}

static int rtl_null_release (struct rtl_file *filp)
{
  filp->f_pos = 0;
  return 0;
}


static ssize_t rtl_null_write(struct rtl_file *filp, const char *buf, size_t count, loff_t* ppos)
{
  return 0;

}

static ssize_t rtl_null_read(struct rtl_file *filp, char *buf, size_t count, loff_t* ppos)
{
  return 0;
}

#if 0
static int rtl_null_mmap (struct rtl_file *file, void  *start,  size_t length, int prot , int flags, off_t offset)
{
  return 0;
}
#endif

static loff_t rtl_null_llseek(struct rtl_file *file, loff_t offset, int origin)
{
  if (origin != SEEK_SET) {
    return -EINVAL;
  }
  return 0;
}

static struct rtl_file_operations rtl_null_fops = {
	rtl_null_llseek,
	rtl_null_read,
	rtl_null_write,
	NULL,
	NULL,
	rtl_null_open,
	rtl_null_release
};

int init_posixio(void)
{
  
#ifdef CONFIG_RTL_DEVMEM_SUPPORT
  if (rtl_register_rtldev (MEM_MAJOR, "mem", &rtl_mem_fops)) {
    return -EIO;
  }
#endif

  if (rtl_register_rtldev (MISC_MAJOR, "null", &rtl_null_fops)) {
    return -EIO;
  }

  STDIN = open("/dev/null",0);
  STDOUT = open("/dev/null",0);
  STDERR = open("/dev/null",0);
  STDMOUSE = open("/dev/null",0);

  return 0;
}


#endif //_RTL_POSIX_IO

