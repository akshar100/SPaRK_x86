#ifndef _I386_PTRACE_H
#define _I386_PTRACE_H

/*
#define FS 0
#define GS 1
#define EBX 2
#define ECX 3
#define EDX 4
#define ESI 5
#define EDI 6
#define EBP 7
#define EAX 8
#define DS 9
#define ES 10
#define ORIG_EAX 11
#define EIP 12
#define CS  13
#define EFL 14
#define UESP 15
#define SS   16
#define FRAME_SIZE 17
*/

/* this struct defines the way the registers are stored on the 
   stack during a system call. */

struct pt_regs {
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	long xds;
	long xes;
	long xgs;
	long xfs;
	long orig_eax;
	long eip;
	long xcs;
	long eflags;
	long esp;
	long xss;

};

/* Arbitrarily choose the same ptrace numbers as used by the Sparc code. */
#define PTRACE_GETREGS            12
#define PTRACE_SETREGS            13
#define PTRACE_GETFPREGS          14
#define PTRACE_SETFPREGS          15
#define PTRACE_GETFPXREGS         18
#define PTRACE_SETFPXREGS         19

#define PTRACE_SETOPTIONS         21

/* options set using PTRACE_SETOPTIONS */
#define PTRACE_O_TRACESYSGOOD     0x00000001

#ifdef __KERNEL__
#define user_mode(regs) ((VM_MASK & (regs)->eflags) || (3 & (regs)->xcs))
#define instruction_pointer(regs) ((regs)->eip)
extern void show_regs(struct pt_regs *);
#endif

#endif
