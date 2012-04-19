/*
 *  $Id: build.c,v 1.5 1997/05/19 12:29:58 mj Exp $
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *  Copyright (C) 1997 Martin Mares
 */

/*
 * This file builds a disk-image from three different files:
 *
 * - bootsect: exactly 512 bytes of 8086 machine code, loads the rest
 * - setup: 8086 machine code, sets up system parm
 * - system: 80386 code for actual system
 *
 * It does some checking that all files are of the correct type, and
 * just writes the result to stdout, removing headers and padding to
 * the right amount. It also writes some system data to stderr.
 */

/*
 * Changes by tytso to allow root device specification
 * High loaded stuff by Hans Lermen & Werner Almesberger, Feb. 1996
 * Cross compiling fixes by Gertjan van Wingerde, July 1996
 * Rewritten by Martin Mares, April 1997
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/boot.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long u32;

#define BUFFER_SIZE 1024

byte buf[BUFFER_SIZE];
int fd;
u32 sys_size; 

void die(const char * str, ...)
{
	va_list args;
	va_start(args, str);
	vfprintf(stderr, str, args);
	fputc('\n', stderr);
	exit(1);
}

void file_open(const char *name)
{
	if ((fd = open(name, O_RDONLY, 0)) < 0)
		die("Unable to open `%s': %m", name);
}

void usage(void)
{
	die("Usage: attach [Num of Guests(at most 2)] <Spark> <Name of Guest1> <Name of Guest2>  [> image]");
}

void readandprint_rtvic(char *filename, int pos_2_insert)
{
	struct stat sb;
	unsigned int sz;
	FILE *sched_op;
	unsigned char *temp;
	unsigned int line_max =100, flag = 0;
	unsigned int *tmp_int;


	temp = (unsigned char *)malloc(0x100);

	file_open(filename);
	if (fstat (fd, &sb))
		die("Unable to stat `%s': %m", filename);
	sz = sb.st_size;
	sys_size += sz;
	fprintf (stderr, "%s is %d kB\n", filename, sz/1024);
	while (sz > 0) {
		int l, n;
		l = (sz > sizeof(buf)) ? sizeof(buf) : sz;
		if ((n=read(fd, buf, l)) != l) {
			if (n < 0)
				die("Error reading %s: %m", filename);
			else
				die("%s: Unexpected EOF", filename);
		}

#if 1
		if(pos_2_insert > l) {
			pos_2_insert -= l;
		}
		else if (flag == 0) {
			flag = 1;
			if ((sched_op = fopen("sched.output", "r")) == NULL)
				die("Unable to open `%s': %m", "sched.output");
				
			tmp_int = (unsigned int *)(buf + pos_2_insert);
			fprintf (stderr, " position to insert = %d \n",pos_2_insert);
			fprintf (stderr, " buf = %d, tmp_int = %d \n",buf, tmp_int);
			while ( getline(&temp, &line_max, sched_op) != -1 ){
				//fprintf(stderr, "%d", atoi(temp));
				*tmp_int = (unsigned int)atoi(temp);
				tmp_int++;
				pos_2_insert += 4;
				if (pos_2_insert > l)
					die("Attach: Boundary case, please check");
			}
			fprintf(stderr, "end of file");
			pos_2_insert -= l;
			//buf[pos_2_insert] = 0x100;
		}
#endif
		if (write(1, buf, l) != l)
			die("Write failed");
		sz -= l;
	}
	close(fd);
}


void readandprint(char *filename)
{
	struct stat sb;
	unsigned int sz;

	file_open(filename);
	if (fstat (fd, &sb))
		die("Unable to stat `%s': %m", filename);
	sz = sb.st_size;

	sys_size += sz;

	fprintf (stderr, "%s is %d kB\n", filename, sz/1024);
	while (sz > 0) {
		int l, n;

		l = (sz > sizeof(buf)) ? sizeof(buf) : sz;
		if ((n=read(fd, buf, l)) != l) {
			if (n < 0)
				die("Error reading %s: %m", filename);
			else
				die("%s: Unexpected EOF", filename);
		}
		if (write(1, buf, l) != l)
			die("Write failed");
		sz -= l;
	}
	close(fd);
}

void writezeros(unsigned int sz)
{
	unsigned int i, l;
	sys_size += sz;
	while (sz > 0) {
		l = (sz > sizeof(buf)) ? sizeof(buf) : sz;
		for(i=0; i< l; i++)
		{
			buf[i] = 0;
		}
		if (write(1, buf, l) != l)
			die("Write failed");
		sz -= l;
	}
}


u32 offset0 = 0x1000;
u32 offset1 = 0x30000;
u32 offset2 = 0x50000;
u32 offset3 = 0x70000;

int main(int argc, char ** argv)
{
	unsigned int i;
	struct stat head_stat;

	if(argc != 6 )
		usage();	

	sys_size = offset0;

	if (stat("head.out",&head_stat) != 0){
		die("head.o can not be stat, exiting.... \n");
		//return 0;
	}
	readandprint_rtvic(argv[2], head_stat.st_size);
	// readandprint(argv[2]);

	if (offset1 < sys_size) {
		die("%s file size is exceeding its limit, please check...", argv[3]);
	}
	else {
		writezeros(offset1 - sys_size);
	}
	readandprint(argv[3]);

	// comment the code below , if you have only 1 guest OS

	if (argc == 6)
	{
		if (offset2 < sys_size) {
			die("%s Guest OS1 size is exceeding its limit, please check...", argv[4]);
		}
		else {
			writezeros(offset2 - sys_size);
		}
		readandprint(argv[4]);

		if (offset3 < sys_size) {
			die("%s Guest OS2 size is exceeding its limit, please check...", argv[4]);
		}
		else {
			writezeros(offset3 - sys_size);
		}
		readandprint(argv[5]);

	}

	fprintf (stderr, "System is %u kB\n", sys_size/1024);

	return 0;					    /* Everything is OK */
}



