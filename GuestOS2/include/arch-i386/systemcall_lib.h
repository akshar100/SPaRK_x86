
#ifndef _ASM_I386_SYSTEMCALL_H_
#define _ASM_I386_SYSTEMCALL_H_

#include <errno.h>
/*
 * This file contains the system call numbers.
 */

#define NR_systemcalls 22 

#define __NR_sys_print			0
#define __NR_pthread_make_periodic_np	1
#define __NR_pthread_wait_np		2
// ITC..
#define __NR_sys_send			3
#define __NR_sys_recv			4
// ITC~
//PCI_DRIVER
#define __NR_sys_pci1710_digital_in	5
#define __NR_sys_pci1710_digital_out	6
#define __NR_sys_pci1710_analog_in	7
#define __NR_sys_pci1710_analog_out	8
//PCI_DRIVER ~
#define __NR_sys_registerMQ             9
#define __NR_sys_sendMQ	                10
#define __NR_sys_receiveMQ              11
#define __NR_sys_reg_intrpt             12
#define __NR_sys_get_kbd_data           13
#define __NR_sys_getTime                14
#define __NR_sys_printLong              15
#define __NR_sys_getTaskInfo            16
#define __NR_sys_sock_init              17
#define __NR_sys_sock_listen            18
#define __NR_sys_sock_tcp_recv          19
#define __NR_sys_sock_tcp_send          20
#define __NR_sys_sock_connect           21


#define __systemcall_return(type, res) \
do { \
	if ((unsigned long)(res) >= (unsigned long)(-125)) { \
		errno = -(res); \
		res = -1; \
	} \
	return (type) (res); \
} while (0)


/* XXX - _foo needs to be __foo, while __NR_bar could be _NR_bar. */

#define _systemcall0(type,name) \
static type name(void) \
{ \
long __res; \
__asm__ volatile ("int $0x90" \
	: "=a" (__res) \
	: "0" (__NR_##name)); \
__systemcall_return(type,__res); \
}

#define _systemcall1(type,name,type1,arg1) \
static type name(type1 arg1) \
{ \
long __res; \
__asm__ volatile ("int $0x90" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)) : "memory"); \
__systemcall_return(type,__res); \
}

#define _systemcall2(type,name,type1,arg1,type2,arg2) \
static type name(type1 arg1,type2 arg2) \
{ \
long __res; \
__asm__ volatile ("int $0x90" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)) : "memory"); \
__systemcall_return(type,__res); \
}

#define _systemcall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
static type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
long __res; \
__asm__ volatile ("int $0x90" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
                  "d" ((long)(arg3)) : "memory"); \
__systemcall_return(type,__res); \
}

#define _systemcall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
static type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
long __res; \
__asm__ volatile ("int $0x90" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)),"S" ((long)(arg4)) : "memory"); \
__systemcall_return(type,__res); \
}

#define _systemcall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
          type5,arg5) \
static type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) \
{ \
long __res; \
__asm__ volatile ("int $0x90" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)),"S" ((long)(arg4)),"D" ((long)(arg5)) : "memory"); \
__systemcall_return(type,__res); \
}

#define _systemcall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
          type5,arg5,type6,arg6) \
static type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5,type6 arg6) \
{ \
long __res; \
__asm__ volatile ("push %%ebp ; movl %%eax,%%ebp ; movl %1,%%eax ; int $0x90 ; pop %%ebp" \
        : "=a" (__res) \
        : "i" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)),"S" ((long)(arg4)),"D" ((long)(arg5)), \
          "0" ((long)(arg6)) : "memory"); \
__systemcall_return(type,__res); \
}

#endif
