//
//  FMRadio.cpp
//  SdrGlut
//
//  Created by Dale on 10/8/16.
//  Copyright © 2016 Dale Ranta. All rights reserved.
//
#include "firstFile.h"
#include <SoapySDR/Version.h>

#include "AudioFile.h"

#include "PlayIQ.h"

#include "mThread.h"

#include <math.h>

#include <GL/glui.h>

#include <string.h>

extern "C" int closeScene(struct Scene *scene);


extern "C" int DrawLine(int x1, int y1, int x2, int y2);

#define ControlGetSelectionBox	102
#define SdrDialog               103


extern "C" int doFFT2(double *x,double *y,long length,int direction);

int doWindow(double *x,double *y,long length,int type);

int doAudioFileOpen(char *name);

static int doRadioOpen2(char *name);

int dialogFileOpen(struct Scene *scene);

ALvoid DisplayALError(unsigned char *szText, ALint errorcode);

static int AudioReset(struct playData4 *play,AudioFilePtr sdr);

static int ProcessSound(void *rxv);

static int freeFilters(struct Filters2 *f);

static int setFilters(struct playData4 *play,struct Filters2 *f);

AudioFile::AudioFile(struct Scene *scene): CWindow(scene)
{
    OpenError=TRUE;
    
    zerol((char *)&water,sizeof(water));
    
    zerol((char *)&pd,sizeof(pd));
    
    zerol((char *)&play,sizeof(play));
    
    zerol((char *)&filter,sizeof(filter));

	zerol((char *)&dd, sizeof(dd));

    zerol((char *)&start, &end-&start+1);

    getPaletteByName((char *)"ps",(unsigned char *)&pd.palette);
    
    pd.sType=2;
    
    //fprintf(stderr,"Radio::Radio\n");
    
    play.frame=0;
    play.gain=1.0;
    play.fc=1.0e6;
    play.f=0.6e6;
    //play.fc=162.39e6;
    //play.f=162.4e6;
    play.scaleFactor=0.0;
    play.FFTcount=4096;
    play.FFTfilter=FILTER_BLACKMANHARRIS7;


    play.fOut=48000;
    play.decodemode=MODE_AM;
    play.decodemode=MODE_NBFM;
    play.samplerate=2000000;
        
    play.Debug=0;
    
    backGroundEvents=0;
    
    inDialog=0;
    
    mute=0;
    
    lineDumpInterval=0.1;
    lineTime=rtime()+lineDumpInterval;
    
    lineAlpha=0.1;
    
    FFTlength=32768;
    
    range=NULL;
    dose=NULL;
    
    lreal=NULL;
    limag=NULL;
    
    p1=NULL;
    p2=NULL;
    p3=NULL;
    p4=NULL;
    p5=NULL;
    p6=NULL;
    p7=NULL;

    range=(double *)cMalloc(FFTlength*sizeof(double),9851);
    dose=(double *)cMalloc(FFTlength*sizeof(double),9851);
    
    lreal=(double *)cMalloc(FFTlength*sizeof(double),9851);
    limag=(double *)cMalloc(FFTlength*sizeof(double),9851);
    
    

    if(!range || !dose || !lreal || !limag)return;
    
    zerol((char *)lreal,FFTlength*sizeof(double));
    zerol((char *)limag,FFTlength*sizeof(double));
    

    OpenError=FALSE;
}

static int doRadioOpen2(char *name)
{
    struct SceneList *list;
    struct Scene *scene;
    int FileType;
    
    FileType=FileTypeAudio;

    list=SceneNext();
    if(list == NULL)
    {
        WarningPrint("doRadioOpen : Error Allocation Scene Memory File\n",FileType);
        return 1;
    }
    scene=&list->scene;
    zerol((char *)scene,sizeof(struct Scene));
    SceneInit(scene);
    scene->windowType=FileTypeAudio;

    
    // fprintf(stderr,"doRadioOpen2 name %s\n",name);
    

    AudioFilePtr w = new AudioFile(scene);
    
    if(w == NULL){
        WarningBatch((char *)"Radio of Memory");
        return 1;
    }
    
    if(w->OpenError != FALSE){
        delete w;
        return 1;
    }
    
    int ret=0;
    
   // if(w->OpenWindows(scene))ret=1;
    
    if(w->LoadFile(scene,name,FileType)){
        ret=1;
    }
    
    myAppl=(CWinPtr)w;
    
    AddWindowList(myAppl);
    
    if(ret){
        closeScene(scene);
        return 1;
    }
    
    w->backGroundEvents=1;
    
    w->dialogSdrFile(scene);

   return 0;
}

