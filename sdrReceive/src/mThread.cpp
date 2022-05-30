#define EXTERN22 extern
#include "firstFile.h"
#include "mThread.h"
#include <stdarg.h>
#include <chrono>
#include <thread>
#include <stdio.h>


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

int zerol(char *p,unsigned long n)
{
	if(!p || !n)return 1;
	
	while(n-- > 0)*p++ = 0;
	
	return 0;
}


int Sleep2(int ms)

{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return 0;
}

int launchThread(void *data,int (*sageThread)(void *data))
{
    std::thread(sageThread,data).detach();
    return 0;
}
