#include "sound.h"

SNDFILE *sfopen(char *filename,int faudio);

static struct audioInfo audioi;

soundClass::soundClass()
{
	
	audiodevice=129;
	faudio=48000;
	ncut=40;
	bS=NULL;
	soundRun=0;
	audioSync=0;
	inputID=-1;
	outputID=-1;
	gain=1.0;
	outfile=NULL;
	audio=NULL;
}
int soundClass::startSound()
{
	
	//fprintf(stderr,"startSound Sleep faudio %g\n",faudio);
	
	if(soundRun)return 0;
		
	soundRun=1;
	
	if(startAudio(&audioi,10,8)){
	    fprintf(stderr,"startAudio failed\n");
	    return 1;
	}
	
	audio=&audioi;
	
	play.channels=1;
	samplerate=48000;
	play.source=getsourceAudio(audio);
	if(play.source == NO_MORE_SPACE){
		fprintf(stderr,"play All sources used\n");
		return 1;
	}
		
	std::thread(&soundClass::playSound,this).detach();
	
	while(soundRun >= 0){
	    //fprintf(stderr,"startSound Sleeping %d\n",soundRun);
		Sleep2(20);
	}
	
	soundRun=0;

	//fprintf(stderr,"stopAudio soundRun %d  end\n",soundRun);
	
	stopAudio(audio);

	Sleep2(100);

	return 0;
}
void soundClass::playSound()
{	
	ALint processed;
    ALenum error;
	ALint val;

	setBuffers(&play);
	setBuffers(&play);
	setBuffers(&play);
	setBuffers(&play);
	setBuffers(&play);
	setBuffers(&play);

	alSourcePlay(play.source);
    if ((error = alGetError()) != AL_NO_ERROR) 
    { 
        DisplayALError((unsigned char *)"alSourcePlay : ", error); 
    } 
	
	ALuint *fbuff= new ALuint[audio->numbuff];
	
	long int count=0;
	
	for (;;) {
		do {
			processed=0;
			if(soundRun < 1)goto OutOfHere;
			alGetSourcei(play.source, AL_BUFFERS_PROCESSED, &processed);
			if(processed == 0)Sleep2(5);
		} while (!processed);
		
		alSourceUnqueueBuffers(play.source,processed,fbuff);
		
		for(int k=0;k<processed;++k){
		//    fprintf(stderr,"finish bufferd %ld\n",(long)k);
			freebuffAudio(audio,fbuff[k]);
		}
		if(setBuffers(&play))break;
		
		alGetSourcei(play.source, AL_SOURCE_STATE, &val);
		//printf("rx->al_state %d AL_STOPPED %d\n",rx->al_state,AL_STOPPED);
		if(val == AL_STOPPED){
			Sleep2(500);
			alSourcePlay(play.source);
			if(count % 5){
				fprintf(stderr,"count %lu sound restarted\n",count);
			}
		}
	}
OutOfHere:
	do{
        alGetSourcei(play.source, AL_SOURCE_STATE, &val);
        Sleep2(5);
        //fprintf(stderr,"wait val %d\n",val);
    }while(val == AL_PLAYING);
	

	return;
} 
int soundClass::setBuffers(struct playData *play)
{
    ALenum error;		
    
	short int buffer2[2*48000];
	
	unsigned int nBufferFrames=48000.0/ncut;
    
    int ibuff = -1;
	
	if(bS)ibuff=bS->popBuffa();
	
	
	if (ibuff >= 0){
		short int *buff= bS->buffa[ibuff % NUM_ABUFF2];

		double amin=1e33;
		double amax=-133;
		for (unsigned int n=0; n<nBufferFrames; n++ ) {
			short int v=(short int)(gain*buff[n]);
			if(v < amin)amin=v;
			if(v > amax)amax=v;
			buffer2[n] = v;
		}
		
		if(outfile){
			size_t ret=sf_write_short(outfile,buff,nBufferFrames);
			if(ret == 0){
				fprintf(stderr,"Error sf_write_short \n");
			}
		}
		//fprintf(stderr,"ibuff %u amax %g amin %g nBufferFrames %u gain %g\n",ibuff,amax,amin,nBufferFrames,gain);
	}else{
		for (unsigned int n=0; n<nBufferFrames; n++ ) {
			  buffer2[n] = 0;
		}
	/*
		double pi=4.0*atan(1.0);
		double freq=1000;
		for (unsigned int n=0; n<nBufferFrames*2; n++) {
		   buffer2[n] = 32760 * sin( (2.0*pi*freq)/play->samplerate * n );
		}
		
		fprintf(stderr,"channels %d samplerate %d samplerate %f\n",play->channels,play->samplerate,samplerate);
	*/
	}
	
	audioSync=0;
 
 	if(soundRun < 1)return 1;
	
	ALuint buffer=getbuffAudio(audio);
	if(buffer == NO_MORE_SPACE){
		fprintf(stderr,"setBuffers All buffers used\n");
		return 1;
	}
	
    alBufferData(buffer, 
		 play->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
		 buffer2, 
		 play->channels*sizeof(short)*nBufferFrames, 
		 samplerate);
		 
    if ((error = alGetError()) != AL_NO_ERROR){ 
        DisplayALError((unsigned char *)"alBufferData 1 : ", error); 
        return 1; 
    } 
		 
    alSourceQueueBuffers(play->source, 1, &buffer);
    
    if ((error = alGetError()) != AL_NO_ERROR){ 
        DisplayALError((unsigned char *)"alSourceQueueBuffers 1 : ", error); 
        return 1; 
    } 
    
	return 0;
}

