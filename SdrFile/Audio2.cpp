//
//  FMRadio.cpp
//  SdrGlut
//
//  Created by Dale on 10/8/16.
//  Copyright © 2016 Dale Ranta. All rights reserved.
//
#include "firstFile.h"
#include <SoapySDR/Version.h>

#include "DialogFileOpenAres.h"

#include "Audio2.h"

#include "PlayIQ.h"

#include "mThread.h"

#include <math.h>

#include <GL/glui.h>

#include <string.h>

int DrawLine3(int x1, int y1, int x2, int y2);


extern "C" int closeScene(struct Scene *scene);


extern "C" int DrawLine(int x1, int y1, int x2, int y2);

#define ControlGetSelectionBox	102
#define SdrDialog               103


extern "C" int doFFT2(double *x,double *y,long length,int direction);

int doWindow(double *x,double *y,long length,int type);

int doAudio2Open(char *name);

static int doRadioOpen2(char *name);

int dialogFileOpen(struct Scene *scene);

ALvoid DisplayALError(unsigned char *szText, ALint errorcode);

static int AudioReset(struct playData4 *play,Audio2Ptr sdr);

static int ProcessSound(void *rxv);

static int freeFilters(struct Filters2 *f);

static int setFilters(struct playData4 *play,struct Filters2 *f);

void SetLighting(unsigned int mode);

#define close2 30
#define quit 31


Audio2::Audio2(struct Scene *scene): CWindow(scene)
{
    OpenError=TRUE;
    
    zerol((char *)&water,sizeof(water));
    
    zerol((char *)&pd,sizeof(pd));
    
    zerol((char *)&rxs,sizeof(rxs));
    
    zerol((char *)&filter,sizeof(filter));

	zerol((char *)&dd, sizeof(dd));

    zerol((char *)&start, &end-&start+1);

    getPaletteByName((char *)"ps",(unsigned char *)&pd.palette);
    
    pd.sType=0;
    
    //fprintf(stderr,"Radio::Radio\n");
    
    
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
    
    range=(double *)cMalloc(FFTlength*sizeof(double),9851);
    dose=(double *)cMalloc(FFTlength*sizeof(double),9851);
    
    lreal=(double *)cMalloc(FFTlength*sizeof(double),9851);
    limag=(double *)cMalloc(FFTlength*sizeof(double),9851);
    
    

    if(!range || !dose || !lreal || !limag)return;
    
    zerol((char *)lreal,FFTlength*sizeof(double));
    zerol((char *)limag,FFTlength*sizeof(double));
    
    OpenError=TRUE;
    backGroundEvents=0;
    
 //   zerol((char *)&rxs,(&rxs.end-&rxs.start)+1);
    rx=&rxs;
    rx->f=100e6;
    rx->fc=100e6;
    rx->bw=5000;
    rx->viewWindow=2e6;
    rx->frame=0;
    rx->gain=1.0;
    rx->fc=1.0e6;
    rx->f=0.6e6;

    rx->scaleFactor=0.0;
    rx->FFTcount=4096;
    rx->FFTfilter=FILTER_BLACKMANHARRIS7;


    rx->fOut=48000;
    rx->decodemode=MODE_AM;
    rx->decodemode=MODE_NBFM;
        
    rx->Debug=0;


    OpenError=FALSE;
}

static int doRadioOpen2(char *name)
{
    struct SceneList *list;
    struct Scene *scene;
    int FileType;
    
    int save_window=glutGetWindow();
    
    FileType=FileTypeAudio2;

    list=SceneNext();
    if(list == NULL)
    {
        WarningPrint("doRadioOpen : Error Allocation Scene Memory File\n",FileType);
        return 1;
    }
    scene=&list->scene;
    zerol((char *)scene,sizeof(struct Scene));
    SceneInit(scene);
    scene->windowType=FileType;

    
    // fprintf(stderr,"doRadioOpen2 name %s\n",name);
    

    Audio2Ptr w = new Audio2(scene);
    
    if(w == NULL){
        WarningBatch((char *)"Radio of Memory");
        return 1;
    }
    
    if(w->OpenError != FALSE){
        delete w;
        return 1;
    }
    
    int ret=0;
    
    if(w->OpenWindows(scene))ret=1;
    
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
    
    //w->dialogSdrFile(scene);
    
    if(save_window)glutSetWindow(save_window);

   return 0;
}