int doAudioFileOpen(char *name)
{
    
    doRadioOpen2(name);
    
    return 0;
}
int AudioFile::setInfo()
{
    unsigned int samples=48000;
    
    
    if(play.decodemode == MODE_CW){
        info.Tone=1;
    }else{
        info.Tone=0;
    }
    
    info.f=1000;
    
    double pi;
    pi=4.0*atan(1.0);
    info.dt=1.0/(double)samples;
    info.sino=0;
    info.coso=1;
    double w=2.0*pi*(info.f);
    info.sindt=sin(w*info.dt);
    info.cosdt=cos(w*info.dt);

    return 0;
}
int AudioFile::doFliters(int samplerate)
{
    p1=new Poly(48000);
    p1->Clowpass("butter",10,1.0,375);

    p2=new Poly(48000);
    p2->Cbandpass("butter",10,1.0,375,750);
    
    p3=new Poly(48000);
    p3->Cbandpass("butter",10,1.0,750,1500);

    p4=new Poly(48000);
    p4->Cbandpass("butter",10,1.0,1500,3000);

    p5=new Poly(48000);
    p5->Cbandpass("butter",10,1.0,3000,6000);

    p6=new Poly(48000);
    p6->Cbandpass("butter",10,1.0,6000,12000);

    p7=new Poly(48000);
    p7->Chighpass("butter",10,1.0,12000);

    return 0;
}
int AudioFile::LoadFile(struct Scene *scene,char *filename, int fileType)
{
    char name[512];
    
    if(play.infile)sf_close(play.infile) ;
    play.infile=NULL;
    
    if ((play.infile = sf_open (filename, SFM_READ, &play.sfinfo))  == NULL)
    {
        printf ("Not able to open input file %s.\n", filename) ;
        return 1 ;
    }
    
   // sf_command (play.infile , SFC_GET_CURRENT_SF_INFO, &play.sfinfo, sizeof (&play.sfinfo));

    printf("frames %lld samplerate %d channels %d format %x\n",
           play.sfinfo.frames,play.sfinfo.samplerate,
           play.sfinfo.channels,play.sfinfo.format);
    
    doFliters(play.sfinfo.samplerate);
    
    mstrncpy(name,filename,sizeof(name));
    mstrncpy(windowName,filename,sizeof(windowName));
    
    play.samplerate=play.sfinfo.samplerate;
    play.fc=play.samplerate/2;
    
    glutSetWindowTitle(filename);
    
    glutSetWindow(this->window);
    glutSetWindowTitle(filename);
    

    if(initPlay(&play))return 1;
    
    playFile (&play);

    return 0;
}
int AudioFile::initPlay(struct playData4 *play)
{
    play->source=getsourceAudio(audio);
    if(play->source== NO_MORE_SPACE){
        fprintf(stderr,"getsourceAudio out of sources\n");
        return 1;
    }
    
    return 0;
}

