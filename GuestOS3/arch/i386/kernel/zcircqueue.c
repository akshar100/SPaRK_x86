/*************************************
*
*   Developed and Written at IIT Bombay
*   This is not a real circular queue. This implementation just monitors
*   an already allocated queue. The actual element insertions and removals 
*   are done where the queue is in scope. This implemention just keeps track
*   od read-write indices and monitors the CQ.
**************************************/
#include <arch/circqueue.h>
#include <sys/types.h>
#include "dietstdio.h"
#include <unistd.h>

unsigned int active_index = 0;

void init_cq_monitor(struct circ_queue_monitor * mycq)
{
    mycq->max_size = APP_QUEUE_SIZE;
    mycq->rear = 0;
    mycq->front = 0;
    mycq->count = 0;
    mycq->cqbuff = &(packet_cq[active_index]);
    memset((void *)packet_cq[active_index], 0, APP_DATA_SIZE * APP_QUEUE_SIZE);
    active_index += APP_DATA_SIZE * APP_QUEUE_SIZE;
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
    int oldrear = 0;
    t = (p->rear+1)%(p->max_size);
    if(t == p->front)
    {
        //rtl_printf("\nQueue Overflow\n");
        return CQ_ELEMENT_INSERT_ERROR;
    }
    else
    {
        oldrear= p->rear;
        memcpy(&(p->cqbuff[p->rear*APP_DATA_SIZE]), addr, size);
        p->rear=t;
        p->count++;
        return (oldrear);
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
        memcpy(addr, &(p->cqbuff[p->front*APP_DATA_SIZE]), APP_DATA_SIZE);
        memset((void *)(&(p->cqbuff[p->front*APP_DATA_SIZE])), 0, APP_DATA_SIZE);
        p->front=(p->front + 1)%(p->max_size);
        p->count--;
        return (oldfront);
    }
}



