
#ifndef _ASM_I386_HYPERCALL_H_
#define _ASM_I386_HYPERCALL_H_

#include <errno.h>
/*
 * This file contains the hyper call numbers.
 */

#define __NR_spark_ni_hypercall		0
#define __NR_spark_print		1
#define __NR_spark_registerTimer	2
#define __NR_spark_registerPages	3
#define __NR_spark_loadPDE		4
#define __NR_spark_rhine_poll           5
#define __NR_spark_perf_measurement     6
#define __NR_spark_parallel_out		7
#define __NR_spark_registerSysCallHandler	8
#define __NR_spark_setLevel2Stack 		9
#define __NR_spark_printLong	 		10
#define __NR_spark_register_interrupt	 	11
#define __NR_spark_registerMQ			12
#define __NR_spark_send				13
#define __NR_spark_receive			14
//PCI_DRIVER
#define __NR_spark_rhine_get_ip 	 	15
#define __NR_spark_rhine_isready	 	16
#define __NR_spark_rhine_transmit         	17
#define __NR_spark_rhine_receive         	18
//PCI_DRIVER ~


#define __hypercall_return(type, res) \
do { \
	if ((unsigned long)(res) >= (unsigned long)(-125)) { \
		errno = -(res); \
		res = -1; \
	} \
	return (type) (res); \
} while (0)


/* XXX - _foo needs to be __foo, while __NR_bar could be _NR_bar. */

#define _hypercall0(type,name) \
type name(void) \
{ \
long __res; \
__asm__ volatile ("int $0x82" \
	: "=a" (__res) \
	: "0" (__NR_##name)); \
__hypercall_return(type,__res); \
}

#define _hypercall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
long __res; \
__asm__ volatile ("int $0x82" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)) : "memory"); \
__hypercall_return(type,__res); \
}

#define _hypercall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
long __res; \
__asm__ volatile ("int $0x82" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)) : "memory"); \
__hypercall_return(type,__res); \
}

#define _hypercall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
long __res; \
__asm__ volatile ("int $0x82" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
                  "d" ((long)(arg3)) : "memory"); \
__hypercall_return(type,__res); \
}

#define _hypercall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
long __res; \
__asm__ volatile ("int $0x82" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)),"S" ((long)(arg4)) : "memory"); \
__hypercall_return(type,__res); \
}

#define _hypercall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
          type5,arg5) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) \
{ \
long __res; \
__asm__ volatile ("int $0x82" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)),"S" ((long)(arg4)),"D" ((long)(arg5)) : "memory"); \
__hypercall_return(type,__res); \
}

#define _hypercall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
          type5,arg5,type6,arg6) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5,type6 arg6) \
{ \
long __res; \
__asm__ volatile ("push %%ebp ; movl %%eax,%%ebp ; movl %1,%%eax ; int $0x82 ; pop %%ebp" \
        : "=a" (__res) \
        : "i" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)),"S" ((long)(arg4)),"D" ((long)(arg5)), \
          "0" ((long)(arg6)) : "memory"); \
__hypercall_return(type,__res); \
}

#endif
