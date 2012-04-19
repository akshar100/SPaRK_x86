/*
 * context.c
 This code is developed in IITB
 */

#include <string.h>
#include <arch/page.h>
#include <arch/memory.h>
#include <rtl_conf.h>
#include <arch/context.h>


extern char _start_context00,_end_context00;
extern char _start_context01,_end_context01;
extern char _stack_context00;
extern char _stack_context01;

struct context_info context[NUM_CONTEXTS] = {
	{ &_start_context00, &_end_context00 , &_start_context00, 0},
	{ &_start_context01, &_end_context01 , &_start_context01, 0}
};

int get_contextid(void *address) {
	int i;

	for (i=0;i<NUM_CONTEXTS;i++) {
		if (context[i].base != context[i].end) {
			if (((unsigned long) address >= (unsigned long) context[i].base) && ((unsigned long) address <= (unsigned long) context[i].end)) {
				return i;
			}
		}
	}
	return -1; 
}
