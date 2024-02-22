#define EXTERN22 extern
#include "firstFile.h"
#include "mThread.h"
#include <stdarg.h>
#include <chrono>
#include <thread>


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

static void WarningBatch(char *message);

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

/*
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
	//pthread_detach(thread);
	return 0;
End:
	return 1;
}
*/

static void WarningBatch(char *message)
{
	int n;
	
	while(*message){
		n = *message++;
		if(n == '\r' || n == '\n'){
			fprintf(stderr,"\n");
			fflush(stderr);
		}else{
			putc(n,stderr);
		}
	}
}

int mprintFlag=1;

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif
extern int WriteToGLUIWindow(char *message);
void mprint(const char *fmt, ...)
{
	char buff[4096];
	va_list arg;
	
    va_start(arg, fmt);
    vsnprintf((char *)buff, sizeof(buff)-1, fmt, arg);
    va_end(arg);
	
	/* if(!WriteToGLUIWindow(buff))return; */
	
	if(mprintFlag)WarningBatch((char *)buff);

}
