#ifndef __MTHREAD__

#define __MTHREAD__

#include <stdio.h>
//#include <pthread.h>
						
	int zerol(char *p,unsigned long n);
	
	void mprint(const char *fmt, ...);

	int Sleep2(int ms);
	
	double rtime(void);
	
	extern int mprintFlag;

#endif
