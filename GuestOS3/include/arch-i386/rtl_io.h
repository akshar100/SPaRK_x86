#ifndef __RTL_IO_H__
#define __RTL_IO_H__

static inline char rtl_inb(short port)
{
	char data;
	__asm__ __volatile__("inb %w1\n\t" : "=a" (data) : "Nd" (port));
	return data;
}

static inline char rtl_inb_p(short port)
{
	char data;
	__asm__ __volatile__("inb %w1\n\toutb %%al, $0x80\n\t" : "=a" (data) : "Nd" (port));
	return data;
}

static inline void rtl_outb(char data, short port)
{
	__asm__ __volatile__("outb %b0, %w1\n\t" : : "a" (data), "Nd" (port));
}

static inline void rtl_outb_p(char data, short port)
{
	__asm__ __volatile__("outb %b0, %w1\n\toutb %%al, $0x80\n\t" : : "a" (data), "Nd" (port));
}

#endif
