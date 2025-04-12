
#include "bStack.h"

void winout(const char *fmt, ...);

cStack::cStack()
{
    bufftopa=0;
    bufftop=0;
    
    for(int k=0;k<NUM_DATA_BUFF2;++k){
        buffStack[k]=0;
        buff[k]=NULL;
    //	winout("k %d buff[k] %p\n",k,buff[k]);
    }
    for(int k=0;k<NUM_ABUFF2;++k){
    	buffStacka[k]=0;
    	buffa[k]=NULL;
    }
}
cStack::~cStack()
{
	for(int k=0;k<NUM_DATA_BUFF2;++k){
		if(buff[k])cFree((char *)buff[k]);
		buff[k]=NULL;
	}
	
	for(int k=0;k<NUM_ABUFF2;++k){
		if(buffa[k])cFree((char *)buffa[k]);
		buffa[k]=NULL;
	}
	
	//winout("Exit cStack %p\n",this);
}

int cStack::setBuff(int sizei,int faudioi)
{
	size=sizei;
	faudio=faudioi;

	//fprintf(stderr,"setBuff size %d faudio %d\n",sizei,faudio);

	for(int k=0;k<NUM_DATA_BUFF2;++k){
	//    winout("k %d buff %p\n",k,buff[k]);
		if(buff[k])cFree((char *)buff[k]);
		buff[k]=(float *)cMalloc(2*size*4*8,5777);
		if(!buff[k]){
			winout("5 cMalloc Errror %ld\n",(long)(2*size*4));
			return 1;
		}
		zerol((unsigned char *)buff[k],2*size*4);
		buffStack[k]=-1;
	}
	
	for(int k=0;k<NUM_ABUFF2;++k){
		if(buffa[k])cFree((char *)buffa[k]);
		buffa[k]=(short int *)cMalloc((size_t)(2*faudio*4),5272);
		if(!buffa[k]){
			winout("10 cMalloc Errror %ld\n",(long)(2*faudio*4));
			return 1;
		}
		zerol((unsigned char *)buffa[k],(unsigned long)(2*faudio*4));
		buffStacka[k]=-1;
	}


	return 0;
}

int cStack::pushBuffa(int nbuffer)
{

	mutexa.lock();
//	mprint("pushBuffa in %d\n",bufftopa);
	
    if(bufftopa >= NUM_ABUFF2){
        bufftopa=NUM_ABUFF2;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_ABUFF2;++k){
             if(buffStacka[k] < small2){
             	small2=buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	buffStacka[ks]=nbuffer;
        }
   }else{
    	buffStacka[bufftopa++]=nbuffer;
    }
    
//    mprint("pushBuffa nbuffer %d top %d\n",nbuffer,rx->bufftopa);
    
//	mprint("pushBuffa out\n");
	mutexa.unlock();

	return 0;
}

int cStack::popBuffa()
{
	int ret;
	
	
	mutexa.lock();
//	mprint("popBuffa in %d\n",bufftopa);
	
	ret=-1;
	
 	if(bufftopa < 1)goto Out;
 	
 	if(bufftopa == 1){
 		ret=buffStacka[0];
 		bufftopa=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<bufftopa;++k){
             if(buffStacka[k] < small2){
             	small2=buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	ret=buffStacka[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<bufftopa;++k)
        	{
        		if(k == ks)continue;
        		buffStacka[kk++]=buffStacka[k];
        	}
        	bufftopa--;
        }
	
	
Out:
//    if(ret > 0)mprint("popBuffa ret %d top %d\n",ret,rx->bufftopa);
//	mprint("popBuffa out\n");
	mutexa.unlock();

	return ret;
}

int cStack::pushBuff(int nbuffer)
{

	mutex1.lock();
	
	//winout("pushBuff nbuffer %d buffStack %p\n",nbuffer,buffStack);

	
    if(bufftop >= NUM_DATA_BUFF2){
        bufftop=NUM_DATA_BUFF2;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_DATA_BUFF2;++k){
             if(buffStack[k] < small2){
             	small2=buffStack[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	buffStack[ks]=nbuffer;
        }
   }else{
    	buffStack[bufftop++]=nbuffer;
    }
    
	mutex1.unlock();

	
	return 0;
}
int cStack::popBuff()
{
	int ret;
	
	
	mutex1.lock();

	
	ret=-1;
	
	
 	if(bufftop < 1)goto Out;
 	
  	//winout("popBuff bufftop %d \n",bufftop );
	
 	if(bufftop == 1){
 		ret=buffStack[0];
 		bufftop=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<bufftop;++k){
             if(buffStack[k] < small2){
             	small2=buffStack[k];
             	ks=k;
             }
        }
  	//winout("ks %d \n",ks);
       
        if(ks >= 0){
        	ret=buffStack[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<bufftop;++k)
        	{
        		if(k == ks)continue;
        		buffStack[kk++]=buffStack[k];
        	}
        	bufftop--;
        }
	
	
Out:
	mutex1.unlock();

	return ret;
}
int cStack::popBuff(float *buffOut,int SizeOut)
{
	int ret;
	
	
	mutex1.lock();

	
	ret=-1;
	
	
 	if(bufftop < 1)goto Out;
 	
  	//winout("popBuff bufftop %d \n",bufftop );
	
 	if(bufftop == 1){
 		ret=buffStack[0];
 		bufftop=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<bufftop;++k){
             if(buffStack[k] < small2){
             	small2=buffStack[k];
             	ks=k;
             }
        }
  	//winout("ks %d \n",ks);
       
        if(ks >= 0){
        	ret=buffStack[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<bufftop;++k)
        	{
        		if(k == ks)continue;
        		buffStack[kk++]=buffStack[k];
        	}
        	bufftop--;
        }
	
Out:
	if(ret >= 0){
	
	    int witch=ret % NUM_DATA_BUFF2;
		float *buffIn=buff[witch];
		for(int n=0;n<SizeOut*2;++n){
			buffOut[n]=buffIn[n];
		}
	}
	mutex1.unlock();

	return ret;
}