void AudioFile::playFile (struct playData4 *play)
{
    
    // ALint processed;
    // ALint val;
    
    play->controlProcess  = -1;
    
    Sleep2(100);
    
     
    for(int k=0;k<NUM_BUFFERS;++k){
        play->bufferState[k]=0;
    }
    
    setFilters(play, &filter);
    
    StartIt(play);
    
    play->controlProcess=0;
    
    launchThread((void *)this,ProcessSound);
    
    return ;
}
int AudioFile::StartIt(struct playData4 *play)
{
    ALenum error;

    play->frame = 0;
    
    if(play->infile)sf_seek(play->infile, 0, SEEK_SET);

    setBuffers(play);
    setBuffers(play);
    setBuffers(play);
    
    alSourcePlay(play->source);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError((unsigned char *)"alSourcePlay : ", error);
    }
    
    
    return 0;

}
int AudioFile::setBuffers(struct playData4 *play)
{
    ALenum error;
    int readcount;
    sf_count_t k;
    int ret;
    
    ALuint buffer;
    
    ret=0;
        
    buffer=getbuffAudio(audio);
    if(buffer == NO_MORE_SPACE){
        fprintf(stderr,"getbuffAudio out of buffers\n");
        return 1;
    }
    
    float *buf1=filter.buf1;
    float *buf2=filter.buf2;

    int nchannels=play->sfinfo.channels;
    k=(sf_count_t)play->size;
    readcount = 0;
    
    if(!wait){
        readcount = (int)sf_readf_short(play->infile,(short *)buf1,k);
        play->frame++;
    }

    //printf(" play->frame %f readcount %d buffer %d play->size %d\n", play->frame,readcount,(int)buffer,play->size);

    short *buf3=(short *)buf1;

    if(k != readcount){
        for(int n=0;n<k;++n){
            buf3[2*n]=0.0;
            buf3[2*n+1]=0.0;
        }
        if(!wait)ret=1;
    }
    
    float gain=play->gain;
    if(mute || wait){
        gain=0;
    }
    
    for(int n=(int)(k-1);n >= 0;--n){
        if(nchannels == 2){
            buf1[n]=gain*(buf3[2*n]+buf3[2*n+1])*0.5;
        }else{
            buf1[n] = gain*buf3[n];
        }
    }
    
    /*
        for(int k=0;k<play->FFTcount;++k){
            if(k < play->size){
                play->real[k]=buf3[2*k];
                play->imag[k]=buf3[2*k+1];
            }else{
                play->real[k]=0;
                play->imag[k]=0;
            }
        }
    */

    unsigned int num=(unsigned int)k;
    unsigned int num2=0;
    if(filter.iqSampler2){
        msresamp_rrrf_execute(filter.iqSampler2, (float *)buf1, num, (float *)buf2, &num2);  // interpolate
        for(unsigned int n=0;n<num2;++n){
            buf1[n] = buf2[n];
        }
        //printf("num %d num2 %d\n",num,num2);
        num=num2;
    }
    
    p1->forceCascadeRun(buf1,buf2,num,0);
    p2->forceCascadeRun(buf1,buf2,num,1);
    p3->forceCascadeRun(buf1,buf2,num,1);
    p4->forceCascadeRun(buf1,buf2,num,1);
    p5->forceCascadeRun(buf1,buf2,num,1);
    p6->forceCascadeRun(buf1,buf2,num,1);
    p7->forceCascadeRun(buf1,buf2,num,1);

    for(unsigned int n=0;n<num;++n){
        buf3[n] = buf2[n];
    }

  //  printf("play->frame %d\n",play->frame);

    alBufferData(buffer,
                 AL_FORMAT_MONO16,
                 buf3,
                 num  * sizeof(short),
                 48000);
    
    if ((error = alGetError()) != AL_NO_ERROR){
        fprintf(stderr,"Error on buffer # %d\n",buffer);
        DisplayALError((unsigned char *)"alBufferData 1 : ", error);
        return 1;
    }
    
    alSourceQueueBuffers(play->source, 1, &buffer);
    
    if ((error = alGetError()) != AL_NO_ERROR){
        DisplayALError((unsigned char *)"alSourceQueueBuffers 1 : ", error);
        return 1;
    }
        
     return ret;
}


AudioFile::~AudioFile()
{
    fprintf(stderr,"~AudioFile \n");
    
    stopPlay(&play);

    if(p1)delete p1;
    if(p2)delete p2;
    if(p3)delete p3;
    if(p4)delete p4;
    if(p5)delete p5;
    if(p6)delete p6;
    if(p7)delete p7;

  
    if(play.infile)sf_close(play.infile) ;
    play.infile=NULL;

    if(range)cFree((char *)range);
    range=NULL;
    
    if(dose)cFree((char *)dose);
    dose=NULL;
    
    if(lreal)cFree((char *)lreal);
    lreal=NULL;
    
    if(limag)cFree((char *)limag);
    limag=NULL;
        
    if(water.data)cFree((char *)water.data);
    water.data=NULL;
    

    sceneClean(scene);
    
    if(inDialog){
        inDialog=0;
        if(dd.glui){
            glutSetWindow(dd.sub_window);
            dd.glui->close();
        }
        dd.glui=NULL;
    }

}
int AudioFile::fftIndex(double frequency)
{
    return -1;
}