int doAudio2Open(char *name)
{
    
    doRadioOpen2(name);
    
    return 0;
}
int Audio2::SetWindow(struct Scene *scene)
{
    
    CheckSceneBuffer(scene);
    
    int xsize=(int)scene->xResolution;
    
    int ysize=(int)scene->yResolution/2;
    
    water.DRect.x=0;
    water.DRect.y=0;
    water.DRect.xsize=xsize;
    water.DRect.ysize=ysize;
    
    //xsize=(int)rx->FFTcount;
    
    if(ysize == water.ysize && xsize == water.xsize && water.data)return 0;
    
    water.amin=1e60;
    water.amax=-1e60;
    
    water.SRect.x=0;
    water.SRect.y=0;
    water.SRect.xsize=xsize;
    water.SRect.ysize=ysize;
    
    water.nline=0;
    
    if(water.data)cFree((char *)water.data);
    
    water.data=(unsigned char *)cMalloc(2*xsize*ysize*3,9999);
    
    if(!water.data)return 1;
    
    water.ysize=ysize;
    water.xsize=xsize;
    
    for(int y=0;y<ysize*2;++y)
    {
        for(int x=0;x<xsize;++x){
            int nn=y*(xsize*3)+x*3;
            water.data[nn]=255;
            water.data[nn+1]=255;
            water.data[nn+2]=255;
        }
    }
    
    return 0;
}

