#define EXTERN22 extern
#include "firstFile.h"
#include <stdio.h>
#include <stdlib.h>
#include "Tools.h"
#include "ulibTypes.h"

int WarningBatch(char *buff);

void checkall(void);

INT8_64 TotalUsedMaximun;

static long total;

//#define DEBUG77
#ifdef DEBUG77

#include <pthread.h>

pthread_mutex_t mutex;

#define END_OF_MEMORY 900000
static void  *given[END_OF_MEMORY];
static unsigned long  givenLength[END_OF_MEMORY];
static int tagm[END_OF_MEMORY];
static int maxgiven;

void *cMalloc(unsigned long r,int tag)
{
    unsigned char *end;
	void *give;
	char buff[256];
	int k;
	static int one=0;
	
	if(one == 0){
	    pthread_mutex_init(&mutex,NULL);
	    one=1;
	}
	
	pthread_mutex_lock(&mutex);
	
	for(k=0;k<maxgiven;++k){
	    if(!given[k])goto empty;
	}
	
	if(++maxgiven >= END_OF_MEMORY){
	       sprintf(buff,"cMalloc Debug  out of table\n");
	       fprintf(stderr,"%s\n",buff);
	       exit(1);
	}
	k=maxgiven-1;
empty:
	if(!(give=(void *)calloc(r+4L,1))){
	       sprintf(buff,"cMalloc Requested (%ld) out of Memory total Used (%ld) \n",r,total);
	       fprintf(stderr,"%s\n",buff);
	       exit(1);
	}
	
	total += r+4L;
	given[k]=give;
	tagm[k]=tag;
    givenLength[k]=r+4L;
    end=(unsigned char *)give;
    end[r]=0xff;
    end[r+1]=0xff;
    end[r+2]=0xff;
    end[r+3]=0xff;
    
    //fprintf(stderr,"malloc tag %d\n",tagm[k]);


	pthread_mutex_unlock(&mutex);

	return give;
}
void *cRealloc(char *p,unsigned long r,int tag)
{
    unsigned char *end;
	void *give;
	char buff[256];
	int k;
	
	pthread_mutex_lock(&mutex);
	
	for(k=0;k<maxgiven;++k){
	    if(given[k] == p)goto found;
	}
	sprintf(buff,"Error Tried to realloc %ld\n",(long)p);
	fprintf(stderr,"%s\n",buff);
	pthread_mutex_unlock(&mutex);
	return 0;
found:

	if(!(give=(void *)realloc(p,r+4L))){
		sprintf(buff,"realloc out of Memory\n");
		fprintf(stderr,"%s\n",buff);
	       	exit(1);
	}
	given[k]=give;
	tagm[k]=tag;
    givenLength[k]=r+4L;
    end=(unsigned char *)give;
    end[r]=0xff;
    end[r+1]=0xff;
    end[r+2]=0xff;
    end[r+3]=0xff;

	pthread_mutex_unlock(&mutex);
	return give;
	
}
int cFree(char *p)
{
	char buff[256];
    unsigned long r;
    unsigned char *end;
	int k;
	
	pthread_mutex_lock(&mutex);
	for(k=0;k<maxgiven;++k){
	    if(given[k] == p)goto found;
	}
	/* sprintf(buff,"Error Tried to free %lu rank %d\n",(unsigned long)p,xg.rank); */
	sprintf(buff,"Error Tried to free %lu\n",(unsigned long)p);
	fprintf(stderr,"%s\n",buff);
	pthread_mutex_unlock(&mutex);
	return 1;
found:
    //fprintf(stderr,"free tag %d\n",tagm[k]);
    r=givenLength[k]-4;
    end=(unsigned char *)p;
    if(end[r] != 0xff || end[r+1] != 0xff || end[r+2] != 0xff || end[r+3] != 0xff){
        fprintf(stderr,"free error data corrupt tab %d\n",tagm[k]);
    }
	free(p);
	given[k]=NULL;
	if(k == maxgiven-1){
	    --maxgiven;
	    if(maxgiven < 0 )maxgiven=0;
	}	
	pthread_mutex_unlock(&mutex);
	return 0;
}
void checkall(void)
{
	int k,count;
	char buff[256];
	
	count=0;
	for(k=0;k<maxgiven;++k){
	    if(given[k]){
		    sprintf(buff,"checkall k %d %ld tag %d\n",k,(long)given[k],tagm[k]);
		    fprintf(stderr,"%s\n",buff);
	        ++count;
	    }
	}
	if(count){
	    sprintf(buff,"checkall not free %d\n",count);
	    fprintf(stderr,"%s\n",buff);
	}else{
	    maxgiven=0;
	   fprintf(stderr,"checkall all Ok\n");
	}
}
#else
void *cMalloc(unsigned long length,int tag)
{
	char buff[256];
	char *ret;
	
	tag=tag;
	
	ret=(char *)calloc(length+8L,1);
	if(ret == NULL){
	    fprintf(stderr,"cMalloc Out of Memory Requested (%ld) Total Used (%ld) tag (%d)\n",length,total,tag);
	    sprintf(buff,"cMalloc Out of Memory Requested (%ld) Total Used (%ld) tag (%d)\n",length,total,tag);
	    WarningBatch(buff);
	    return (char *)NULL;
	}
	total += length+8L;
	return ret;
}
void *cRealloc(char *p,unsigned long r,int tag)
{
	char *pn;

	tag=tag;
	
	if(!(pn=(char *)realloc(p,r))){
	    return (char *)NULL;
	}

	return pn;	
}
int cFree(char *ptr)
{
	if(ptr){
	    free(ptr);
	    return 0;
	}
	return 1;
}
void checkall(void)
{
	return;
}
#endif
