#define EXTERN22 extern
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mThread.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


char WarningBuff[256];

int Sleep2(int ms)
{
    usleep(ms*1000);
	return 0;
}


static int zerol(char *p,long n)
{
	if(!p)return 1;
	while(n-- > 0)*p++ = 0;
	return 0;
}

int launchThread(void *data,int (*sageThread)(void *data))
{
	pthread_t thread;
	int ret;

	if(!data || !sageThread)return 1;

	zerol((char *)&thread,sizeof(pthread_t));

	ret=pthread_create(&thread, NULL,(void *(*)(void *))sageThread,(void *)data);
	if(ret != 0){
	    printf("launchThread pthread_create ret %d\n",ret);
	    goto End;
	}
	pthread_detach(thread);
	return 0;
End:
	return 1;
}
