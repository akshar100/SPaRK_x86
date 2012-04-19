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
_hypercall1(long, spark_serial_out , int , str)
_hypercall1(long, spark_parallel_out , unsigned char , bData)
_hypercall0(long, spark_initFinish)
_hypercall2(long, spark_register_interrupt, int , intnum , unsigned int , loc )
//PCI_DRIVER
_hypercall2(long, spark_pci1710_digital_in, char* , buffer , unsigned long , size )
_hypercall2(long, spark_pci1710_digital_out, char* , buffer , unsigned long , size )
_hypercall2(long, spark_pci1710_analog_in, char* , buffer , unsigned long , size )
_hypercall2(long, spark_pci1710_analog_out, char* , buffer , unsigned long , size )
//PCI_DRIVER ~
_hypercall2(long, spark_registerMQ, unsigned long , ulChannelId , unsigned long , ulMemSize )
_hypercall3(long, spark_send, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)
_hypercall3(long, spark_receive, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)


/* Any new definition added over here should be declared in include/hypercall.h */
