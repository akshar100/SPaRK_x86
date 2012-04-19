/*
 */
#ifndef _RTL_IRQS_H_
#define _RTL_IRQS_H_

#define NR_IRQS 16

void set_trap_gate(unsigned int n, void *addr);
void set_system_gate(unsigned int n, void *addr);
void setup_arch(void);
#endif