int Audio2::setInfo()
{
    unsigned int samples=48000;
    
    
    if(rx->decodemode == MODE_CW){
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
int Audio2::doFliters(int samplerate)
{
    
/*
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
*/
    return 0;
}
int Audio2::LoadFile(struct Scene *scene,char *filename, int fileType)
{
    char name[512];
    
    if(rx->infile)sf_close(rx->infile) ;
    rx->infile=NULL;
    
    if ((rx->infile = sf_open (filename, SFM_READ, &rx->sfinfo))  == NULL)
    {
        printf ("Not able to open input file %s.\n", filename) ;
        return 1 ;
    }
    
   // sf_command (play.infile , SFC_GET_CURRENT_SF_INFO, &play.sfinfo, sizeof (&play.sfinfo));

    printf("frames %lld samplerate %d channels %d format %x\n",
           (long long)rx->sfinfo.frames,rx->sfinfo.samplerate,
           rx->sfinfo.channels,rx->sfinfo.format);
    
    doFliters(rx->sfinfo.samplerate);
    
    mstrncpy(name,filename,sizeof(name));
    mstrncpy(windowName,filename,sizeof(windowName));
    
    rx->samplerate=rx->sfinfo.samplerate;
    rx->fc=rx->samplerate/2;
    
    //glutSetWindowTitle(filename);

    if(initPlay(rx))return 1;
    
    playFile (rx);

    return 0;
}
int Audio2::initPlay(struct playData4 *play)
{
    play->source=getsourceAudio(audio);
    if(play->source== NO_MORE_SPACE){
        fprintf(stderr,"getsourceAudio out of sources\n");
        return 1;
    }
    
    return 0;
}

void Audio2::playFile (struct playData4 *play)
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
int Audio2::StartIt(struct playData4 *play)
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
int Audio2::setBuffers(struct playData4 *play)
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
    for(unsigned int n=0;n<num;++n){
        buf3[n] = buf1[n];
    }

  //  printf("play->frame %d\n",play->frame);

   updateLine(buf3,num);
    
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


Audio2::~Audio2()
{
    fprintf(stderr,"~Audio2 \n");
    
    stopPlay(rx);
    
    
    fprintf(stderr,"~Audio2 stopPlay\n");
    
    if(rx->infile)sf_close(rx->infile) ;
    rx->infile=NULL;
    
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
    
    
    
    fprintf(stderr,"~Audio2 sceneClean\n");
    
}
int Audio2::fftIndex(double frequency)
{
    return -1;
}

int Audio2::updateLine(short *buf3,unsigned int nc)
{

    double *real,*imag;
    double amin,amax,v;
    
        
    if(rx->FFTcount > FFTlength){
        printf(" FFTlength %ld\n",FFTlength);
        return 1;
    }
    
    int length=rx->FFTcount;
    
    
//    fprintf(stderr,"1 length %d\n",length);
    
    for(int k=0;k<length;++k){
        if(k >= (int)nc){
            rx->reals[k]=0;
            rx->imags[k]=0;
        }else {
            rx->reals[k]=buf3[k];
            rx->imags[k]=0;
        }
    }
    real=rx->reals;
    imag=rx->imags;
    
    doWindow(real,imag,length,rx->FFTfilter);
    
    for(int n=0;n<length;++n){
        real[n] *= pow(-1.0,n);
        imag[n] *= pow(-1.0,n);
    }
    
    doFFT2(real,imag,length,1);
    
    
    amin =  1e33;
    amax = -1e33;

    double rmin=  1e33;
    double rmax= -1e33;
    
    double ddx=(double)rx->samplerate/(double)(length-1);
    for(int n=0;n<length;++n){
        double r;
        r=rx->fc-0.5*rx->samplerate+n*ddx;
        if(r < rmin)rmin=r;
        if(r > rmax)rmax=r;
        range[n]=r;
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        rx->real[length-n-1]=v+rx->scaleFactor;
        if(v < amin)amin=v;
        if(v > amax)amax=v;
    }
    
  //  printf("a amin %g amax %g rmin %g rmax %g\n",amin,amax,rmin,rmax);

    if(water.data == NULL)return 0;
    
    //fprintf(stderr,"2 length %d\n",length);
    
    pd.dmin=amin;
    pd.dmax=amax;
    
    
    //setDialogRange(amin,amax);
        
    amin=water.amin;
    amax=water.amax;
    
    if(water.nline >= water.ysize)water.nline=0;
    
    // fprintf(stderr,"water length %ld\n",length);
    
    unsigned char *wateric=(unsigned char *)dose;
    
    FloatToImage(rx->real,length,&pd,wateric);
    
    int ns1=3*water.xsize*(water.ysize-water.nline-1);
    int ns2=3*water.xsize*water.ysize+3*water.xsize*(water.ysize-1-water.nline++);
        
    
    //fprintf(stderr,"2 length %d\n",length);
        
    double dx=(double)water.xsize/(double)length;

    for(int nnn=length/2;nnn<length;++nnn){
        unsigned char ic;
        
        ic=wateric[nnn];
    

        //ic=nnn/16;
        
        
        int nn=2.0*dx*(nnn-length/2);
        
        //printf("%d %d",ic,nn);

        water.data[ns1+3*nn]=pd.palette[3*ic];
        water.data[ns1+3*nn+1]=pd.palette[3*ic+1];
        water.data[ns1+3*nn+2]=pd.palette[3*ic+2];
        
       // ic=250;

        
        water.data[ns2+3*nn]=pd.palette[3*ic];
        water.data[ns2+3*nn+1]=pd.palette[3*ic+1];
        water.data[ns2+3*nn+2]=pd.palette[3*ic+2];
        
    }
    //printf("\n");

    InvalRectMyWindow(scene);
    
    //fprintf(stderr,"3 length %d\n",length);

    return 0;
}

int Audio2::UpdateTemperatureColors(struct Scene *scene)
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
int Audio2::BackGroundEvents(struct Scene *scene)
{
    if(!backGroundEvents)return 1;
    
    //setDialogFrame((play.frame*play.size));

    //updateLine();
    
    return 0;
}
static int ProcessSound(void *rxv)
{
    Audio2Ptr sdr=(Audio2Ptr)rxv;
    
    struct playData4 *play=sdr->rx;

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
        setFilters(sdr->rx,&sdr->filter);
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
static int AudioReset(struct playData4 *rx,Audio2Ptr sdr)
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

int Audio2::SetFrequency(struct Scene *scene,double f,double bw,int message)
{
    return 0;
}
int Audio2::setFrequency(struct playData4 *play)
{
    return 0;
}
int Audio2::stopPlay(struct playData4 *play)
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
static int DrawString(int x, int y, char *out,int which)
{
    int h;
    
    if(!out)return 1;
    
    glPushMatrix();
    glRasterPos2i( x, y);
    while((h=*out++))
    {
        
        if(which == 0){
           glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, h);
        }else if(which == 1){
           glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, h);
        }else if(which == 2){
          glutBitmapCharacter(GLUT_BITMAP_9_BY_15, h);
        }else if(which == 3){
          glutBitmapCharacter(GLUT_BITMAP_8_BY_13, h);
        }
        
    }
    
    glPopMatrix();
    
    return 0;
}
int Audio2::display(void)
{
    Audio2Ptr s = (Audio2Ptr)this;
    if(!s)return 1;
    
   // fprintf(stderr,"glut %d s->width %d\n",glutGetWindow(),s->width);
        
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //glClearColor(1.0, 0.0, 0.0, 0.0);
    
    glRasterPos2i(0,0);
    
    glColor3f(0.0,0.0,0.0);
    
    unsigned long f,fc;
    
    f=s->rx->f;
    
    fc=s->rx->fc;
    
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, s->width, 0, s->height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        glRasterPos2i(0,0);        /* Position at base of window */
        
        char value[256];
        

        
        msprintf(value,sizeof(value),"F: %010ld Hz CF: %010ld Hz Power: %.0f db",f,fc,0);

        uSetRect(&s->boxFrequency,0,s->height,s->width,28);
        
        //DrawBox(&s->boxFrequency,(int)0);

      //  DrawString(20, (int)s->scene->yResolution-20, value, 0);
        
        uSetRect(&s->xAxis,0,s->height/2,s->width,20);
        
        //DrawBox(&s->xAxis,(int)0);
        
 //       DrawString(20, (int)s->scene->yResolution-s->height/2-15, value, 3);
        
        glColor3f(1.0,0.0,0.0);
        
        uSetRect(&s->yAxis,0,s->height-28,70,s->height/2-27);
  //      DrawBox(&s->yAxis,(int)0);
        
        glColor3f(0.0,0.0,0.0);

        s->drawAxis();

    }
    
    s->Display2(scene);

    glutSwapBuffers();
    
    return 0;
}
static int flip(unsigned char *b,int xsize,int ysize)
{
    unsigned char *p1,*p2,c1,c2,c3;
    int x,y,xinc;
    
    if(!b)return 1;
    
    xinc=xsize*3;
    
    for(y=0;y<ysize/2;++y){
        p1=b+y*xinc;
        p2=b+(ysize-1-y)*xinc;
        for(x=0;x<xsize;++x){
            c1 = *p1;
            c2 = *(p1+1);
            c3 = *(p1+2);
            *p1++ = *(p2);
            *p1++ = *(p2+1);
            *p1++ = *(p2+2);
            *p2++ = c1;
            *p2++ = c2;
            *p2++ = c3;
        }
    }
    
    if(ysize & 1){
        y=ysize/2;
        p1=b+y*xinc;
        for(x=0;x<xsize;++x){
            c1 = *p1;
            c2 = *(p1+1);
            c3 = *(p1+2);
            *p1++ = c1;
            *p1++ = c2;
            *p1++ = c3;
        }
        
    }
    return 0;
}

