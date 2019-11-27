#define EXTERN22 extern
#include "firstFile.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/stat.h>
//#include <unistd.h>
#include "Utilities.h"
#include "mThread.h"

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif


#ifndef FALSE
#define FALSE 0
#endif

#include <time.h>

#ifndef TRUE
#define TRUE 1
#endif


extern char WarningBuff[256];

int Sleep2(int ms)
{
#ifdef _MSC_VER
	Sleep(ms);
#else
    usleep(ms*1000);
#endif
    
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
	    WarningPrint("launchThread pthread_create ret %d\n",ret);
	    goto End;
	}
	pthread_detach(thread);
	return 0;
End:
	return 1;
}
int runThreads3(long ThreadCount,void *data,long numsteps,long ZonesPerBlock,
                int (*sageThread)(mThread *Threads),
			    int (*sageThread2)(mThread *Threads,int count))
{
	pthread_t *thread;
	mThread Threads;
	mThread *myIconTH;
	
	int nt,threads;
	long tstart,tend;
	int done,ret;
	
	if(!sageThread)return 1;
	
	
	if(ZonesPerBlock <= 0)
	{
		return runThreads2(ThreadCount,data,numsteps,sageThread,sageThread2);
	}
	
	threads=(int)ThreadCount;
	
	myIconTH=NULL;
	thread=NULL;
	if(threads <= 0){
	    zerol((char *)&Threads,sizeof(Threads));
	    Threads.smin=0;
	    Threads.smax=numsteps;
		Threads.data=data;
	    ret=(*sageThread)(&Threads);
		if(sageThread2)(*sageThread2)(&Threads,1);
	    return ret;		
		
	}else{
		
	    myIconTH=(mThread *)cMalloc((threads+1)*sizeof(Threads),9020);
	    if(!myIconTH){
			WarningPrint("runThreads3 Out of memory Requested (%ld)\n",(long)(threads*sizeof(Threads)));
	        goto End;
	    }
	    zerol((char *)myIconTH,(threads+1)*sizeof(Threads));
		
		
	    thread=(pthread_t *)cMalloc(threads*sizeof(pthread_t),9021);
	    if(!thread){
	        WarningPrint("runThreads3 Out of memory Requested (%ld)\n",(long)(threads*sizeof(pthread_t)));
	        goto End;
	    }
	    zerol((char *)thread,threads*sizeof(pthread_t));
		
		for(nt=0;nt<threads;++nt){
			myIconTH[nt].data= data;
			myIconTH[nt].done = -1;
			myIconTH[nt].nthread=nt;
		}
		
				
	    tstart=0;
		do{
			tend=tstart+ZonesPerBlock;
			if(tend > numsteps){
				tend=numsteps;
			}
		
			while(1){
				done=FALSE;
				for(nt=0;nt<threads;++nt){
					if(!myIconTH[nt].done)continue;
					if(myIconTH[nt].done == 1)
					{
						if(sageThread2)(*sageThread2)(&myIconTH[nt],1);
					}
					myIconTH[nt].done = FALSE;
					myIconTH[nt].smin=tstart;
					myIconTH[nt].smax=tend;
					ret=pthread_create(&thread[nt], NULL,(void *(*)(void *))sageThread,(void *)&myIconTH[nt]);
					if(ret != 0){
						WarningPrint("runThreads3 pthread_create ret %d\n",ret);
						goto End;
					}
					pthread_detach(thread[nt]);
	                done=TRUE;
					break;
				}
	            if(done)break;
	            Sleep2(5);
			}
			tstart=tend;
		}while(tend < numsteps);
		
		
	    while(1){
	        done=TRUE;
	        for(nt=0;nt<threads;++nt){
	            if(!myIconTH[nt].done)done=FALSE;
	        }
	        if(done)break;
	        Sleep2(5);
	    }
		
		for(nt=0;nt<threads;++nt){
			if(myIconTH[nt].done == 1)
			{
				if(sageThread2)(*sageThread2)(&myIconTH[nt],1);
			}
		}
		
	}
End:
	if(myIconTH)cFree((char *)myIconTH);
	if(thread)cFree((char *)thread);
	return 0;
}

