#include "audiolib.h"

#include <pthread.h>

static int zerol(unsigned char *s,unsigned long n);

static void *cMalloc(unsigned long n);

static int cFree(unsigned char *data);

static pthread_mutex_t buffermutex;

int updateAudio(struct audioInfo *audio)
{
	if(!audio)return 1;
	
	return 0;
}
int checksourceAudio(struct audioInfo *audio,ALuint n)
{
    for(int k=0;k<audio->numsource;++k)
    {
        if(audio->source[k] == n){
            fprintf(stderr,"checksourceAudio %d\n",n);
            return 1;
        }
    }
    
    return 0;
}
int freesourceAudio(struct audioInfo *audio,ALuint n)
{
    for(int k=0;k<audio->numsource;++k)
    {
        if(audio->source[k] == n){
            audio->sourceFlag[k]=0;
            //fprintf(stderr,"freesourceAudio %d\n",n);
            break;
        }
    }
    
    return 0;
}
ALuint getsourceAudio(struct audioInfo *audio)
{
    if(!audio)return NO_MORE_SPACE;
    
    for(int k=0;k<audio->numsource;++k)
    {
        if(!audio->sourceFlag[k]){
            audio->sourceFlag[k]=1;
            //fprintf(stderr,"getsourceAudio %d\n",audio->source[k]);
            return audio->source[k];
        }
    }
    
    return NO_MORE_SPACE;
}

int startAudio(struct audioInfo *audio,int numsource,int numbuff)
{
	if(!audio)return 1;
	
	zerol((unsigned char *)audio,sizeof(*audio));
	
    audio->dev = alcOpenDevice(NULL); // select the "preferred dev" 
    audio->ctx = alcCreateContext(audio->dev,NULL); 
    alcMakeContextCurrent(audio->ctx);  
	
	audio->numsource=numsource;
	
	audio->source=(ALuint *)cMalloc(numsource*sizeof(*(audio->source)));
	if(!audio->source)return 1;
	
	audio->sourceFlag=(int *)cMalloc(numsource*sizeof(*(audio->sourceFlag)));
	if(!audio->sourceFlag)return 1;
	
	zerol((unsigned char *)audio->sourceFlag,numsource*sizeof(*(audio->sourceFlag)));
	
	ALenum error;
	
	alGenSources(numsource, audio->source);
    if ((error = alGetError()) != AL_NO_ERROR){
        DisplayALError((unsigned char *)"alGenSources : ", error);
        return 1;
    }
    
    
	audio->numbuff=numbuff;
	
	audio->buff=(ALuint *)cMalloc(numbuff*sizeof(*(audio->buff)));
	if(!audio->buff)return 1;
	
	audio->buffFlag=(int *)cMalloc(numbuff*sizeof(*(audio->buffFlag)));
	if(!audio->buffFlag)return 1;
	
	zerol((unsigned char *)audio->buffFlag,numbuff*sizeof(*(audio->buffFlag)));
	
	alGenBuffers(numbuff, audio->buff);
    if ((error = alGetError()) != AL_NO_ERROR){
        DisplayALError((unsigned char *)"alGenBuffers : ", error);
        return 1;
    }
    
    pthread_mutex_init(&buffermutex,NULL);

	return 0;
}
ALuint freebuffAudio(struct audioInfo *audio,ALuint n)
{
    int ret;
    
    if(!audio)return 1;
    
    pthread_mutex_lock(&buffermutex);
    
    ret = 1;
    
	for(int k=0;k<audio->numbuff;++k)
	{
		if(audio->buff[k] == n){
		    audio->buffFlag[k]=0;
		   //fprintf(stderr,"freebuffAudio k %d audio->buff[k] %d\n",k,audio->buff[k]);
            ret = 0;
			break;
		}
	}
    
    pthread_mutex_unlock(&buffermutex);

	return ret;
}
ALuint getbuffAudio(struct audioInfo *audio)
{
    int ret;
    
	if(!audio)return NO_MORE_SPACE;
	
    pthread_mutex_lock(&buffermutex);

    ret = NO_MORE_SPACE;

	for(int k=0;k<audio->numbuff;++k)
	{
		if(!audio->buffFlag[k]){
		    //fprintf(stderr,"getbuffAudio k %d audio->buff[k] %d\n",k,audio->buff[k]);
		    audio->buffFlag[k]=1;
            ret = audio->buff[k];
            break;
		}
	}
    
    pthread_mutex_unlock(&buffermutex);

	return ret;
}
int stopAudio(struct audioInfo *audio)
{
	if(!audio)return 1;
	
	alSourceStopv(audio->numsource, audio->source);
    alDeleteSources(audio->numsource, audio->source);

	
	if(audio->source)cFree((unsigned char *)audio->source);
	if(audio->sourceFlag)cFree((unsigned char *)audio->sourceFlag);
	
	
    alDeleteBuffers(audio->numbuff, audio->buff);
	
	if(audio->buff)cFree((unsigned char *)audio->buff);
	if(audio->buffFlag)cFree((unsigned char *)audio->buffFlag);
	
    audio->ctx = alcGetCurrentContext();
    audio->dev = alcGetContextsDevice(audio->ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio->ctx);
    alcCloseDevice(audio->dev);
    
    pthread_mutex_destroy(&buffermutex);
	
	return 0;	
}

static int cFree(unsigned char *data)
{
	if(!data)return 1;
	
	free(data);
	
	return 0;
}
static void *cMalloc(unsigned long n)
{
	return malloc(n);
}
static int zerol(unsigned char *s,unsigned long n)
{
    if(!s || (n <= 0))return 1;

	while(n-- > 0)*s++ = 0;
	
	return 0;
}

ALvoid DisplayALError(unsigned char *szText, ALint errorcode)
{
	printf("%s%s\n", szText, alGetString(errorcode));
}
