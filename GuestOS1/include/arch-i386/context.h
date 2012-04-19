#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#define NUM_CONTEXTS 2

struct context_info {
  char *base;
  char *end;
  char *stack;	
  unsigned long cr3;
};

int map_context(int num,char *ptr,int size);
int get_contextid(void *address);
int get_idle_contextid(void);
void init_context(void);

extern	struct context_info	context[];

#endif // _CONTEXT_H_
