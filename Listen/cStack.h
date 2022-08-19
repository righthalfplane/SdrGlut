#ifndef __CSTACKH__
#define __CSTACKH__

#include "firstFile.h"
#include <stdio.h>
#include "Clisten.h"

class cStack{
public:
    cStack(class Listen *rx);
	~cStack();
	int pushBuff(int nbuffer,class Listen *rx);
	int popBuff(class Listen *rx);
	int pushBuffa(int nbuffer,class Listen *rx);
	int popBuffa(class Listen *rx);
	struct playData *rx;
};
#endif