int Audio2::Display2(struct Scene *scene)
{
    if(!scene)return 1;
    
    //cout<<std::this_thread::get_id()<<endl;
    
    SetWindow(scene);
    
    water.SRect.y=water.ysize+1-water.nline;
    
    // printf("water.nline %d water.ysize %d\n",water.nline,water.ysize);
    
   //  printf("water.xsize %d water.ysize %d xResolution %d yResolution %d\n",(int)water.xsize,(int)water.ysize*2,(int)scene->xResolution,(int)scene->yResolution);
    
 
    WriteRect24ToRect24(water.data, &water.SRect,water.xsize, water.ysize*2,
                        scene->dpp,&water.DRect,scene->xResolution,scene->yResolution/2);
    
    // printf("x %d y %d %d %d\n",water.DRect.x,water.DRect.y,water.DRect.xsize,water.DRect.ysize);
    
    
    flip(scene->dpp,(int)scene->xResolution,(int)scene->yResolution/2);
    
    // glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glRasterPos2i(0,0);        // Position at base of window
    
    if(scene->dpp != NULL){
        glDrawPixels( (int)scene->xResolution, (int)scene->yResolution/2, GL_RGB, GL_UNSIGNED_BYTE, scene->dpp);
    }
    
    /* DrawPalette(scene,(int)scene->xResolution,(int)scene->yResolution,0); */
    
    glRasterPos2i(0,0);        // Position at base of window
    
//    glutSwapBuffers();
    
    return 0;
}
static void palette_select(int item)
{
    
    int window=glutGetWindow();
    Audio2Ptr s = FindAudio2Window(window);
    if(!s){
        fprintf(stderr,"palette_select error window %d\n",window);
        return;
    }

    fprintf(stderr,"item %d\n",item);
    
    if(item == close2){
        if (FindScene(s->scene))closeScene(s->scene);
    }
    if(item == quit)dialogQuitC();

}
static void myReshape(int wscr, int hscr)
{
    int window=glutGetWindow();
    Audio2Ptr s = FindAudio2Window(window);
    if(!s){
        fprintf(stderr,"myReshape error window %d\n",window);
        return;
    }

    s->Reshape(wscr,hscr);
}

