#include "audiolib.h"
#include <Utilities.h>
#include <mutex>

static std::mutex buffermutex;

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
	
	zerol((char *)audio,sizeof(*audio));
	
    audio->dev = alcOpenDevice(NULL); // select the "preferred dev" 
    audio->ctx = alcCreateContext(audio->dev,NULL); 
    alcMakeContextCurrent(audio->ctx);  
	
	audio->numsource=numsource;
	
	audio->source=(ALuint *)cMalloc(numsource*sizeof(*(audio->source)),3525);
	if(!audio->source)return 1;
	
	audio->sourceFlag=(int *)cMalloc(numsource*sizeof(*(audio->sourceFlag)),3456);
	if(!audio->sourceFlag)return 1;
	
	zerol((char *)audio->sourceFlag,numsource*sizeof(*(audio->sourceFlag)));
	
	ALenum error;
	
	alGenSources(numsource, audio->source);
    if ((error = alGetError()) != AL_NO_ERROR){
        DisplayALError((unsigned char *)"alGenSources : ", error);
        return 1;
    }
    
    
	audio->numbuff=numbuff;
	
	audio->buff=(ALuint *)cMalloc(numbuff*sizeof(*(audio->buff)),4054);
	if(!audio->buff)return 1;
	
	audio->buffFlag=(int *)cMalloc(numbuff*sizeof(*(audio->buffFlag)),3455);
	if(!audio->buffFlag)return 1;
	
	zerol((char *)audio->buffFlag,numbuff*sizeof(*(audio->buffFlag)));
	
	alGenBuffers(numbuff, audio->buff);
    if ((error = alGetError()) != AL_NO_ERROR){
        DisplayALError((unsigned char *)"alGenBuffers : ", error);
        return 1;
    }
    
 
	return 0;
}
ALuint freebuffAudio(struct audioInfo *audio,ALuint n)
{
    int ret;
    
    if(!audio)return 1;
    
    buffermutex.lock();
    
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
    
    buffermutex.unlock();

	return ret;
}
ALuint getbuffAudio(struct audioInfo *audio)
{
    int ret;
    
	if(!audio)return NO_MORE_SPACE;
	
    buffermutex.lock();

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
    
    buffermutex.unlock();

	return ret;
}
int stopAudio(struct audioInfo *audio)
{
    
   // fprintf(stderr,"audio->source %p\n",audio->source);
    
	if(!audio)return 1;
	
	alSourceStopv(audio->numsource, audio->source);
    alDeleteSources(audio->numsource, audio->source);

	
	if(audio->source)cFree((char *)audio->source);
	if(audio->sourceFlag)cFree((char *)audio->sourceFlag);
	
	
    alDeleteBuffers(audio->numbuff, audio->buff);
	
	if(audio->buff)cFree((char *)audio->buff);
	if(audio->buffFlag)cFree((char *)audio->buffFlag);
	
    audio->ctx = alcGetCurrentContext();
    audio->dev = alcGetContextsDevice(audio->ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio->ctx);
    alcCloseDevice(audio->dev);
    
	return 0;	
}

ALvoid DisplayALError(unsigned char *szText, ALint errorcode)
{
	printf("%s%s\n", szText, alGetString(errorcode));
}