int soundClass::startRecord(char *fname)
{

	if(fname == NULL){
	//	fprintf(stderr,"stop Record %s\n",fname);
		if(outfile)sf_close(outfile);
		outfile=NULL;
	}else{
	//	fprintf(stderr,"startRecord %s\n",fname);
		outfile=sfopen(fname,faudio);
		if(!outfile)fprintf(stderr,"Error Opening File %s for WAV data\n",fname);
	}
	return 0;
}
SNDFILE *sfopen(char *filename,int faudio)
{
    char strbuffer[256];
    static int pversion=0;
    
    sf_command (NULL, SFC_GET_LIB_VERSION, strbuffer, sizeof (strbuffer)) ;
    
    if(pversion == 1)printf("sndfile version %s\n",strbuffer);
    pversion=0;
    
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof (sfinfo));
    sfinfo.samplerate = faudio;
    sfinfo.channels   = 1; // mono
    sfinfo.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    
    if(!filename)return NULL;
    
    SNDFILE * fid_wav = sf_open(filename, SFM_WRITE, &sfinfo);
    
    return fid_wav;
}

soundClass::~soundClass()
{
	fprintf(stderr,"exit soundClass %p\n",this);
}

static void *cMalloc(unsigned long n);

static int cFree(unsigned char *data);

int updateAudio(struct audioInfo *audio)
{
	if(!audio)return 1;
	
	return 0;
}
int freesourceAudio(struct audioInfo *audio,ALuint n)
{
	for(int k=0;k<audio->numsource;++k)
	{
		if(audio->source[k] == n){
		    audio->sourceFlag[k]=0;
			break;
		}
	}
	
	return 0;
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
	error = alGetError();
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

	return 0;
}
ALuint freebuffAudio(struct audioInfo *audio,ALuint n)
{	
	if(!audio)return 1;
	
	for(int k=0;k<audio->numbuff;++k)
	{
		if(audio->buff[k] == n){
		    audio->buffFlag[k]=0;
		    //fprintf(stderr,"freebuffAudio k %d audio->buff[k] %d\n",k,audio->buff[k]);
			break;
		}
	}
	
	
	return 0;
}
ALuint getbuffAudio(struct audioInfo *audio)
{
	if(!audio)return NO_MORE_SPACE;
	
	for(int k=0;k<audio->numbuff;++k)
	{
		if(!audio->buffFlag[k]){
		    //fprintf(stderr,"getbuffAudio k %d audio->buff[k] %d\n",k,audio->buff[k]);
		    audio->buffFlag[k]=1;
			return audio->buff[k];
		}
	}
	
	return NO_MORE_SPACE;
}
ALuint getsourceAudio(struct audioInfo *audio)
{
	if(!audio)return NO_MORE_SPACE;
	
	for(int k=0;k<audio->numsource;++k)
	{
		if(!audio->sourceFlag[k]){
		    audio->sourceFlag[k]=1;
			return audio->source[k];
		}
	}
	
	return NO_MORE_SPACE;
}
int stopAudio(struct audioInfo *audio)
{
	if(!audio)return 1;
	
	alSourceStopv(audio->numsource, audio->source);
    alDeleteSources(audio->numsource, audio->source);

	
	if(audio->source)cFree((unsigned char *)audio->source);
	audio->source=NULL;
	if(audio->sourceFlag)cFree((unsigned char *)audio->sourceFlag);
	audio->sourceFlag=NULL;
	
	
    alDeleteBuffers(audio->numbuff, audio->buff);
	
	if(audio->buff)cFree((unsigned char *)audio->buff);
	audio->buff=NULL;
	if(audio->buffFlag)cFree((unsigned char *)audio->buffFlag);
	audio->buffFlag=NULL;
	
    audio->ctx = alcGetCurrentContext();
    audio->dev = alcGetContextsDevice(audio->ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio->ctx);
    alcCloseDevice(audio->dev);
	
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
ALvoid DisplayALError(unsigned char *szText, ALint errorcode)
{
	printf("%s%s\n", szText, alGetString(errorcode));
}

