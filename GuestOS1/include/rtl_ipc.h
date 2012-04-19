/*
 *  rtl_ipc.h -- intertask communication primitives for Real-Time Linux
 *
 *  Copyright (C) 1997 Jerry Epplin.  All rights reserved.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  History:
 *   17-Jul-97 jhe  Original.
 *   28-Jul-97 jhe  V0.2 Timeouts on semaphores.  Message queues.
 *   15-Aug-97 jhe  V0.3 rt_ipc fifos.  Modified semantics of timeouts.
 */

#include <rtl_sched.h>

typedef long long RTIME;

typedef enum { RT_SEM_BINARY, RT_SEM_COUNTING } RT_SEMTYPE;

typedef enum { RT_MQ_NORMAL, RT_MQ_URGENT } RT_MQ_PRIO;

typedef enum { RT_MQ_FULL, RT_MQ_EMPTY, RT_MQ_NEITHER } RT_MQ_STATUS;

/* use this macro when calling rtl functions requiring an RT_TASK */
#define MAKE_RT_TASK(rt_task_ipc)	(&((rt_task_ipc)->rtl_task))

/* use this macro for a timeout relative to the current time.  rt_ipc */
/* timeouts are stated in absolute times; i.e., the functions time out */
/* at the time specified in the timeout parameter. */
#define RELATIVE_TIME(TIME_FROM_NOW)	(rt_get_time() + (TIME_FROM_NOW))

#define RT_WAIT_FOREVER	-1	/* use for no timeout on funcs with timeout */
#define RT_NO_WAIT	0	/* use for return immediately if can't */
  				/* perform operation */

#define RT_SEM_MAGIC		0x8529afdb
#define RT_MQ_MAGIC		0x6a39746c
#define RT_TASK_IPC_MAGIC	0x9573ed98

#define IPC_RTF_NO		64	/* should be same as in rt_fifo_new.c */

#define IPC_DATA_INDEX          0

typedef struct tsk_list
{
  struct tsk_list *prev;
  struct tsk_list *next;
  pthread_t *task;
} RT_TASK_ENTRY;		/* wait list entry structure */



typedef struct
{
  int magic;			/* for verifying correctness of struct */
  int val;			/* current semaphore value */
  RT_SEMTYPE type;		/* type of semaphore */
  RT_TASK_ENTRY *wait_list;	/* list of tasks waiting for this mq */
} rt_sem_t;			/* semaphore structure */

typedef struct
{
  int magic;			/* for verifying correctness of struct */
  char *q;			/* message queue data */
  int msg_size;			/* size of each message queue entry */
  int max_msgs;			/* number of messages allowed at a time */
  char *f, *r;			/* current front and rear of queue */
  RT_MQ_STATUS status;		/* indicates full/empty/neither status of q */
  RT_TASK_ENTRY *wait_list;	/* list of tasks waiting for this mq */
} rt_mq_t;			/* message queue structure */


typedef struct
{
  pthread_t rtl_task;	/* normal, rtlinux part of structure -- MUST BE FIRST */
  int magic;
  RT_TASK_ENTRY rte;	/* used when this task is blocked */
  rt_sem_t *sem_at;	/* sem at which task is blocked (or NULL if none) */
  rt_mq_t *mq_at;	/* mq at which task is blocked (or NULL if none) */
  int timed_out;	/* set if rt_sem_wait() timed out */
} RT_TASK_IPC;		/* task structure for tasks using ipc primitives */


/* available functions */
extern int rt_sem_init(rt_sem_t *sem, RT_SEMTYPE type, int init_val);
extern int rt_sem_destroy(rt_sem_t *sem);
extern int rt_sem_post(rt_sem_t *sem);
extern int rt_sem_wait(rt_sem_t *sem, RTIME timeout);
extern int rt_sem_trywait(rt_sem_t *sem);

extern int rt_task_delay(RTIME duration);

extern int rt_task_ipc_init(pthread_t *task, void (*fn)(int data),
  int data, int stack_size, int priority);
extern int rt_task_ipc_delete(pthread_t *task);

extern int rt_mq_init(rt_mq_t *mq, int max_msgs, int msg_size);
extern int rt_mq_destroy(rt_mq_t *mq);
extern int rt_mq_send(rt_mq_t *mq, char *msg, RT_MQ_PRIO prio, RTIME wait);
extern int rt_mq_receive(rt_mq_t *mq, char *msg, RTIME wait);

extern int rtf_ipc_create(unsigned int fifo, int size, int rtl_to_linux);
extern int rtf_ipc_destroy(unsigned int fifo);
extern int rtf_receive(unsigned int fifo, void *buf, int count, RTIME timeout);
extern int rtf_send(unsigned int fifo, void *buf, int count, RTIME timeout);

