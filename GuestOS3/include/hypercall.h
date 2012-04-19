#ifndef __HYPERCALL_H
#define __HYPERCALL_H

long	spark_print(int);
long	spark_ni_hypercall(void);
long	spark_registerTimer(int,unsigned int, unsigned long);
long	spark_registerPages(int ,unsigned int);
long	spark_loadPDE(unsigned long);
long	spark_initFinish(void);
long	spark_serial_out(unsigned long);
long	spark_parallel_out(unsigned char);
#endif
