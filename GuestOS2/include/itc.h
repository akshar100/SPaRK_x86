#ifndef _ITC_H_
#define _ITC_H_

// total channels of communication
#define	MAX_CHANNEL	1

// unique ID for every channel...this needs to start from 0 and +1 for every new channel
#define	CHANNEL_T1_T2	0

// max data buffer size for every channel
#define	CHANNEL_T1_T2_MAXDATA	8

// channel structure
typedef	struct	CHANNEL
{
	int	front;
	int	rear;
	int	max_data;
	char	*data;
}channel;


extern	int	init_itc(void);

#endif
