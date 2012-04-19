#include <arch/systemcall_lib.h>

/* All system call definitions over here */
//ITC..
_systemcall3(long, sys_send, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)
_systemcall3(long, sys_recv, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)
//ITC~

_systemcall1(long, sys_print , int , str)
_systemcall1(long, pthread_make_periodic_np,unsigned long, period)
_systemcall0(long, pthread_wait_np)
_systemcall2(long, sys_reg_intrpt, int, num, unsigned long, loc)
_systemcall1(long, sys_get_kbd_data,unsigned long, loc)
//PCI_DRIVER
_systemcall2(long, sys_pci1710_digital_in, char* , buffer , unsigned long , size )
_systemcall2(long, sys_pci1710_digital_out, char* , buffer , unsigned long , size )
_systemcall2(long, sys_pci1710_analog_in, char* , buffer , unsigned long , size )
_systemcall2(long, sys_pci1710_analog_out, char* , buffer , unsigned long , size )
//PCI_DRIVER ~
_systemcall2(long, sys_registerMQ, unsigned long , ulChannelId , unsigned long , ulMemSize )
_systemcall3(long, sys_sendMQ, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)
_systemcall3(long, sys_receiveMQ, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)
_systemcall1(long, sys_getTime , unsigned long , pTime)
_systemcall1(long, sys_printLong , unsigned long, number)
_systemcall1(long, sys_getTaskInfo, unsigned int, pTaskInfo)
_systemcall0(long, sys_sock_init)
_systemcall1(long, sys_sock_listen, unsigned long, port)
_systemcall3(long, sys_sock_tcp_recv, unsigned int, sockid, unsigned long, pbuffer, unsigned long, size)
_systemcall3(long, sys_sock_tcp_send, unsigned int, sockid, unsigned long, pbuffer, unsigned long, size)
_systemcall2(long, sys_sock_connect, unsigned long, pip_addr, unsigned long, port)

/* Any new definition added over here should be declared in include/systemcall.h */

static	void myitoaHex(int  i , char    *a)
{
	int j=0 , k = 0;
	a[k++] = '\n';	
	do{
		j = i%16;
		if (j < 10 )
			a[k++] = j + 0x30;
		else
		{
			switch(j)
			{
				case 10: a[k++] = 'a'; break;
				case 11: a[k++] = 'b'; break;
				case 12: a[k++] = 'c'; break;
				case 13: a[k++] = 'd'; break;
				case 14: a[k++] = 'e'; break;
				case 15: a[k++] = 'f'; break;
				default: ;
			}

		}

		i = i/16;
	}while(i);
	a[k] = '\0';	
	for( j = 0 ; j < k/2 ; j++)
	{
		a[j] = a[j]^a[k-j -1];
		a[k-j -1] = a[j]^a[k-j -1];
		a[j] = a[j]^a[k-j -1];
	}
}
