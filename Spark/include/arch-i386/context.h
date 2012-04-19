#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#define NUM_CONTEXTS 10

struct context_info {
  char *base;
  char *end;
  unsigned long cr3;
};

int map_context(int num,char *ptr,int size);
int get_contextid(void *address);
int get_idle_contextid(void);
void init_context(void);

#endif // _CONTEXT_H_
