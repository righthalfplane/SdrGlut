#ifndef __BSTACKH__
#define __BSTACKH__

#include <stdio.h>
#include <mutex>

#define NUM_DATA_BUFF2 10
#define NUM_ABUFF2     10

extern void *cMalloc(unsigned long r, int tag);
extern int zerol(unsigned char *p,unsigned long n);
extern int cFree(char *p);


class cStack{
public:
    cStack();
	~cStack();
	int popBuff(float *buffOut,int SizeOut);
	int pushBuff(int nbuffer);
	int popBuff(void);
	int pushBuffa(int nbuffer);
	int popBuffa();
	int setBuff(int sizei,int faudioi);
	
	float *buff[NUM_DATA_BUFF2];
    int buffStack[NUM_DATA_BUFF2];
    
    short int *buffa[NUM_ABUFF2];
    int buffStacka[NUM_ABUFF2];

	int size;
	int faudio;

    int bufftopa;
    int bufftop;
    
    std::mutex mutexa;    
    std::mutex mutexo;
    std::mutex mutex1;

};
#endif

