#include <rtl_conf.h>
#include <rtl_sched.h>
#include <arch/linkage.h>
#include <rtl_time.h>
#include <time.h>

#include <uip/uip.h>
#include <uip/uip_arp.h>
#include <uip/timer.h>

int kbd_data_k;
unsigned char eth_data[1540]; 

/*****************************************************************************/
asmlinkage void sys_print_k(char *str)
{
	spark_print(str);
}
/*****************************************************************************/
asmlinkage void sys_printLong_k(unsigned long number)
{
	spark_printLong(number);
}

/*****************************************************************************/
// right now not using temp, may be used in later stages
// system call that call hypercall to register the interrupt 
// and also passes the address of a kernel variable
// right now we are using a single variable but it can also be a pointer
// of a structure a fifo or queue or just anything
asmlinkage void sys_reg_intrpt_k(int intr, unsigned int  temp)
{
	spark_register_interrupt(intr, &kbd_data_k);
}

/*****************************************************************************/
// sytem call that will copy the keyboard data from GOS kernel area to usre area
asmlinkage void  sys_get_kbd_data_k(int  *temp)
{
	*temp = kbd_data_k; 
}

/*****************************************************************************/
//PCI_DRIVER
asmlinkage long sys_rhine_get_ip_k(char *buffer)
{
     return spark_rhine_get_ip(buffer);
}

/*****************************************************************************/
asmlinkage long sys_rhine_isready_k()
{
	return spark_rhine_isready();
}

/*****************************************************************************/
asmlinkage long sys_rhine_transmit_k(char *buffer, unsigned long size)
{
	return spark_rhine_transmit(buffer,size);
}

/*****************************************************************************/
asmlinkage long sys_pci1710_analog_out_k(char *buffer, unsigned long size)
{
	return 5555;
}

/*****************************************************************************/
// System Call to register the messgage queue for IPC
asmlinkage long  sys_registerMQ_k(unsigned long ulChannelID, unsigned long ulMemSize)
{
	return spark_registerMQ(ulChannelID, ulMemSize);
}

/*****************************************************************************/
// System Call to send the data to messgage queue IPC
asmlinkage long  sys_sendMQ_k(unsigned long ulChannelID, char *msg, unsigned long ulDataSize)
{
	return spark_send(ulChannelID, msg, ulDataSize);
}

/*****************************************************************************/
// System Call to recieve data from message queue queue for IPC
asmlinkage long sys_receiveMQ_k(unsigned long ulChannelID, char *msg, unsigned long ulDataSize)
{
	return spark_receive(ulChannelID, msg, ulDataSize);
}

/*****************************************************************************/
// System call to get the current time
asmlinkage long sys_getTime_k(unsigned long *ptrTime)
{
	hrtime_t t;
	t = gethrtime();
	*ptrTime = (unsigned long)t;
	return 0;
}
/*****************************************************************************/
/* Specify the interval in milliseconds 
 * The interval should not be less than the resolution of the 
 * timer registered by the GOS 
 */
asmlinkage long sys_timerExpired_k(struct spark_timer * ptimer)
{
    unsigned long mytime = 0;
    if(ptimer->start != 0)
    {
        mytime = gethrtime();
        ptimer->msec += (mytime - ptimer->start)/1000000;
        ptimer->start = mytime;
        if(ptimer->msec >= ptimer->interval)
        {
            //ptimer->start = gethrtime();
            //ptimer->msec = 0;
            return 1;
        }
    }
    else
    {
        ptimer->start = gethrtime();
        ptimer->msec = 0;
    }
    return 0;
}
/*****************************************************************************/
/* Set a timer.
 * This function is used to set a timer for a time sometime in the
 * future. 
 */
asmlinkage void sys_timerSet_k(struct spark_timer *ptimer, clock_time_t interval)
{
  ptimer->interval = interval;
  ptimer->start = gethrtime();
  ptimer->msec = 0;
  return;
}
/*****************************************************************************/
asmlinkage void sys_uip_init_k()
{
  uip_init();
  return;
}

