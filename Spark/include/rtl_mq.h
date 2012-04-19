#define MAX_QUEUES          10 //Maximum number of message queues

typedef struct msg_queue
{
        unsigned long ulChannelID; //ID of the channel reserved by the guest partition
        char *start_buff; //Holds the pointer returned by the kgetpage call
	char *end_buff;
        char *current_read_ptr;
        char *current_write_ptr;
	unsigned int total_size;
	unsigned int q_filled_bytes;
}MSG_QUEUE;


