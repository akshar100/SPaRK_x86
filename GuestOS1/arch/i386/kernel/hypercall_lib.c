#include <arch/hypercall_lib.h>

/* All hyper call definitions over here */
_hypercall1(long, spark_print , int , str)
_hypercall0(long, spark_ni_hypercall)
_hypercall3(long, spark_registerTimer, int , iPeriod , unsigned int , iHandler , unsigned long , pllRealTime)
_hypercall2(long, spark_registerSysCallHandler, unsigned int , handler_address , int , intr_number )
_hypercall2(long, spark_registerPages, int , iProcessId , unsigned int , baseAddr )
_hypercall1(long, spark_loadPDE , unsigned long , pde)
_hypercall1(long, spark_printLong , unsigned long , pde)
_hypercall1(long, spark_setLevel2Stack , unsigned long , l2stack)

_hypercall2(long, spark_perf_measurement, unsigned long , parameter, unsigned long, preading)

_hypercall1(long, spark_parallel_out , unsigned char , bData)
_hypercall0(long, spark_rhine_poll)
_hypercall2(long, spark_register_interrupt, int , intnum , unsigned int , loc )
//PCI_DRIVER
_hypercall1(long, spark_rhine_get_ip, char* , buffer)
_hypercall0(long, spark_rhine_isready)

_hypercall2(long, spark_rhine_transmit, char* , buffer , unsigned int , size )
_hypercall2(long, spark_rhine_receive, char* , buffer , unsigned long *, ethhdr_type)
//PCI_DRIVER ~
_hypercall2(long, spark_registerMQ, unsigned long , ulChannelId , unsigned long , ulMemSize )
_hypercall3(long, spark_send, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)
_hypercall3(long, spark_receive, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)



/* Any new definition added over here should be declared in include/hypercall.h */