int AudioFile::updateLine()
{

    return 0;
}

int AudioFile::UpdateTemperatureColors(struct Scene *scene)
{
    if(!scene)return 1;
    
    for(int n=0;n<768;++n){
        pd.palette[n]=g.palette[n];
    }
    
    return 0;
}
static int freeFilters(struct Filters2 *f)
{
    if (f->iqSampler)msresamp_crcf_destroy(f->iqSampler);
    f->iqSampler=0;

    if (f->iqSampler2)msresamp_rrrf_destroy(f->iqSampler2);
    f->iqSampler2=0;

    if(f->lowpass)iirfilt_crcf_destroy(f->lowpass);
    f->lowpass=0;

    if(f->dcFilter)iirfilt_crcf_destroy(f->dcFilter);
    f->dcFilter=0;


    if(f->demod)freqdem_destroy(f->demod);
    f->demod=0;

    if(f->demodAM)ampmodem_destroy(f->demodAM);
    f->demodAM=0;
    
    if(f->buf1)cFree((char *)f->buf1);
    f->buf1=NULL;
    
    if(f->buf2)cFree((char *)f->buf2);
    f->buf2=NULL;
    
    if(f->data)cFree((char *)f->data);
    f->data=NULL;

    return 0;

}
static int setFilters(struct playData4 *rx,struct Filters2 *f)
{
    float As = 60.0f;
    
    float ratio= (float)(48000.0/rx->samplerate);

    printf("ratio %f\n",ratio);
    
    if(ratio > 1.11 || ratio < 0.99)f->iqSampler2 = msresamp_rrrf_create(ratio, As);

    double rate=rx->samplerate;
    
    int size=(int)(rate/10.0);
    
    rx->size=size;
    
    size = 48000.0;
    if(ratio > 1.0)size = (size+1)*ratio;
    
    if(f->buf1)cFree((char *)f->buf1);
    f->buf1=(float *)cMalloc(2*size*sizeof(float),8887);
    if(!f->buf1){
        fprintf(stderr,"1 cMalloc Errror %ld\n",(long)(2*size*sizeof(float)));
        return 1;
    }
    
    if(f->buf2)cFree((char *)f->buf2);
    f->buf2=(float *)cMalloc(2*size*sizeof(float),8887);
    if(!f->buf2){
        fprintf(stderr,"2 cMalloc Errror %ld\n",(long)(2*size*sizeof(float)));
        return 1;
    }
    
    if(f->data)cFree((char *)f->data);
    f->data=(short *)cMalloc(2*size*sizeof(float),8887);
    if(!f->data){
        fprintf(stderr,"2 cMalloc Errror %ld\n",(long)(2*size*sizeof(float)));
        return 1;
    }


    return 0;
    
}
int AudioFile::BackGroundEvents(struct Scene *scene)
{
    if(!backGroundEvents)return 1;
    
    setDialogFrame((play.frame*play.size));

    updateLine();
    
    return 0;
}
static int ProcessSound(void *rxv)
{
    AudioFilePtr sdr=(AudioFilePtr)rxv;
    
    struct playData4 *play=&sdr->play;

    ALint processed;
    ALenum error;

    if(!play)return 1;
    
    if(play->ctx){
        alcMakeContextCurrent(play->ctx);
        alcProcessContext(play->ctx);
    }
StartUp:
    while(play->controlProcess >= 0)
    {
         Sleep2(2);
         processed=0;
        
        alGetSourcei(play->source, AL_BUFFERS_PROCESSED, &processed);
        
        if(processed){
            ALuint *fbuff= new ALuint[audio->numbuff];
            alSourceUnqueueBuffers(play->source, processed, fbuff);
            if ((error = alGetError()) != AL_NO_ERROR)
            {
                DisplayALError((unsigned char *)"alSourceUnqueueBuffers : ", error);
            }
            for(ALint k=0;k<processed;++k){
               // fprintf(stderr,"finish bufferd %d\n",fbuff[k]);
                freebuffAudio(audio,fbuff[k]);
            }
            
            if(sdr->setBuffers(play)){
                //fprintf(stderr,"stopPlay frame %d - rewind SDR File",play->frame);
                play->frame = 0;
                sf_seek(play->infile, 0, SEEK_SET);
           }
            
         }else{
            alGetSourcei(play->source, AL_SOURCE_STATE, &play->al_state);
            //fprintf(stderr,"al state %d %x AL_STOPPED %d AL_INITIAL %d AL_PLAYING %d\n",play->al_state,play->al_state,AL_STOPPED,AL_INITIAL,AL_PLAYING);
            if(play->al_state != AL_PLAYING && play->al_state != AL_INITIAL){
                if(play->al_state == AL_STOPPED){
                   AudioReset(play,sdr);
                }else{
                    printf("al state %d %x\n",play->al_state,play->al_state);
                }
            }
            if(play->al_state == AL_INITIAL){
                sdr->StartIt(play);
            }
         }
       
    }
    
    if(play->controlProcess == -2){
        play->controlProcess=0;
        freeFilters(&sdr->filter);
        setFilters(&sdr->play,&sdr->filter);
        goto StartUp;
    }else if(play->controlProcess == -3){
        play->controlProcess=0;
        sf_count_t location=(sf_count_t)(play->setFrameNumber*play->samplerate);
        play->frame = play->setFrameNumber*play->samplerate/play->size;
        sf_seek(play->infile, location, SEEK_SET);
        //fprintf(stderr,"setFrameNumber %ld location %lld ret = %lld play->frame %d\n",play->setFrameNumber,location,ret,play->frame);
        //sdr->setDialogFrame((int)location);
        goto StartUp;
    }
    return 0;
}
static int AudioReset(struct playData4 *rx,AudioFilePtr sdr)
{
    
    ALint processed;
    ALenum error;
    
    alSourceStopv(1, &rx->source);
    
    processed = 0;
    alGetSourcei(rx->source, AL_BUFFERS_PROCESSED, &processed);
    
    while (processed) {
        ALuint fbuff;
        alSourceUnqueueBuffers(rx->source, 1, &fbuff);
        if ((error = alGetError()) != AL_NO_ERROR)
        {
            DisplayALError((unsigned char *)"AudioReset alSourceUnqueueBuffers : ", error);
        }
        freebuffAudio(audio, fbuff);
        fprintf(stderr, "AudioReset free buffer %d\n", fbuff);
        --processed;
    }
    
    sdr->setBuffers(rx);
    
    alSourcePlay(rx->source);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError((unsigned char *)"alSourcePlay : ", error);
    }

    printf("AudioReset \n");
    
    return 0;
}

