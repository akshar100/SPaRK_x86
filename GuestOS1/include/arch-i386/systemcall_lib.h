
#ifndef _ASM_I386_SYSTEMCALL_H_
#define _ASM_I386_SYSTEMCALL_H_

#include <errno.h>
/*
 * This file contains the system call numbers.
 */

#define NR_systemcalls 41

#define __NR_sys_print			0
#define __NR_pthread_make_periodic_np	1
#define __NR_pthread_wait_np		2
// ITC..
#define __NR_sys_send			3
#define __NR_sys_recv			4
// ITC~
//PCI_DRIVER
#define __NR_sys_rhine_get_ip   	5
#define __NR_sys_rhine_isready	        6
#define __NR_sys_rhine_transmit 	7
#define __NR_sys_pci1710_analog_out	8
//PCI_DRIVER ~
#define __NR_sys_registerMQ		9
#define __NR_sys_sendMQ			10
#define __NR_sys_receiveMQ		11
#define __NR_sys_reg_intrpt		12
#define __NR_sys_get_kbd_data		13
#define __NR_sys_getTime		14
#define __NR_sys_printLong	        15	

#define __NR_sys_timerExpired	        16	
#define __NR_sys_timerSet	        17	

#define __NR_sys_uip_init	        18	
#define __NR_sys_uip_arp_init	        19
#define __NR_sys_uip_ip_init	        20

#define __NR_sys_uip_setethaddr	        21
#define __NR_sys_tapdev_read    	22
#define __NR_sys_uip_arp_ipin	        23
#define __NR_sys_uip_input	        24
#define __NR_sys_uip_arp_out	        25
#define __NR_sys_tapdev_send    	26
#define __NR_sys_uip_arp_arpin	        27
#define __NR_sys_timerReset             28
#define __NR_sys_uip_periodic           29
#define __NR_sys_uip_udp_periodic       30
#define __NR_sys_uip_arp_timer          31
#define __NR_sys_uip_get_len            32
#define __NR_sys_uip_dummy              33
#define __NR_sys_uip_send_response      34
#define __NR_sys_uip_listen             35
#define __NR_sys_uip_connect            36
#define __NR_sys_uip_tcp_send           37
#define __NR_sys_perf_measurement       38
#define __NR_sys_task_switch_time       39
#define __NR_sys_rhine_poll             40




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