/*****************************************************************************/
asmlinkage void sys_uip_arp_init_k()
{
  uip_arp_init();
  return;
}
/*****************************************************************************/
asmlinkage void sys_uip_ip_init_k(unsigned char *hostaddr, unsigned char*draddr, unsigned char* netmask)
{
  uip_ipaddr_t ipaddr;
  uip_ipaddr(ipaddr, hostaddr[0], hostaddr[1], hostaddr[2], hostaddr[3]);
  uip_sethostaddr(ipaddr);
  uip_ipaddr(ipaddr, draddr[0], draddr[1], draddr[2], draddr[3]);
  uip_setdraddr(ipaddr);
  uip_ipaddr(ipaddr, netmask[0], netmask[1], netmask[2], netmask[3]);
  uip_setnetmask(ipaddr);
}
/*****************************************************************************/
asmlinkage void sys_uip_setethaddr_k()
{
  struct uip_eth_addr nic_addr;

  nic_addr.addr[0] = 0x00;
  nic_addr.addr[1] = 0x1c;
  nic_addr.addr[2] = 0xf0;
  nic_addr.addr[3] = 0xa6;
  nic_addr.addr[4] = 0x27;
  nic_addr.addr[5] = 0xe6;

  uip_setethaddr(nic_addr);
}
/*****************************************************************************/
asmlinkage long sys_tapdev_read_k(unsigned long * ethhdr_type)
{
  uip_len = spark_rhine_receive(uip_buf, ethhdr_type);
  return uip_len;
}
/*****************************************************************************/
asmlinkage void sys_uip_arp_ipin_k()
{
  uip_arp_ipin();
  return;
}
/*****************************************************************************/
asmlinkage void sys_uip_input_k()
{
  uip_input();
  return;
}
/*****************************************************************************/
asmlinkage void sys_uip_arp_out_k()
{
  uip_arp_out();
  return;
}
/*****************************************************************************/
asmlinkage void sys_tapdev_send_k()
{
  memcpy(eth_data, uip_buf, uip_len);
  spark_rhine_transmit(eth_data, uip_len);
  return;  
}
/*****************************************************************************/
asmlinkage void sys_uip_arp_arpin_k()
{
  int i;
  uip_arp_arpin();
  return;
}
/*****************************************************************************/
asmlinkage void sys_timerReset_k(struct spark_timer *ptimer)
{
  ptimer->start = gethrtime();
  ptimer->msec = 0;
  return;
}
/*****************************************************************************/
asmlinkage void sys_uip_periodic_k(unsigned long conn_index)
{
/*
  uip_periodic(conn_index);
  return;
*/
  uip_conn = &uip_conns[conn_index];
  spark_uip_flags = UIP_TIMER; 
  uip_process();
}
/*****************************************************************************/
asmlinkage void sys_uip_udp_periodic_k(unsigned long conn_index)
{
//#if UIP_UDP
//  uip_udp_periodic(conn_index);
//#endif
#if UIP_UDP
  uip_udp_conn = &uip_udp_conns[conn_index]; 
  spark_uip_flags = UIP_UDP_TIMER; 
  uip_process();
#endif
  return;
}
/*****************************************************************************/
asmlinkage void sys_uip_arp_timer_k()
{
  uip_arp_timer();
  return;
}
/*****************************************************************************/
asmlinkage long sys_uip_get_len_k()
{
  return get_uip_len();
}
/*****************************************************************************/
asmlinkage long sys_uip_dummy_k()
{
  return example3_init();
}
/*****************************************************************************/
asmlinkage long sys_uip_send_response_k()
{
  return uip_send_response();
}
/*****************************************************************************/
asmlinkage void sys_uip_listen_k(unsigned long port, unsigned long gos_id)
{
  uip_listen(htons(port), gos_id);
  return;
}
/*****************************************************************************/
asmlinkage void sys_uip_connect_k(unsigned long ip_address, unsigned long port, unsigned long gos_id, unsigned long sock_id)
{
  uip_connect(&ip_address, htons(port), gos_id, sock_id);
  return;
}
/*****************************************************************************/
asmlinkage long sys_uip_tcp_send_k(unsigned long sock_id, unsigned long gos_id, unsigned long pbuffer, unsigned long size)
{
  return uip_tcp_send(sock_id, gos_id, pbuffer, size);
}
/*****************************************************************************/
asmlinkage long sys_perf_measurement_k(unsigned long parameter, unsigned long preading)
{
  return spark_perf_measurement(parameter, preading);
}
/*****************************************************************************/
//struct task_switch_time task_switch_performace;

asmlinkage void sys_task_switch_time_k(unsigned long ptime)
{
#if 0
  *(unsigned long *)(ptime) = 0x90807060;
#endif
  return;
}
/*****************************************************************************/
asmlinkage long sys_rhine_poll_k()
{
        //spark_print("sys_rhine_poll_k\n");
	return spark_rhine_poll();
}
/*****************************************************************************/
