/*
 * Developed at ERTS IIT Bombay
 */

#ifndef __ARCH_CQ_MONITOR_
#define __ARCH_CQ_MONITOR__

#define CQ_ELEMENT_REMOVE_ERROR    999
#define CQ_ELEMENT_INSERT_ERROR    999

struct circ_queue_monitor
{
    int max_size;
    int rear;
    int front;
    int count;
};


void init_cq_monitor(struct circ_queue_monitor * mycq, unsigned int size);
int isempty(struct circ_queue_monitor *p);
int insertq(struct circ_queue_monitor *p, unsigned char *, unsigned int);
int removeq(struct circ_queue_monitor *p, unsigned char *);

#endif //__ARCH_CQ_MONITOR__

