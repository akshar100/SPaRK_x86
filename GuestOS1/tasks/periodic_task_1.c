#include <rtl_conf.h>
#include <time.h>
#include <pthread.h>
#include "systemcall_lib.c"


#include <uip/uipopt.h>
#include <uip/uip_arp.h>
//Add here any other header files required by task.

#define DRIVER_STACK_BAD     0
#define DRIVER_STACK_GOOD    1

int stack_task1[1024];

unsigned char myip[]     = {0x0a, 0x81, 0x0b, 0xa3};
unsigned char hostaddr[] = {0x0a, 0x81, 0x0b, 0xa3};
unsigned char draddr[]   = {0x0a, 0x81, 0x0b, 0xa3};
unsigned char netmask[]  = {0xFF, 0xFF, 0x80, 0x00};

struct spark_timer periodic_timer;
struct spark_timer arp_timer;

void *task1(void)
{
        int m;
        int i;
        int w;
        unsigned long eth_flag = DRIVER_STACK_BAD;
        unsigned long packet_len=0;
        unsigned long type=0;
        unsigned long netid=0;
           
        //sys_print("GuestOS1:Task1\n");
	//Give period of task in milli sec 
	//pthread_make_periodic_np (500);
	pthread_make_periodic_np (16);
        sys_print("\tCOMM PART : IP Address :");
        m = sys_rhine_get_ip(myip, 4);
        if(m == 0)
        {
            eth_flag = DRIVER_STACK_BAD;
            sys_print("\tCOMM PART : sys_rhine_get_ip FAIL\n");
        }
        else
        {
            //m = sys_rhine_isready(NULL, 0);
            m = sys_rhine_isready();
            if(m == 0)
            {
                eth_flag = DRIVER_STACK_BAD;
                sys_print("\tCOMM PART : sys_rhine_isready FAIL\n");
            }
            else
            {
                sys_print("\tCOMM PART : Ethernet Device : READY\n");
                sys_print("\tCOMM PART : Network Stack : INIT\n");
    
                sys_timerSet(&periodic_timer, 500);  // half sec
                sys_timerSet(&arp_timer, 10000);     // 10 sec 
    
                sys_uip_init();
                sys_uip_arp_init();
                sys_uip_ip_init(hostaddr, draddr, netmask);
                sys_uip_setethaddr();
                eth_flag = DRIVER_STACK_GOOD;
                netid = sys_uip_dummy();
            }
        }
        //eth_flag = DRIVER_STACK_BAD;
        if(eth_flag == DRIVER_STACK_GOOD)
        {
            while(1)
            {
            	//Make task wait till next period
                pthread_wait_np ();
                //sys_print("GuestOS1:Task1\n");

            	//Add additional operations to be performed by task in its one execution here
                sys_rhine_poll();
                packet_len = sys_tapdev_read(&type);
            	//sys_print("\n\n\n\nGuest 1: sys_tapdev_read complete\n");
            	//sys_printLong(packet_len);
            	//sys_printLong(type);
                if(packet_len > 0) 
                {
                    if(type == HTONS(UIP_ETHTYPE_IP))
                    {
            	        //sys_print("UIP_ETHTYPE_IP \n");
                        sys_uip_arp_ipin();
                        sys_uip_input();
                        /*If the above function invocation resulted in data that
                          should be sent out on the network, the global variable
                          packet_len is set to a value > 0. */
                        if(sys_uip_get_len() > 0) 
                        {
                            sys_uip_arp_out();
                            sys_tapdev_send();
                        }
                    } 
                    else if(type == HTONS(UIP_ETHTYPE_ARP)) 
                    {
            	    //    sys_print("UIP_ETHTYPE_ARP \n");
                        sys_uip_arp_arpin();
                        /*If the above function invocation resulted in data that
                          should be sent out on the network, the global variable
                          packet_len is set to a value > 0. */
                        if(sys_uip_get_len() > 0) 
                        {
            	            //sys_print("********************************\n");
            	            //sys_print("********************************\n");
                            sys_tapdev_send();
                        }
                   }
                }
                if(sys_timerExpired(&periodic_timer)) 
                {
                    sys_timerReset(&periodic_timer);
                    for(w = 0; w < UIP_CONNS; w++) 
                    {
                        sys_uip_periodic(w);
            	        //sys_print("Guest 1: sys_uip_periodic done \n");
                        /*If the above function invocation resulted in data that
                          should be sent out on the network, the global variable
                          packet_len is set to a value > 0. */
                        if(sys_uip_get_len() > 0) 
                        {
            	            //sys_print("Guest 1: Inside IF\n");
                            sys_uip_arp_out();
                            sys_tapdev_send();
            	            //sys_print("Guest 1: sys_tapdev_send done\n");
                        }
            	        //sys_print("Guest 1: Outside IF\n");
                    }
#if UIP_UDP
                    for(i = 0; i < UIP_UDP_CONNS; i++) 
                    {
            	        //sys_print("Guest 1: Checking UDP cons \n");
                        sys_uip_udp_periodic(i);
                        /*If the above function invocation resulted in data that
                          should be sent out on the network, the global variable
                          packet_len is set to a value > 0. */
                        if(sys_uip_get_len() > 0) 
                        {
                            sys_uip_arp_out();
                            sys_tapdev_send();
                        }
                    }
#endif /* UIP_UDP */
               
                    /* Call the ARP timer function every 10 seconds. */
                    if(sys_timerExpired(&arp_timer)) 
                    {
//            	        sys_print("**********Guest 1: arp_timer expired\n");
                        sys_timerReset(&arp_timer);
            	        //sys_print("Guest 1: arp_timer reset\n");
                        sys_uip_arp_timer();
                    }
                }
            }// end of while
        }
        else
        {
            while(1)
            {
            	//Make task wait till next period
                pthread_wait_np ();
            	sys_print("Guest 1: Task1\n");
            	//Add additional operations to be performed by task in its one execution here
                //sys_rhine_transmit(icmp_data, sizeof(icmp_data));

            }
        }
        return 0;
}
