#define EXTERN22 extern
#include "firstFile.h"
#include "mThread.h"
#include <sys/timeb.h>

#include <stdarg.h>
#include <chrono>
#include <thread>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


void winout(const char *fmt, ...);

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
	
	if(mprintFlag)winout((char *)buff);

}
int GetTime(long *Seconds,long *milliseconds)
{
	struct timeb t;
	
	if(!Seconds || !milliseconds)return 1;
	

	ftime(&t);

	*Seconds=(long)t.time;
	*milliseconds=t.millitm;
	
	return 0;
}
double rtime(void)
{
	long milliseconds;
	long Seconds;
	double ret;
	
	
	GetTime(&Seconds,&milliseconds);
	
	ret=(double)Seconds+(double)milliseconds/1000.;
	
	return ret;

}