static void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small)
{
    
    double xmn = *xmnc,xmx = *xmxc;
    double delx,step;
    int nn;
    long long stepi,min,max;
    double xmin,xmax;
    
    if(!xmnc || !xmxc || !Large || !Small)return;
    
    delx=xmx-xmn;
    
    step=delx/8;
    
    stepi=(long long)step;
    
    nn=0;
    while(stepi >= 10){
        stepi /= 10;
        ++nn;
    }
    
    while(nn-- > 0){
        stepi *= 10;
    }
    
    step=stepi;
    
    //printf("step %g stepi %lld\n",step,stepi);
    
    min=(xmn-step)/step;
    xmin=min*step;
    
    max=(xmx+step)/step;
    xmax=max*step;
    
    //printf("xmin %g xmax %g\n\n",xmin,xmax);
    
    *Large=step;
    
    *Small=step/5;
    
    *xmnc=xmin;
    
    *xmxc=xmax;

}

int Audio2::DrawLine35(int x1, int y1, int x2, int y2,int ic)
{
    
    

    glColor3f(pd.palette[3*ic],pd.palette[3*ic+1],pd.palette[3*ic+2]);

    glLineWidth((GLfloat)1.0);
    glBegin(GL_LINES);
    
    glVertex2f((x1),(y1));
    glVertex2f((x2),(y2));
    
    glEnd();
        
    return 0;
}


