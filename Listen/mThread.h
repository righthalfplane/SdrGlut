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

	int launchThread(void *data,int (*sageThread)(void *data));
	
	int Sleep2(int ms);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
