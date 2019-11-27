#ifndef __MTHREAD__

#define __MTHREAD__


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
	typedef struct mThreadStruct{
	void *data;
	long smin;
	long smax;
    volatile int done;
    int nthread;
	double value1;
	double value2;
	double value3;
	void *saveThreadData;
    void *save;     /* place to save windows thread handle for later release */
}mThread;

	
	int runThreads(long ThreadCount,void *data,long Count,int (*sageThread)(mThread *Threads));
	
	int runThreads2(long ThreadCount,void *data,long Count,int (*sageThread)(mThread *Threads),
					int (*sageThread2)(mThread *Threads,int count));
					
					
	int runThreads3(long ThreadCount,void *data,long DataCount,long ZonesPerBlock,
                int (*sageThread)(mThread *Threads),
				int (*sageThread2)(mThread *Threads,int count));
					
	int launchThread(void *data,int (*sageThread)(void *data));
	
	int Sleep2(int ms);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
