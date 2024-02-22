#ifndef __MTHREAD__

#define __MTHREAD__

#include <stdio.h>
#include <pthread.h>


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
						
	int launchThread(void *data,int (*sageThread)(void *data));
	
	int zerol(char *p,unsigned long n);
	
	void mprint(const char *fmt, ...);

	int Sleep2(int ms);
	
	extern int mprintFlag;

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