int Audio2::drawAxis()
{
    double xmins,xmaxs,Large,Small;
    double xmin,xmax,dx;
    double xmins2,xmaxs2,dxs;
    double fc,samplerate;
    char buff[256];
    
    fc=rx->fc;
    samplerate=rx->samplerate;
    xmin=fc-0.5*samplerate;
    xmax=fc+0.5*samplerate;
    xmins=rx->f-0.5*rx->viewWindow;
    if(xmins < xmin)xmins=xmin;
    xmaxs=rx->f+0.5*rx->viewWindow;
    if(xmaxs > xmaxs)xmins=xmax;
    dx=xmax-xmin;
    dxs=xmaxs-xmins;
    xmins2=xmins;
    xmaxs2=xmaxs;
    GridPlotNeat(&xmins2,&xmaxs2,&Large,&Small);
    xmins=0;
    xmaxs=24000;
    dxs=24000;
    xmins2=0;
    xmaxs2=24000;
    Large=2000;
   // fprintf(stderr,"xmin %f xmax %f xmins %f xmaxs %f Large %g Small %g count %g\n",xmin,xmax,xmins,xmaxs,Large,Small,(xmaxs-xmins)/Large);
    //DrawString(20, (int)scene->yResolution-height/2-15,(char *)"10.0", 3);
    for(double f=xmins2;f<=xmaxs2;f += Large){
        if(f < xmins || f > xmaxs)continue;
        sprintf(buff,"%.0f",f);
        int ii=(f-xmins)*width/dxs;
        //int iio=(f-xmin)*width/dx;
        int i=ii-4.0*strlen(buff);
        if(i < 0)continue;
        DrawLine3(ii, (int)scene->yResolution-height/2+14, ii, height-1);
//        printf("i %d f %f width %d\n",i,f,width);
        DrawString(i, (int)scene->yResolution-height/2+5,(char *)buff, 3);

    }
    
    DrawLine35(0, (int)(height-1), width, (int)(height-1),128);
    DrawLine35(0, (int)(height-100), 600, (int)(height-100),128);
    DrawLine35(0, (int)(height-120), width, (int)(height-120),128);
    DrawLine35(0, (int)(height/2), width, (int)(height/2),128);
    DrawLine35(0, (int)(height/2+16), width, (int)(height/2+16),128);

    
    int length=rx->FFTcount;
    unsigned char *icc=(unsigned char *)dose;
    if(!icc)return 1;
    
    dx=(double)width/(double)length;
    double dy=((double)(height-1)-(height/2+16))/(double)255;
    
    int imin=1000;
    int imax=-1000;
    int xo=0;
    int yo=0;
    for(int n=0;n<length;++n){
        int nn=length/2;
        unsigned char ic=icc[nn+n/2];
        if(ic < imin)imin=ic;
        if(ic > imax)imax=ic;
        int ix=n*dx;
        int iy=height/2+16+ic*dy;
        DrawLine35(xo, yo, ix, iy,1);
        xo=ix;
        yo=iy;
    }
    
                    
   // fprintf(stderr,"length %d imin %d imax %d width %d dy %g\n",length,imin,imax,width,dy);
    
    return 0;
}
int Audio2::Reshape(int wscr, int hscr)
{
    double xmin,ymin,xmax,ymax;
    
    Audio2Ptr s = this;
    if(!s)return 1;
    
   // fprintf(stderr,"Reshape\n");
    
    s->width=wscr; s->height=hscr;
    glViewport(0,0,(GLsizei)s->width,(GLsizei)s->height);

    xmin=ymin=0.0; xmax=ymax=1.0;
    if(s->width<=s->height){
        ymax=s->height*(GLfloat)s->height/(GLfloat)s->width;
        xmax=s->width;
    }else{
        xmax=s->width*(GLfloat)s->width/(GLfloat)s->height;
        ymax=s->height;
    }
    
    gluOrtho2D(xmin,xmax,ymin,ymax);
    glMatrixMode(GL_MODELVIEW);
    
    //glLoadIdentity();
    
    s->scene->xResolution=s->width;
    
    s->scene->yResolution=s->height;
    
    return 0;
}
static void getMousel(int button, int state, int x, int y)
{
    int window=glutGetWindow();
    Audio2Ptr s = FindAudio2Window(window);
    if(!s){
        fprintf(stderr,"getMousel error window %d\n",window);
        return;
    }
    s->getMouse(button,state,x,y);
}
int Audio2::getMouse(int button, int state, int x, int y)
{
    
    uPoint p;
    p.x=x;
    p.y=height-y;
    
    fprintf(stderr,"getMouse button %d state %d x %d y %d\n",button,state,x,y);
    
    if(uPtInRect(&p,&boxFrequency) ){
        printf("in boxFrequency\n");
    }
    
    static double fsave;
    if(state == GLUT_DOWN)
    {
        double fclick;
        if(uPtInRect(&p,&xAxis)){
    
            fclick=rx->fc-0.5*rx->samplerate+x*(double)rx->samplerate/(scene->xResolution);
            
            rx->f=fclick;
            
            fdown=fclick;
            
            fcdown=rx->fc;
            
            fsave=rx->fc;
            
            fcount=0;
        }

    }else{
        //if(fsave != rx->fc) setFrequency2(rx);
    }
    glutPostRedisplay();
    return 0;
}
static void moveMousec(int x, int y)
{
    int window=glutGetWindow();
    Audio2Ptr s = FindAudio2Window(window);
    if(!s){
        fprintf(stderr,"moveMousec error window %d\n",window);
        return;
    }
    s->moveMouse(x,y);
}

int Audio2::moveMouse(int x, int y)
{
    Audio2Ptr s = this;
    if(!s)return 1;
    
    printf("moveMouse x %d y %d\n",x,y);

    uPoint p;
    p.x=x;
    p.y=s->height-y;

    struct Scene *scene;
    
    scene=s->scene;
    
    if(!scene)return 1;
    
    if(uPtInRect(&p,&s->xAxis)){

        double fmove=s->fcdown-0.5*s->rx->samplerate+x*(double)s->rx->samplerate/(scene->xResolution);
        
        double diff=fmove-s->fdown;
        
        s->rx->fc=s->fcdown-diff;
        
         glutPostRedisplay();
        
        printf("fc %g\n",s->rx->fc);
    }
    
    return 0;
}
static void displayc(void)
{
    int window=glutGetWindow();
    Audio2Ptr s = FindAudio2Window(window);
    if(!s){
        fprintf(stderr,"displayc error window %d\n",window);
        return;
    }
    s->display();
}
static void entryFunt(int state)
{
    //fprintf(stderr,"entryFunt state %d\n",state);
    
   int window=glutGetWindow();
    Audio2Ptr s = FindAudio2Window(window);
    if(!s){
        fprintf(stderr,"displayc error window %d\n",window);
        return;
    }
}
static void SelectPalette(int state)
{
    fprintf(stderr,"SelectPalette state %d\n",state);
    
    int window=glutGetWindow();
    Audio2Ptr s = FindAudio2Window(window);
    if(!s){
        fprintf(stderr,"displayc error window %d\n",window);
        return;
    }
}