int runThreads2(long ThreadCount,void *data,long numsteps,int (*sageThread)(mThread *Threads),
			    int (*sageThread2)(mThread *Threads,int count))
{
	pthread_t *thread;
	mThread Threads;
	mThread *myIconTH;
	
	int nt,threads;
	long tstep,tstart,tend;
	int done,ret;
	
	if(!sageThread)return 1;
	
	threads=(int)ThreadCount;
	
	myIconTH=NULL;
	thread=NULL;
	if(threads <= 0){
	    zerol((char *)&Threads,sizeof(Threads));
	    Threads.smin=0;
	    Threads.smax=numsteps;
		Threads.data=data;
	    (*sageThread)(&Threads);
	    return (*sageThread2)(&Threads,1);		
		
	}else{
		
	    myIconTH=(mThread *)cMalloc((threads+1)*sizeof(Threads),9020);
	    if(!myIconTH){
			WarningPrint("runThreads2 Out of memory Requested (%ld)\n",(long)(threads*sizeof(Threads)));
	        goto End;
	    }
	    zerol((char *)myIconTH,(threads+1)*sizeof(Threads));
		
	    thread=(pthread_t *)cMalloc(threads*sizeof(pthread_t),9021);
	    if(!thread){
	        WarningPrint("runThreads2 Out of memory Requested (%ld)\n",(long)(threads*sizeof(pthread_t)));
	        goto End;
	    }
	    zerol((char *)thread,threads*sizeof(pthread_t));
		
	    tstep=numsteps/(threads+1);
	    if(tstep <= 0){
	        tstep = 1;
			threads = (int)numsteps-1;
	    }
		
	    tstart=0;
	    for(nt=0;nt<threads;++nt){
	        myIconTH[nt].data= data;
			myIconTH[nt].done = FALSE;
			myIconTH[nt].smin=tstart;
			myIconTH[nt].nthread=nt;
			tend=tstart+tstep;
			myIconTH[nt].smax=tend;
			
			ret=pthread_create(&thread[nt], NULL,(void *(*)(void *))sageThread,(void *)&myIconTH[nt]);
			if(ret != 0){
			    WarningPrint("runThreads2 pthread_create ret %d\n",ret);
			    goto End;
			}
		    pthread_detach(thread[nt]);
			tstart += tstep;
			if(tstart >= numsteps){
			    tstart=numsteps;
			}
	    }
		
        zerol((char *)&Threads,sizeof(Threads));
        
	    if(tstart < numsteps){
			Threads.smin=tstart;
			Threads.smax=numsteps;
	        Threads.nthread=threads;
	        Threads.data=data;
			(*sageThread)(&Threads);
	    }
	    
		
	    while(1){
	        done=TRUE;
	        for(nt=0;nt<threads;++nt){
	            if(!myIconTH[nt].done)done=FALSE;
	        }
	        if(done)break;
	        Sleep2(5);
	    }
		
	    if(tstart < numsteps){
			myIconTH[threads++]=Threads;
	    }
		
		(*sageThread2)(myIconTH,threads);
	}
End:
	if(myIconTH)cFree((char *)myIconTH);
	if(thread)cFree((char *)thread);
	return 0;
}
int runThreads(long ThreadCount,void *data,long numsteps,int (*sageThread)(mThread *Threads))
{
	pthread_t *thread;
	mThread Threads;
	mThread *myIconTH;
	
	int nt,threads;
	long tstep,tstart,tend;
	int done,ret;
	
	if(!sageThread)return 1;
	
	threads=(int)ThreadCount;
	
	myIconTH=NULL;
	thread=NULL;
	if(threads <= 0){
	    zerol((char *)&Threads,sizeof(Threads));
	    Threads.smin=0;
	    Threads.smax=numsteps;
		Threads.data=data;
	    return (*sageThread)(&Threads);
	}else{
		
	    myIconTH=(mThread *)cMalloc((threads)*sizeof(Threads),9020);
	    if(!myIconTH){
			WarningPrint("runThreads Out of memory Requested (%ld)\n",(long)(threads*sizeof(Threads)));
	        goto End;
	    }
	    zerol((char *)myIconTH,(threads)*sizeof(Threads));
		
		
	    thread=(pthread_t *)cMalloc(threads*sizeof(pthread_t),9021);
	    if(!thread){
	        WarningPrint("runThreads Out of memory Requested (%ld)\n",(long)(threads*sizeof(pthread_t)));
	        goto End;
	    }
	    zerol((char *)thread,threads*sizeof(pthread_t));
		
	    tstep=numsteps/(threads+1);
	    if(tstep <= 0){
	        tstep = 1;
			threads = (int)numsteps-1;
	    }
		
	    tstart=0;
	    for(nt=0;nt<threads;++nt){
	        myIconTH[nt].data= data;
			myIconTH[nt].done = FALSE;
			myIconTH[nt].smin=tstart;
			myIconTH[nt].nthread=nt;
			tend=tstart+tstep;
			myIconTH[nt].smax=tend;
			
			ret=pthread_create(&thread[nt], NULL,(void *(*)(void *))sageThread,(void *)&myIconTH[nt]);
			if(ret != 0){
			    WarningPrint("runThreads pthread_create ret %d\n",ret);
			    goto End;
			}
		    pthread_detach(thread[nt]);
			tstart += tstep;
			if(tstart >= numsteps){
			    tstart=numsteps;
			}
	    }
		
	    if(tstart < numsteps){
			zerol((char *)&Threads,sizeof(Threads));
			Threads.smin=tstart;
			Threads.smax=numsteps;
	        Threads.nthread=threads;
	        Threads.data=data;
			(*sageThread)(&Threads);
	    }
	    
		
	    while(1){
	        done=TRUE;
	        for(nt=0;nt<threads;++nt){
	            if(!myIconTH[nt].done)done=FALSE;
	        }
	        if(done)break;
	        Sleep2(5);
	    }
		
	}
End:
	if(myIconTH)cFree((char *)myIconTH);
	if(thread)cFree((char *)thread);
	return 0;
}

