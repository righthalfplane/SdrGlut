#ifndef __CSTACKH__
#define __CSTACKH__

#include "firstFile.h"
#include <stdio.h>
#include "sdrReceive.h"

class cStack{
public:
    cStack(struct playData *rx);
	~cStack();
	int pushBuff(int nbuffer,struct playData *rx);
	int popBuff(struct playData *rx);
	int pushBuffa(int nbuffer,struct playData *rx);
	int popBuffa(struct playData *rx);
	struct playData *rx;
};
#endif