int AudioFile::SetFrequency(struct Scene *scene,double f,double bw,int message)
{
    return 0;
}


int AudioFile::setFrequency(struct playData4 *play)
{
    return 0;
}
int AudioFile::stopPlay(struct playData4 *play)
{
    play->controlProcess=-1;

    Sleep2(100);

	freeFilters(&filter);

    ALint processed;
    ALenum error;
    
    if(checksourceAudio(audio, play->source)){
        alSourceStopv(1, &play->source);
        
        alGetSourcei(play->source, AL_SOURCE_STATE, &play->al_state);
        fprintf(stderr,"stopPlay al state %d %x\n",play->al_state,play->al_state);
        
        processed=0;
        alGetSourcei(play->source, AL_BUFFERS_PROCESSED, &processed);
        
        while(processed){
            ALuint fbuff;
            alSourceUnqueueBuffers(play->source, 1, &fbuff);
            if ((error = alGetError()) != AL_NO_ERROR)
            {
                DisplayALError((unsigned char *)"stopPlay alSourceUnqueueBuffers : ", error);
            }
            freebuffAudio(audio,fbuff);
            --processed;
        }
        
        freesourceAudio(audio,play->source);
    }

    return 0;
}
int AudioFile::SetWindow(struct Scene *scene)
{
    
    CheckSceneBuffer(scene);
    
    return 0;
}



int AudioFile::Display(struct Scene *scene)
{
    if(!scene)return 1;
    
    return 0;
}

int AudioFile::OpenWindows(struct Scene *scene)
{
   return 0;
}

int AudioFile::resetDemod()
{
    return 0;
}
int AudioFile::mMenuSelectl(struct Scene *scene,int item)
{
    if(!scene)return 1;
    
    return 0;
}

void AudioFile::getMouse(int button, int state, int x, int y)
{
    
}

int AudioFile::FindPoint(struct Scene *scene,int x,int y)
{
    return 0;
}
