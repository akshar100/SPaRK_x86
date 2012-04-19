#include <itc.h>
#include <arch/linkage.h>

#define TWOCHANNELS

// objects for every channel
#ifdef TWOCHANNELS
channel	Channels[MAX_CHANNEL + 1];
#else
channel	Channels[MAX_CHANNEL];
#endif

unsigned char * channelptr;

// static array for every channel...need to make an entry for every new channel
char	data_channel_t1_t2[CHANNEL_T1_T2_MAXDATA + 2];
char	data_channel_t2_t1[CHANNEL_T1_T2_MAXDATA + 2];

/****************************************************************/
int	init_itc(void)
{
#ifdef TWOCHANNELS
	Channels[CHANNEL_T1_T2].front = 0;
        Channels[CHANNEL_T1_T2].rear = 0;
	Channels[CHANNEL_T1_T2].max_data = CHANNEL_T1_T2_MAXDATA;
	Channels[CHANNEL_T1_T2].data = data_channel_t1_t2;

	Channels[CHANNEL_T2_T1].front = 0;
        Channels[CHANNEL_T2_T1].rear = 0;
	Channels[CHANNEL_T2_T1].max_data = CHANNEL_T2_T1_MAXDATA;
	Channels[CHANNEL_T2_T1].data = data_channel_t2_t1;

#else
	Channels[0].front = Channels[0].rear = 0;
	Channels[0].max_data = CHANNEL_T1_T2_MAXDATA;
	Channels[0].data = data_channel_t1_t2;
#endif
	return 0;

}


/****************************************************************/
// System Call to send data using ITC (Inter Task Communication)
asmlinkage long  sys_send_k(unsigned long ulChannelID, char *msg, unsigned long ulDataSize)
{
	int     iCount = 0, iBuffTail = 0;
	char    *dest = 0;
	
	if (ulChannelID > MAX_CHANNEL || ulDataSize > Channels[ulChannelID].max_data)
	{
		return 1;
	}

	if (Channels[ulChannelID].front + ulDataSize > Channels[ulChannelID].max_data)
	{
		dest = &Channels[ulChannelID].data[Channels[ulChannelID].front];
		iBuffTail = Channels[ulChannelID].max_data - Channels[ulChannelID].front;

		for(iCount = 0; iCount < iBuffTail ; iCount++)
		{
			*dest++ = *msg++;
		}

		ulDataSize = ulDataSize - iBuffTail;
		Channels[ulChannelID].front = 0;
	}


	dest = &Channels[ulChannelID].data[Channels[ulChannelID].front];
	for(iCount = 0; iCount < ulDataSize ; iCount++)
	{
		*dest++ = *msg++;
	}
	Channels[ulChannelID].front = (Channels[ulChannelID].front + ulDataSize) % Channels[ulChannelID].max_data; 
	return 0;
}
/*****************************************************************************/
// System Call to recv data using ITC (Inter Task Communication)
asmlinkage long  sys_recv_k(unsigned long ulChannelID, char *msg, unsigned long ulDataSize)
{
#if 1
	int	iCount = 0, iBuffTail = 0;
	char	*src = 0;

	if ((ulChannelID > MAX_CHANNEL || ulDataSize > Channels[ulChannelID].max_data)
           ||(Channels[ulChannelID].front == Channels[ulChannelID].rear))
		return 1;


	if (Channels[ulChannelID].rear + ulDataSize > Channels[ulChannelID].max_data)
	{
		src = &Channels[ulChannelID].data[Channels[ulChannelID].rear];
		iBuffTail = Channels[ulChannelID].max_data - Channels[ulChannelID].rear;

		for(iCount = 0; iCount < iBuffTail ; iCount++)
		{
			*msg++ = *src++;
		}

		ulDataSize = ulDataSize - iBuffTail;	
		Channels[ulChannelID].rear = 0;
	}

	src = &Channels[ulChannelID].data[Channels[ulChannelID].rear];
	for(iCount = 0; iCount < ulDataSize ; iCount++)
	{
		*msg++ = *src++;
	}
	Channels[ulChannelID].rear = (Channels[ulChannelID].rear + ulDataSize) % Channels[ulChannelID].max_data;
#endif
	return ulDataSize;
}
/*****************************************************************************/
