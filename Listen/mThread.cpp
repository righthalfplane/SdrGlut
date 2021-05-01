#define EXTERN22 extern
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

static void WarningBatch(char *message)
{
	int n;
	
	while(*message){
		n = *message++;
		if(n == '\r' || n == '\n'){
			printf("\n");
			fflush(stdout);
		}else{
			putc(n,stdout);
		}
	}
}


#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif
extern int WriteToGLUIWindow(char *message);
void mprint(const char *fmt, ...)
{
	char buff[4096];
	va_list arg;
	int ret;
	
    va_start(arg, fmt);
    ret = vsnprintf((char *)buff, sizeof(buff)-1, fmt, arg);
    va_end(arg);
	
	/* if(!WriteToGLUIWindow(buff))return; */
	
	WarningBatch((char *)buff);

}
