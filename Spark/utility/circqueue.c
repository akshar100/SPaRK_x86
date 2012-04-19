/*************************************
*
*   Developed and Written at IIT Bombay
*   This is not a real circular queue. This implementation just monitors
*   an already allocated queue. The actual element insertions and removals 
*   are done where the queue is in scope. This implemention just keeps track
*   od read-write indices and monitors the CQ.
**************************************/
#include <rtl_printf.h>
#include <arch/circqueue.h>


#include <sys/types.h>
#include "dietstdio.h"
#include <unistd.h>

#define  APP_DATA_SIZE 200
#define  PACKET_SIZE APP_DATA_SIZE 
unsigned char cqbuff[PACKET_SIZE * 16];

void init_cq_monitor(struct circ_queue_monitor * mycq, unsigned int size)
{
    mycq->max_size = size;
    mycq->rear = 0;
    mycq->front = 0;
    mycq->count = 0;
    memset((void *)cqbuff, 0, PACKET_SIZE * 16);
    return;
}

int isempty(struct circ_queue_monitor *p)
{
    if(p->front ==p->rear)
        return 1;
    else
        return 0;
}

int insertq(struct circ_queue_monitor *p, unsigned char * addr, unsigned int size)
{
    int t;
    int i;
    t = (p->rear+1)%(p->max_size);
    if(t == p->front)
    {
        //rtl_printf("\nQueue Overflow\n");
        return CQ_ELEMENT_INSERT_ERROR;
    }
    else
    {
        memcpy(&(cqbuff[p->rear*PACKET_SIZE]), addr, size);
        p->rear=t;
        p->count++;
        return (p->rear);
    }
}

int removeq(struct circ_queue_monitor *p, unsigned char * addr)
{
    int oldfront = 0;
    unsigned char * temp;
    int i;
    if(isempty(p))
    {
        //rtl_printf("\nQueue Underflow");
        return CQ_ELEMENT_REMOVE_ERROR;
    }
    else
    {
        oldfront = p->front;
        memcpy(addr, &(cqbuff[p->front*PACKET_SIZE]), PACKET_SIZE);
        memset((void *)(&(cqbuff[p->front*PACKET_SIZE])), 0, PACKET_SIZE);
        p->front=(p->front + 1)%(p->max_size);
        p->count--;
        return (oldfront);
    }
}



