
#include "cStack.h"

cStack::cStack(struct playData *rxi)
{
    rx=rxi;
    bufftopa=0;
    bufftop=0;
    
    for(int k=0;k<NUM_DATA_BUFF;++k){
        buffStack[k]=0;
        buff[k]=NULL;
    //	fprintf(stderr,"k %d buff[k] %p\n",k,buff[k]);
    }
    for(int k=0;k<NUM_ABUFF;++k){
    	buffStacka[k]=0;
    	buffa[k]=NULL;
    }
}
cStack::~cStack()
{
	for(int k=0;k<NUM_DATA_BUFF;++k){
		if(buff[k])cFree((char *)buff[k]);
		buff[k]=NULL;
	}
	
	for(int k=0;k<NUM_ABUFF;++k){
		if(buffa[k])cFree((char *)buffa[k]);
		buffa[k]=NULL;
	}
}

int cStack::setBuff(struct playData *rx)
{


	for(int k=0;k<NUM_DATA_BUFF;++k){
	//    fprintf(stderr,"k %d buff %p\n",k,buff[k]);
		if(buff[k])cFree((char *)buff[k]);
		buff[k]=(float *)cMalloc(2*rx->size*4*8,5777);
		if(!buff[k]){
			fprintf(stderr,"5 cMalloc Errror %ld\n",(long)(2*rx->size*4));
			return 1;
		}
		zerol((char *)buff[k],2*rx->size*4);
		buffStack[k]=-1;
	}
	
	for(int k=0;k<NUM_ABUFF;++k){
		if(buffa[k])cFree((char *)buffa[k]);
		buffa[k]=(short int *)cMalloc((size_t)(2*rx->faudio*4),5272);
		if(!buffa[k]){
			fprintf(stderr,"10 cMalloc Errror %ld\n",(long)(2*rx->faudio*4));
			return 1;
		}
		zerol((char *)buffa[k],(unsigned long)(2*rx->faudio*4));
		buffStacka[k]=-1;
	}


	return 0;
}

int cStack::pushBuffa(int nbuffer,struct playData *rx)
{

	rx->mutexa.lock();
//	mprint("pushBuffa in %d\n",bufftopa);
	
    if(bufftopa >= NUM_ABUFF){
        bufftopa=NUM_ABUFF;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_ABUFF;++k){
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
	rx->mutexa.unlock();

	return 0;
}

int cStack::popBuffa(struct playData *rx)
{
	int ret;
	
	
	rx->mutexa.lock();
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
	rx->mutexa.unlock();

	return ret;
}

int cStack::pushBuff(int nbuffer,struct playData *rx)
{

	rx->mutex1.lock();
	
    if(bufftop >= NUM_DATA_BUFF){
        bufftop=NUM_DATA_BUFF;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_DATA_BUFF;++k){
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
    
	rx->mutex1.unlock();

	
	return 0;
}
int cStack::popBuff(struct playData *rx)
{
	int ret;
	
	
	rx->mutex1.lock();

	
	ret=-1;
	
 	if(bufftop < 1)goto Out;
 	
 	//fprintf(stderr,"popBuff bufftop %d ",bufftop );
 	
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
  	//fprintf(stderr,"ks %d \n",ks);
       
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
	rx->mutex1.unlock();

	return ret;
}
