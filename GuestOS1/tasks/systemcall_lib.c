#include <arch/systemcall_lib.h>
#include <time.h>
#include <uip/clock-arch.h>

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
_systemcall2(long, sys_rhine_get_ip, char* , buffer , unsigned long , size )
//_systemcall2(long, sys_rhine_isready, char* , buffer , unsigned long , size )
_systemcall0(long, sys_rhine_isready)
_systemcall2(long, sys_rhine_transmit, char* , buffer , unsigned long , size )
_systemcall2(long, sys_pci1710_analog_out, char* , buffer , unsigned long , size )
//PCI_DRIVER ~
_systemcall2(long, sys_registerMQ, unsigned long , ulChannelId , unsigned long , ulMemSize )
_systemcall3(long, sys_sendMQ, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)
_systemcall3(long, sys_receiveMQ, unsigned long , ulChannelId , char * , msg, unsigned long , ulDataSize)
_systemcall1(long, sys_getTime , unsigned long , pTime)
/* Any new definition added over here should be declared in include/systemcall.h */
_systemcall1(long, sys_printLong , unsigned long, number)

_systemcall1(long, sys_timerExpired , struct spark_timer *, ptimer)
_systemcall2(void, sys_timerSet, struct spark_timer *, ptimer, clock_time_t, interval)

_systemcall0(void, sys_uip_init)
_systemcall0(void, sys_uip_arp_init)
_systemcall3(void, sys_uip_ip_init, unsigned char *, hostaddr, unsigned char *, draddr, unsigned char *, netmask)
_systemcall0(void, sys_uip_setethaddr)
_systemcall1(unsigned long, sys_tapdev_read, unsigned long *, ethhdr_type)
_systemcall0(void, sys_uip_arp_ipin)
_systemcall0(void, sys_uip_input)
_systemcall0(void, sys_uip_arp_out)
_systemcall0(void, sys_tapdev_send)
_systemcall0(void, sys_uip_arp_arpin)

_systemcall1(void, sys_timerReset, struct spark_timer *, ptimer)
_systemcall1(void, sys_uip_periodic, unsigned long, conn_index)
_systemcall1(void, sys_uip_udp_periodic, unsigned long, conn_index)
_systemcall0(void, sys_uip_arp_timer)
_systemcall0(long, sys_uip_get_len)
_systemcall0(long, sys_uip_dummy)
_systemcall0(long, sys_uip_send_response)
_systemcall2(void, sys_uip_listen, unsigned long, port, unsigned long, gos_id)
_systemcall4(void, sys_uip_connect, unsigned long, ip_address, unsigned long, port, unsigned long, gos_id, unsigned long, sock_id)
_systemcall4(void, sys_uip_tcp_send, unsigned long, sock_id, unsigned long, gos_id, unsigned long, pbuffer, unsigned long, size)


_systemcall2(long, sys_perf_measurement, unsigned long, parameter, unsigned long, preading)
_systemcall1(void, sys_task_switch_time, unsigned long, ptime)

_systemcall0(long, sys_rhine_poll)

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
