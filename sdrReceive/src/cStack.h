#ifndef __CSTACKH__
#define __CSTACKH__

#include "firstFile.h"
#include <stdio.h>
#include "sdrReceive.h"
#include "cReceive.h"

class cStack{
public:
    cStack(struct playData *rx);
	~cStack();
	int pushBuff(int nbuffer,struct playData *rx);
	int popBuff(struct playData *rx);
	int pushBuffa(int nbuffer,struct playData *rx);
	int popBuffa(struct playData *rx);
	int setBuff(struct playData *rx);
	struct playData *rx;
	
	float *buff[NUM_DATA_BUFF];
    int buffStack[NUM_DATA_BUFF];
    
    short int *buffa[NUM_ABUFF];
    int buffStacka[NUM_ABUFF];

    int bufftopa;
    int bufftop;

};
#endif

