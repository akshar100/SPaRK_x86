#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_


extern void serializer(void *func,...); 

extern struct ser_info_struct{
       unsigned long params[10];
       void *function;
       } ser_info[100];

extern volatile int serial_index_read;
extern volatile int serial_index_write;


#define serial_call(index)   {              \
       asm volatile("movl %0,%%eax                  \n"\
		    "movl $0xa,%%ecx                \n"\
		    "otroparam%=:                   \n"\
		    "    pushl (%%eax)              \n"\
		    "    subl  $0x4,%%eax           \n"\
		    "decl  %%ecx                    \n"\
		    "jnz  otroparam%=               \n"\
		    "   call *%1                    \n"\
		    "addl $40,%%esp                 \n"\
		    ::"r" (&ser_info[index].params[9]), \
		      "r" (ser_info[index].function)   \
		    : "eax","ecx","esp","cc");   \
        };  



#endif // _SERIALIZER_H_
