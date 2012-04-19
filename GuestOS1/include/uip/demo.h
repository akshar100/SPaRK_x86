
/**
 * ERTS Lab IITB
 *
 */

#ifndef __DEMO_H__
#define __DEMO_H__

#define SOCKET_CALL_CHANNEL1                  0x01020201
#define SOCKET_CALL_REPLY_CHANNEL1            0xA1A2A2A1
#define SOCKET_TCP_RECV_CHANNEL1              0x02010102
#define CALL_CHANNEL1_SIZE                    512 
#define CALL_REPLY_CHANNEL1_SIZE              512 
#define TCP_RECV_CHANNEL1_SIZE                512 

#define SOCKET_CALL_CHANNEL2                  0x01020301
#define SOCKET_CALL_REPLY_CHANNEL2            0xA1A2A3A1
#define SOCKET_TCP_RECV_CHANNEL2              0x03010102
#define CALL_CHANNEL2_SIZE                    512 
#define CALL_REPLY_CHANNEL2_SIZE              512 
#define TCP_RECV_CHANNEL2_SIZE                512 

#define APPLICATION_LAYER_OFFSET              (UIP_LLH_LEN + UIP_IPTCPH_LEN)
#define DEST_PORT_OFFSET                      (UIP_LLH_LEN + UIP_IPH_LEN + 2) 

#endif /* __DEMP_H__ */