int Audio2::OpenWindows(struct Scene *scene)
{
    struct SceneList *list;
    
    //struct Scene *sceneOpen=scene;
    
    if(!scene)return 1;
    
    list=SceneFindScene(scene);
    if(list == NULL)
    {
        WarningPrint("Audio2::OpenWindows : Error Could Not Find Scene\n");
        return 1;
    }
    
    scene->ThreadCount=8;
    
    if( scene->stereoType == StereoDoubleBuffered){
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO);
    }else if(scene->stereoType == StereoSingleBuffered){
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH  | GLUT_STEREO);
    }else{
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    }
    glutInitWindowSize(960,340);
    window = glutCreateWindow("Audio2");
    list->window=window;
    scene->window=window;
    fprintf(stderr,"Audio2::OpenWindows window %d\n",window);
    glutReshapeFunc(myReshape);
    glutMouseFunc(getMousel);
    glutDisplayFunc(displayc);
    glutMotionFunc(moveMousec);
    glutEntryFunc(entryFunt);

    int palette_menu=glutCreateMenu(SelectPalette);
    
    glutAddMenuEntry("Apricot Blue->Pink", 0);
    glutAddMenuEntry("Carnation Red->White", 1);
    glutAddMenuEntry("Ether Blue->Yellow", 2);
    glutAddMenuEntry("GrayScale", 3);
    glutAddMenuEntry("GrayScale-Banded", 4);
    glutAddMenuEntry("GrayScale-Inverted", 5);
    glutAddMenuEntry("Hot Metal", 6);
    glutAddMenuEntry("Lava Waves", 7);
    glutAddMenuEntry("Macintosh System Table", 8);
    glutAddMenuEntry("Malachite Green->Blue", 9);
    glutAddMenuEntry("Morning Glory Blue->Tan", 10);
    glutAddMenuEntry("PeanutButter&Jelly", 11);
    glutAddMenuEntry("Ps", 12);
    glutAddMenuEntry("Purple Haze", 13);
    glutAddMenuEntry("Rainbow", 14);
    glutAddMenuEntry("Rainbow-Banded", 15);
    glutAddMenuEntry("Rainbow-High Black", 16);
    glutAddMenuEntry("Rainbow-Inverted", 17);
    glutAddMenuEntry("Rainbow-Low Black", 18);
    glutAddMenuEntry("Rainbow-Striped", 19);
    glutAddMenuEntry("Saturn PastelYellow->Purple", 20);
    glutAddMenuEntry("Seismic Blue->White->Red", 21);
    glutAddMenuEntry("Space Black->Purple->Yellow", 22);
    glutAddMenuEntry("Supernova Green->Purple->Yellow", 23);
    glutAddMenuEntry("System", 24);
    glutAddMenuEntry("Green-White", 27);

    glutCreateMenu(palette_select);
    glutAddMenuEntry("About", 28);
    glutAddSubMenu("Palette", palette_menu);
    glutAddMenuEntry("--------------------", -1);
    glutAddMenuEntry("Close", close2);
    glutAddMenuEntry("Quit", quit);

    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutSetCursor( GLUT_CURSOR_TEXT );

    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glClearColor(1.0,1.0,1.0,0.);
    
    SetLighting(3);
    
    glEnable(GL_LIGHTING);
    
    glShadeModel(GL_SMOOTH);
        
    glutPostRedisplay();

   return 0;
}

int Audio2::resetDemod()
{
    return 0;
}
int Audio2::mMenuSelectl(struct Scene *scene,int item)
{
    if(!scene)return 1;
    
    return 0;
}

int Audio2::FindPoint(struct Scene *scene,int x,int y)
{
    return 0;
}
Audio2Ptr FindAudio2Window(int window)
{
    Audio2Ptr f;
    CWinPtr w;
    
    if(!Root)return NULL;
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeAudio2){
            f=(Audio2Ptr)w;
            if(f->window == window)return f;
        }
        w=w->CNext;
    }
    return NULL;
}
