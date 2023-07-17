//
//  FMRadio.cpp
//  SdrGlut
//
//  Created by Dale on 10/8/16.
//  Copyright © 2016 Dale Ranta. All rights reserved.
//
#include "firstFile.h"
#include <SoapySDR/Version.h>

#include "SdrFile.h"

#include "PlayIQ.h"

#include "mThread.h"

#include <math.h>

#include <GL/glui.h>

#include <string.h>

extern "C" int closeScene(struct Scene *scene);

extern "C" int GridPlotScale(struct uGridPlot *b);

void menu_select(int item);

static void menu_selectl(int item);

static void palette_select2(int item);

static void setMode(int item);

void SetLighting(unsigned int mode);

static void myReshape(int w, int h);

static void reshape(struct Scene *scene,int wscr,int hscr);

static void moveMouse(int x, int y);

void getMouse(int button, int state, int x, int y);

static void keys2(unsigned char key, int x, int y);

void keyss(int key, int x, int y);

void display(void);

extern "C" int DrawLine(int x1, int y1, int x2, int y2);

#define ControlGetSelectionBox	102
#define SdrDialog               103

static int initScene(struct Scene *scene);

extern "C" int doFFT2(double *x,double *y,long length,int direction);

int doWindow(double *x,double *y,long length,int type);

static void getMousel(int button, int state, int x, int y);

static void getMousePassive(int x, int y);

static int DrawString(int x, int y, char *out);

static int DrawBox(uRect *box,int offset);

int doSdrFileOpen(char *name);

static int doRadioOpen2(char *name);

int dialogFileOpen(struct Scene *scene);

ALvoid DisplayALError(unsigned char *szText, ALint errorcode);

static int AudioReset(struct playData4 *play,SdrFilePtr sdr);

static int ProcessSound(void *rxv);

static int freeFilters(struct Filters2 *f);

static int setFilters(struct playData4 *play,struct Filters2 *f);

static void setAudio(int item);

static void doFFTMenu(int item);

static void doFilterMenu(int item);

SdrFile::SdrFile(struct Scene *scene): CWindow(scene)
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
    play.gain=0.25;
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
    
    FileType=FileTypeSDR;

    list=SceneNext();
    if(list == NULL)
    {
        WarningPrint("doRadioOpen : Error Allocation Scene Memory File\n",FileType);
        return 1;
    }
    scene=&list->scene;
    zerol((char *)scene,sizeof(struct Scene));
    SceneInit(scene);
    scene->windowType=FileTypeSDR;

    
    // fprintf(stderr,"doRadioOpen2 name %s\n",name);
    

    SdrFilePtr w = new SdrFile(scene);
    
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
        w->mMenuSelectl(scene,ControlClose);
        return 1;
    }
    
    w->backGroundEvents=1;
    
   return 0;
}

int doSdrFileOpen(char *name)
{
    
    doRadioOpen2(name);
    
    return 0;
}
int SdrFile::setInfo()
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
int SdrFile::LoadFile(struct Scene *scene,char *filename, int fileType)
{
    char name[512];
    
    if(play.infile)fclose8 (play.infile) ;
    play.infile=NULL;
    
    if ((play.infile = fopen8 (filename,(char *)"rb")) == NULL)
    {
        printf ("Not able to open input file %s.\n", filename) ;
        return 1 ;
    }
    
    mstrncpy(name,filename,sizeof(name));
    mstrncpy(windowName,filename,sizeof(windowName));
    
    char *end=strrchr(name,'_');
    if(end){
        play.samplerate=0;
        end--;
        unsigned long num=1;
        while(isdigit(*end)){
            play.samplerate += num*(*end-'0');
            num *= 10;
            --end;
        }
        play.fc=0;
        end--;
        num=1;
        while(isdigit(*end)){
            play.fc += num*(*end-'0');
            num *= 10;
            --end;
        }
        
        play.f=play.fc;

        printf("play.fc %.0f play.samplerate %.0f\n",play.fc,play.samplerate);
        
    }else{
        play.samplerate=500000;
        play.fc=759472;
    }
    
    glutSetWindow(lines2->window);
    glutSetWindowTitle(filename);
    
    glutSetWindow(this->window);
    glutSetWindowTitle(filename);
    

    if(initPlay(&play))return 1;
    
    playFile (&play);

    return 0;
}
int SdrFile::initPlay(struct playData4 *play)
{
    play->source=getsourceAudio(audio);
    if(play->source== NO_MORE_SPACE){
        fprintf(stderr,"getsourceAudio out of sources\n");
        return 1;
    }
    
    return 0;
}

void SdrFile::playFile (struct playData4 *play)
{
    
    // ALint processed;
    // ALint val;
    
    play->controlProcess  = -1;
    
    Sleep2(100);
    
     
    for(int k=0;k<NUM_BUFFERS;++k){
        play->bufferState[k]=0;
    }
    
    setFilters(play, &filter);
    
    setFrequency(play);

    StartIt(play);
    
    play->controlProcess=0;
    
    launchThread((void *)this,ProcessSound);
    
    return ;
}
int SdrFile::StartIt(struct playData4 *play)
{
    ALenum error;

    play->frame = 0;
    
    if(play->infile)fseek8(play->infile, 0, SEEK_SET);

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
int SdrFile::setBuffers(struct playData4 *play)
{
    ALenum error;
    int readcount,k;
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
    
    k=(int)play->size;
    readcount = fget8(play->infile,(char *)buf1, 2*sizeof(float),k);
    
    //printf(" play->frame %d readcount %d ftell8 %lld buffer %d\n", play->frame,readcount,ftell8(play->infile),(int)buffer);
 
    if(k != readcount){
        /*
        if(readcount < 1){
            freebuffAudio(audio,buffer);
            return 1;
        }
        */
        //fprintf(stderr,"readcount %lu requested %lu buffer %d\n",(unsigned long)readcount,(unsigned long)k,buffer);
        for(int n=0;n<k;++n){
            buf1[2*n]=0.0;
            buf1[2*n+1]=0.0;
        }
        ret=1;
    }

    double scale=pow(10.0,play->scaleFactor/20.0);

    for (k = 0 ; k < readcount ; k++){
        float r = (float)(buf1[k * 2]*scale);
        float i = (float)(buf1[k * 2 + 1]*scale);
        buf1[k * 2] = r;
        buf1[k * 2 + 1] = i;
    }

    play->frame++;
    
    
    for(int k=0;k<play->FFTcount;++k){
        if(k < play->size){
            play->real[k]=buf1[2*k];
            play->imag[k]=buf1[2*k+1];
        }else{
            play->real[k]=0;
            play->imag[k]=0;
        }
    }
    
/*
    float shift=play->fc-play->f;
    
    if(play->fShift)nco_crcf_destroy(play->fShift);
    
    play->fShift = nco_crcf_create(LIQUID_NCO);
    
    nco_crcf_set_frequency(play->fShift,(float) ((2.0 * M_PI) * (((double) abs(shift)) / ((double) play->samplerate))));

     if (shift != 0) {
         if (shift > 0) {
             nco_crcf_mix_block_up(play->fShift, (liquid_float_complex *)buf1, (liquid_float_complex *)buf2, play->size);
         } else {
             nco_crcf_mix_block_down(play->fShift, (liquid_float_complex *)buf1, (liquid_float_complex *)buf2,play->size);
         }
     }
*/

    double sint,cost;
    
    for (int k = 0 ; k < play->size ; k++){
        float r = buf1[k * 2];
        float i = buf1[k * 2 + 1];
        //r = 0.001*(rand() % 100);
        //i = 0.001*(rand() % 100);
        if(play->dt > 0){
            buf2[k * 2] = (float)(r*play->coso - i*play->sino);
            buf2[k * 2 + 1] = (float)(i*play->coso + r*play->sino);
            sint=play->sino*play->cosdt+play->coso*play->sindt;
            cost=play->coso*play->cosdt-play->sino*play->sindt;
            play->coso=cost;
            play->sino=sint;
        }else{
            buf2[k * 2] = r;
            buf2[k * 2 + 1] = i;
        }
    }
    
    double rr=sqrt(play->coso*play->coso+play->sino*play->sino);
    play->coso /= rr;
    play->sino /= rr;
    
    unsigned int num;
    unsigned int num2;
    
    num=0;
    num2=0;
    
    msresamp_crcf_execute(filter.iqSampler, (liquid_float_complex *)buf2, play->size, (liquid_float_complex *)buf1, &num);  // decimate

    //fprintf(stderr,"play->size %d num %u play->bw %.0f\n",play->size,num,play->bw);
    
    if(play->decodemode < MODE_AM){
        freqdem_demodulate_block(filter.demod, (liquid_float_complex *)buf1, (int)num, (float *)buf2);
        msresamp_rrrf_execute(filter.iqSampler2, (float *)buf2, num, (float *)buf1, &num2);  // interpolate
        //printf("2 rx->size %d num %u num2 %u\n",rx->size,num,num2);
    }else if(play->decodemode < MODE_USB){
#define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate
        
        for(unsigned int n=0;n<num;++n){
            double mag=sqrt(buf1[2*n]*buf1[2*n]+buf1[2*n+1]*buf1[2*n+1]);
            double z0=mag + (filter.amHistory * DC_ALPHA);
            buf2[n]=(float)(z0-filter.amHistory);
            filter.amHistory=z0;
        }
        msresamp_rrrf_execute(filter.iqSampler2, (float *)buf2, num, (float *)buf1, &num2);  // interpolate
    }else{
        ampmodem_demodulate_block(filter.demodAM,  (liquid_float_complex *)buf1, (int)num, (float *)buf2);
        msresamp_rrrf_execute(filter.iqSampler2, (float *)buf2, num, (float *)buf1, &num2);  // interpolate
    }

    //printf("2 rx->size %d num %u num2 %u\n",play->size,num,num2);

    double dmin,dnom,gain;
    
    double amin=1e30;
    double amax=-1e30;

    gain=play->gain;
    
    if(gain <= 0.0)gain=1.0;
    
    double average=0;
    for (size_t i=0; i<num2; i++ ) {
        double v;
        v=buf1[i];
        average += v;
        if(v < amin)amin=v;
        if(v > amax)amax=v;
    }

    average /= num2;
    
    amin -= average;
    amax -= average;

    if(play->aminGlobal == 0.0)play->aminGlobal=amin;
    play->aminGlobal = 0.8*play->aminGlobal+0.2*amin;
    amin=play->aminGlobal;
    
    if(play->amaxGlobal == 0.0)play->amaxGlobal=amax;
    play->amaxGlobal = 0.8*play->amaxGlobal+0.2*amax;
    amax=play->amaxGlobal;


    if((amax-amin) > 0){
        dnom=65535.0/(amax-amin);
    }else{
        dnom=65535.0;
    }
    
    dmin=amin;
    int short *data=filter.data;
    for(size_t k=0;k<num2;++k){
        double v;
        v=buf1[k];
        v=gain*((v-average)*dnom);
        if(mute)v=0;
        if(v < -32765){
            v = -32765;
        }else if(v > 32765){
            v=32765;
        }
        if(info.Tone){
            double sint=info.sino*info.cosdt+info.coso*info.sindt;
            double cost=info.coso*info.cosdt-info.sino*info.sindt;
            info.coso=cost;
            info.sino=sint;
            v *= cost;
        }
        data[k]=(short int)v;
    }

    if(info.Tone){
        double r=sqrt(info.coso*info.coso+info.sino*info.sino);
        info.coso /= r;
        info.sino /= r;
    }
    
    // fprintf(stderr,"buffer %d used\n",buffer);
/*
    {
        static int count=0;
        static FILE *out=NULL;
        if(out==NULL){
            out=fopen("sound.raw","wb");
        }
        
        fwrite(data, 2, 4800,out);
        if(count++ > 100){
            fclose(out);
            exit(1);
        }
        
    }

*/
    
    alBufferData(buffer,
                 AL_FORMAT_MONO16,
                 data,
                 4800  * sizeof(short),
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


SdrFile::~SdrFile()
{
    fprintf(stderr,"~SdrFile \n");
    
    stopPlay(&play);
    
    if(play.infile)fclose8(play.infile) ;
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
int SdrFile::fftIndex(double frequency)
{
    int index=(int)(0.5+play.FFTcount*((frequency - play.fc)+0.5*play.samplerate)/play.samplerate);
    if(index >= 0 && index < play.FFTcount-1)return index;
    return -1;
}

int SdrFile::updateLine()
{
    
    double *real,*imag;
    double amin,amax,v;
    
    if(rtime() < lineTime)return 0;
    
    lineTime=rtime()+lineDumpInterval;
    
    if(play.FFTcount > FFTlength){
        printf(" FFTcount %d error\n",play.FFTcount);
        return 1;
    }
    
    int length=play.FFTcount;

    for(int k=0;k<length;++k){
        play.reals[k]=play.real[k];
        play.imags[k]=play.imag[k];
    }

    real=play.reals;
    imag=play.imags;
    
    doWindow(real,imag,length,play.FFTfilter);

    for(int n=0;n<length;++n){
        real[n] *= pow(-1.0,n);
        imag[n] *= pow(-1.0,n);
    }
    
    doFFT2(real,imag,length,1);
    
    amin =  0.0;
    int nn=0;
    for(int n=10;n<length-10;++n){
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        double mag=(1.0-lineAlpha)*limag[length-n-1]+v*lineAlpha;
        amin +=  mag;
        ++nn;
    }
    
    amin /= nn;
    
    double shift=-90-amin;
    
    if(play.aminGlobal3 == 0.0)play.aminGlobal3=shift;
    play.aminGlobal3 = 0.9*play.aminGlobal3+0.1*shift;
    //shift=play.aminGlobal3;
    
    //printf("shift %g amin %g ",shift,amin);
    
    amin =  1e33;
    amax = -1e33;
    
    double rmin=  1e33;
    double rmax= -1e33;
    
    float dx=play.samplerate;
    double ddx=(double)play.samplerate/(double)(length);
    long nf=0;
    for(int n=0;n<length;++n){
        double r;
        r=play.fc-0.5*play.samplerate+(n+0.5)*ddx;
        if(r < rmin)rmin=r;
        if(r > rmax)rmax=r;
        range[n]=r;
        if(abs(range[n]-play.f) < dx)
        {
            dx=abs(range[n]-play.f);
            nf=n;
        }
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        limag[length-n-1]=(1.0-lineAlpha)*limag[length-n-1]+v*lineAlpha+shift;
        v=limag[length-n-1];
        lreal[length-n-1]=v+play.scaleFactor;
        if(v < amin)amin=v;
        if(v > amax)amax=v;
    }
    
    if(FindScene(scenel)){
        lines->plotPutData(scenel,range,lreal,length,0L);
        
        uGridPlotPtr Plot;
        Plot=lines->lines->Plot;
        if(!Plot)return 1;
        
        if(!Plot->xManualControl){
            Plot->xAutoMaximum=TRUE;
            Plot->xAutoMinimum=TRUE;
        }
        
        GridPlotScale(Plot);
        
        if(!Plot->xManualControl){
            Plot->xAutoMaximum=FALSE;
            Plot->xAutoMinimum=FALSE;
            Plot->xSetMaximum=rmax;
            Plot->xSetMinimum=rmin;
        }else{
            rmax=Plot->xSetMaximum;
            rmin=Plot->xSetMinimum;
            
        }
    }
    
    long ns,ne,nsub;
    nsub=length/20;
    ns=nf-nsub;
    if(ns < 0)ns=0;
    ne=nf+nsub;
    if(ne >= length){
        ne=length-1-nf+nsub;
    }else{
        ne=2*nsub+1;
    }
    
    
    
    if(FindScene(scenel2))lines2->plotPutData(scenel2,&range[ns],&lreal[ns],ne,0L);
    
    if(water.data == NULL)return 0;
    
    pd.dmin=amin;
    pd.dmax=amax;
    
    
    setDialogRange(amin,amax);
    
    amin=water.amin;
    amax=water.amax;
    
    if(water.nline >= water.ysize)water.nline=0;
    
    long n1=fftIndex(play.f-0.5*play.bw);
    if(n1 < 0)n1=nf-5;
    long n2=fftIndex(play.f+0.5*play.bw);
    if(n2 < 0)n2=nf+5;

    double meterMax=lreal[nf];
    int nmin,nmax;
    nmin=length-1;
    nmax=0;
    for(int n=0;n<length;++n){
        if(range[n] <= rmin)nmin=n;
        if(range[n] <= rmax)nmax=n;
        if(n > n1 && n < n2){
            if(lreal[n] > meterMax)meterMax=lreal[n];
        }
    }
    
    setDialogPower(meterMax);
    
    play.meterMax=meterMax;
    
    
    unsigned char *wateric=(unsigned char *)range;
    
    FloatToImage(lreal,length,&pd,wateric);
    
    int ns1=3*water.xsize*(water.ysize-water.nline-1);
    int ns2=3*water.xsize*water.ysize+3*water.xsize*(water.ysize-1-water.nline++);
    
    
    // fprintf(stderr,"water length %ld\n",length);
    
    double dxn = -1;
    if(nmax-nmin){
        dxn=(double)(nmax-nmin)/(double)(length-1);
    }else{
        nmin=0;
        dxn = 1;
    }
    
    double dxw=(double)(water.xsize-1)/(double)(length-1);
    
    
    int ics=wateric[(int)(2*dxn+nmin)];
    
    for(int nnn=2;nnn<length-2;++nnn){
        int ic;
        
        int n=nnn*dxn+nmin;
        
        int next=(nnn+1)*dxn+nmin;
        
        ic=wateric[n];
        
        int nn=nnn*dxw;
        
        int nn2=next*dxw;
        
        //            fprintf(stderr,"nn %d nn2 %d nnn %d n %d next %d ic %d ics %d\n",nn,nn2,nnn,n,next,ic,ics);
        
        if(ic > ics)ics=ic;
        
        if(nn == nn2)continue;
        
        ic=ics;
        
        ics=wateric[next];
        
        
        water.data[ns1+3*nn]=pd.palette[3*ic];
        water.data[ns1+3*nn+1]=pd.palette[3*ic+1];
        water.data[ns1+3*nn+2]=pd.palette[3*ic+2];
        
        water.data[ns2+3*nn]=pd.palette[3*ic];
        water.data[ns2+3*nn+1]=pd.palette[3*ic+1];
        water.data[ns2+3*nn+2]=pd.palette[3*ic+2];
        
    }
    
    InvalRectMyWindow(scene);
    
    return 0;
}

int SdrFile::UpdateTemperatureColors(struct Scene *scene)
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
    
    float ratio=(float)(rx->fOut / rx->samplerate);
    
    liquid_ampmodem_type mode=LIQUID_AMPMODEM_DSB;
    
    int iflag=0;
    
    if(rx->decodemode == MODE_AM){
        rx->bw=10000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    } else if(rx->decodemode == MODE_NAM){
        rx->bw=5000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    } else if(rx->decodemode == MODE_NBFM){
        rx->bw=12500.0;
    }else if(rx->decodemode == MODE_FM){
        rx->bw=200000.0;
    }else if(rx->decodemode == MODE_USB){   // Above 10 MHZ
        rx->bw=6000.0;
        mode=LIQUID_AMPMODEM_USB;
        iflag=1;
    }else if(rx->decodemode == MODE_LSB){  // Below 10 MHZ
        rx->bw=6000.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }else if(rx->decodemode == MODE_CW){  // Below 10 MHZ
        rx->bw=500.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }
    rx->Ratio = (float)(rx->bw/ rx->samplerate);
    ratio= (float)(48000.0/rx->bw);

    
    f->demod=freqdem_create(0.5);
#ifdef LIQUID_VERSION_4
    f->demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
#else
    f->demodAM = ampmodem_create(0.5, mode, iflag);
#endif

    f->iqSampler  = msresamp_crcf_create(rx->Ratio, As);
    
    f->iqSampler2 = msresamp_rrrf_create(ratio, As);
    
    //msresamp_crcf_print(f->iqSampler);
    
    //rx->lowpass = iirfilt_crcf_create_lowpass(6,0.04);
    //f->lowpass = iirfilt_crcf_create_lowpass(6,0.0625); // +- 3000 HZ
    f->lowpass = iirfilt_crcf_create_lowpass(6,0.104f);    // +- 5000 HZ
    
    
    f->dcFilter = iirfilt_crcf_create_dc_blocker(0.0005f);
    
    
    double rate=rx->samplerate;
    
    int size=(int)(rate/10.0);
    
    rx->size=size;

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
int SdrFile::BackGroundEvents(struct Scene *scene)
{
    if(!backGroundEvents)return 1;
    
    updateLine();
    
    return 0;
}
static int ProcessSound(void *rxv)
{
    SdrFilePtr sdr=(SdrFilePtr)rxv;
    
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
                fprintf(stderr,"stopPlay frame %d - rewind SDR File ftell8 %lld\n",play->frame,ftell8(play->infile));
                play->frame = 0;
                fseek8(play->infile, 0, SEEK_SET);
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
        size_t location=(size_t)(play->setFrameNumber*2*sizeof(float)*play->samplerate/10);
        play->frame = (int)play->setFrameNumber;
        fseek8(play->infile, location, SEEK_SET);
       // fprintf(stderr,"location %lld ret = %d sizeof(long) %ld sizeof(long long) %ld\n",location,ret,sizeof(long),sizeof(long long));
        goto StartUp;
    }
    return 0;
}
static int AudioReset(struct playData4 *rx,SdrFilePtr sdr)
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

int SdrFile::SetFrequency(struct Scene *scene,double f,double bw,int message)
{
    SdrFilePtr sdrRA=(SdrFilePtr)FindScene(scene);
    
    if(!sdrRA)return 1;
    
    if(message == M_MUTE){
        sdrRA->mute = !sdrRA->mute;
        return 0;
    }
    
    sdrRA->play.f=f;
    
    fdown=f;
    
    fcdown=sdrRA->play.fc;
    
    fcount=0;
    
    sdrRA->setFrequency(&sdrRA->play);
    
    return 0;
}


int SdrFile::setFrequency(struct playData4 *play)
{
    
    setDialogFrequency(play->f);
    
    setDialogFc(play->fc);

    {
        float pi;
        pi=4.0*atan(1.0);
        play->dt=1.0/(double)play->samplerate;
        play->sino=0;
        play->coso=1;
        play->w=2.0*pi*(play->fc - play->f);
        play->sindt=sin(play->w*play->dt);
        play->cosdt=cos(play->w*play->dt);
    }
    
    setInfo();
    
    play->aminGlobal=0;
    
    play->amaxGlobal=0;
    
    play->aminGlobal3=0;
    
    play->averageGlobal=0;
    
    if(FindScene(scenel2)){
        SetFrequencyScene(scenel2, play->f, play->bw,M_FREQUENCY_BANDWIDTH);
    }
    
    if(FindScene(scenel)){
        SetFrequencyScene(scenel, play->f,  play->bw, M_FREQUENCY_BANDWIDTH);
    }
    
    return 0;
}
int SdrFile::stopPlay(struct playData4 *play)
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
int SdrFile::SetWindow(struct Scene *scene)
{
    
    CheckSceneBuffer(scene);
    
    int xsize=(int)scene->xResolution;
    
    int ysize=(int)scene->yResolution/2;
    
    water.DRect.x=0;
    water.DRect.y=0;
    water.DRect.xsize=xsize;
    water.DRect.ysize=ysize;
    
    
    //xsize=(int)play.FFTcount;
    
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

int SdrFile::Display(struct Scene *scene)
{
    if(!scene)return 1;
    
    
    SdrFile::SetWindow(scene);
    
    water.SRect.y=water.ysize+1-water.nline;
    
    // printf("water.nline %d water.ysize %d\n",water.nline,water.ysize);
    
    // printf("swater.xs %d swater.ys %d dwater.xs %d dwater.ys %d\n",(int)water.xsize,(int)water.ysize*2,(int)scene->xResolution,(int)scene->yResolution);

    WriteRect24ToRect24(water.data, &water.SRect,water.xsize, water.ysize*2,
                        scene->dpp,&water.DRect,scene->xResolution,scene->yResolution/2);
    
    // printf("x %d y %d %d %d\n",water.DRect.x,water.DRect.y,water.DRect.xsize,water.DRect.ysize);
    
    
    flip(scene->dpp,(int)scene->xResolution,(int)scene->yResolution/2);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glRasterPos2i(0,0);		// Position at base of window
    
    if(scene->dpp != NULL){
        glDrawPixels( (int)scene->xResolution, (int)scene->yResolution/2, GL_RGB, GL_UNSIGNED_BYTE, scene->dpp);
    }
    
    /* DrawPalette(scene,(int)scene->xResolution,(int)scene->yResolution,0); */
    
    glRasterPos2i(0,0);		// Position at base of window
    
    int nf=(int)((scene->xResolution)*((play.f-play.fc)+0.5*play.samplerate)/(double)play.samplerate);
    
    int nf1=(int)((scene->xResolution)*((play.f-play.fc)+0.5*play.samplerate+0.5*play.bw)/(double)play.samplerate);

    int nf2=(int)((scene->xResolution)*((play.f-play.fc)+0.5*play.samplerate-0.5*play.bw)/(double)play.samplerate);
    
    if((nf1-nf2) < 6){
        nf1=nf+3;
        nf2=nf-3;
    }
    
    if(play.wShift > 0){
        nf2=nf;
    }else if(play.wShift < 0){
        nf1=nf;
    }
    

    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, w, 0, h/2);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        glRasterPos2i(0,0);		/* Position at base of window */
        
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
        
        DrawLine(nf1, 0, nf1, (int)scene->yResolution/2);
        
        DrawLine(nf2, 0, nf2, (int)scene->yResolution/2);
        
        glDisable(GL_COLOR_LOGIC_OP);
    
    }

    
    glutSwapBuffers();
    
    return 0;
}

static void myReshape(int w, int h)
{
    struct SceneList *list;
    struct Scene *scene;
    list=SceneFindByNumber(glutGetWindow());
    
    if(!list)return;
    scene=&list->scene;
    
    reshape(scene,w,h);
    
    glutSetWindow(list->window);
}

static void reshape(struct Scene *scene,int wscr,int hscr)
{
    double xmin,ymin,xmax,ymax;
    
    SdrFilePtr s=(SdrFilePtr)FindScene(scene);
    if(!s)return;

    
    s->w=wscr; s->h=hscr;
    glViewport(0,0,(GLsizei)s->w,(GLsizei)s->h);
    
    /* WarningPrint("reshape %d %d\n",w,h); */
    
    // printf("Csdr reshape wscr %d hscr %d\n",wscr,hscr);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    xmin=ymin=0.0; xmax=ymax=1.0;
    if(s->w<=s->h){
        ymax=s->h*(GLfloat)s->h/(GLfloat)s->w;
        xmax=s->w;
    }else{
        xmax=s->w*(GLfloat)s->w/(GLfloat)s->h;
        ymax=s->h;
    }
    
    gluOrtho2D(xmin,xmax,ymin,ymax);
    glMatrixMode(GL_MODELVIEW);
    
    //glLoadIdentity();
    
    scene->xResolution=s->w;
    
    scene->yResolution=s->h;
    
    glutSetWindow(s->window1);
    glutPositionWindow(0,s->h/2);
    glutReshapeWindow(s->w, s->h/2);
    
    glutSetWindow(s->window2);
    glutPositionWindow(0,20);
    glutReshapeWindow(s->w, s->h/2-20);
}

static void displayc(void)
{
    struct SceneList *list;
    struct Scene *scene;
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        return;
    }
    scene=&list->scene;
    
    SdrFilePtr images=(SdrFilePtr)FindScene(scene);
    if(!images)return;
    
    glutSetWindow(images->window1);
    
    images->Display(scene);
    
    glutSetWindow(list->window);
    
    //glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glClearColor(0.0, 0.0, 1.0, 0.0);

    glRasterPos2i(0,0);
    
    glColor3f(1.0,1.0,1.0);
    
    unsigned long f,fc;
    
    f=(unsigned long)images->play.f;
    
    fc=(unsigned long)images->play.fc;
    
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, images->w, 0, images->h);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        glRasterPos2i(0,0);        /* Position at base of window */
        
        char value[256];
        int time=images->play.frame/10;
        int hours=time/3600;
        int min=(time-hours*3600)/60;
        int sec=(time-hours*3600-min*60);
        if(images->mute){
            msprintf(value,sizeof(value),"Frequency: %010ld Hz   Center Frequency: %010ld Hz Power: %.0f db Time: %02d:%02d:%02d MUTE",f,fc,images->play.meterMax,hours,min,sec);
        }else{
            msprintf(value,sizeof(value),"Frequency: %010ld Hz   Center Frequency: %010ld Hz Power: %.0f db Time: %02d:%02d:%02d",f,fc,images->play.meterMax,hours,min,sec);
        }
        DrawString(20, (int)scene->yResolution-15, value);
        
        DrawBox(&images->box,(int)scene->yResolution-20);
        
    }

    glutSwapBuffers();

}
static int DrawBox(uRect *box,int offset)
{
    if(box->xsize <= 0)return 0;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND); //Enable blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function.

    
    glColor4f(1.0, 1.0, 1.0, 0.75);
    glBegin(GL_QUADS);
    glVertex2f(box->x, box->y+offset);
    glVertex2f(box->x+box->xsize, box->y+offset);
    glVertex2f(box->x+box->xsize, box->y-box->ysize+offset);
    glVertex2f(box->x, box->y-box->ysize+offset);
    glEnd();
    
    glDisable(GL_BLEND);
    
    //printf("DrawBox %d %d %d %d\n",box->x,box->x+box->xsize,box->y+offset,box->y-box->ysize+offset);
    
    //DrawLine(box->x, box->y+offset, box->x+box->xsize, box->y-box->ysize+offset);

    
    return 0;
}
int SdrFile::OpenWindows(struct Scene *scene)
{
    struct SceneList *list;
    
    struct Scene *sceneOpen=scene;
    
    if(!scene)return 1;
    
    list=SceneFindScene(scene);
    if(list == NULL)
    {
        WarningPrint("FMRadio::OpenWindows : Error Could Not Find Scene\n");
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
    glutInitWindowSize(1024,400);
    window = glutCreateWindow(scene->WindowTitle);
    list->window=window;
    glutReshapeFunc(myReshape);
    glutKeyboardFunc(keys2);
    glutSpecialFunc( keyss );
    glutMouseFunc(getMousel);
    glutMotionFunc(moveMouse);
    glutPassiveMotionFunc(getMousePassive);
#ifdef USE_JOYSTICK
    glutJoystickFunc(JoyStick, 20);
#endif
    glutDisplayFunc(displayc);
    
    window1 = glutCreateSubWindow(window,0,200,1024,200);
    glutMouseFunc(getMousel);
    glutMotionFunc(moveMouse);
    glutKeyboardFunc(keys2);
    glutDisplayFunc(displayc);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glClearColor(1.0,1.0,1.0,0.);
    
    SetLighting(3);
    
    glEnable(GL_LIGHTING);
    
    glShadeModel(GL_SMOOTH);
    
    int palette_menu=glutCreateMenu(palette_select2);
    
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
    
    
    int menu3=glutCreateMenu(setMode);
    glutAddMenuEntry("AM", MODE_AM);
    glutAddMenuEntry("NAM", MODE_NAM);
    glutAddMenuEntry("FM", MODE_FM);
    glutAddMenuEntry("NBFM", MODE_NBFM);
    glutAddMenuEntry("USB", MODE_USB);
    glutAddMenuEntry("LSB", MODE_LSB);
    glutAddMenuEntry("CW", MODE_CW);

    int menu6=glutCreateMenu(doFilterMenu);
    glutAddMenuEntry("RECTANGULAR", FILTER_RECTANGULAR);
    glutAddMenuEntry("HANN", FILTER_HANN);
    glutAddMenuEntry("HAMMING", FILTER_HAMMING);
    glutAddMenuEntry("FLATTOP", FILTER_FLATTOP);
    glutAddMenuEntry("BLACKMANHARRIS", FILTER_BLACKMANHARRIS);
    glutAddMenuEntry("BLACKMANHARRIS7", FILTER_BLACKMANHARRIS7);
    


    int menu4=glutCreateMenu(setAudio);
    glutAddMenuEntry("IQ Playback", MODE_AM);

    int menu5=glutCreateMenu(doFFTMenu);
    glutAddMenuEntry("1024", FFT_1024);
    glutAddMenuEntry("2048", FFT_2048);
    glutAddMenuEntry("4096", FFT_4096);
    glutAddMenuEntry("8192", FFT_8192);
    glutAddMenuEntry("16384", FFT_16384);
    glutAddMenuEntry("32768", FFT_32768);

    glutCreateMenu(menu_selectl);
    glutAddMenuEntry("Sdr Dialog...", SdrDialog);
   // glutAddMenuEntry("Transmit...", SdrTransmit);
    glutAddSubMenu("Palette", palette_menu);
    glutAddSubMenu("Mode", menu3);
    glutAddSubMenu("IQ Playback", menu4);
    glutAddSubMenu("FFT Size", menu5);
    glutAddSubMenu("Window Filter", menu6);


    glutAddMenuEntry("--------------------", -1);
    glutAddMenuEntry("Close", ControlClose);
    glutAddMenuEntry("Quit", ControlQuit);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    
    list=SceneNext();
    if(list == NULL)
    {
        WarningPrint("FMRadio : Error Allocation Scene Memory File\n");
        return 0;
    }
    scenel=&list->scene;
    zerol((char *)scenel,sizeof(struct Scene));
    
    initScene(scenel);
    
    lines = CLines::CLinesOpen(scenel,window);
    
    lines->plotPutData(scenel,range,dose,FFTlength,-1L);
    
    lines->sceneSource=sceneOpen;
    
    lines->wShift=0;
//    lines->sdr=NULL;

    window2=list->window;
    
    lines->lines->Plot->yLogScale=0;
    
    lines->lines->Plot->gridHash=1;
    lines->lines->Plot->yAutoMaximum=FALSE;
    lines->lines->Plot->yAutoMinimum=FALSE;
    lines->lines->Plot->ySetMaximum=-20;
    lines->lines->Plot->ySetMinimum=-130;
    lines->lines->Plot->yMajorStep=20;

    
    list=SceneNext();
    if(list == NULL)
    {
        WarningPrint("FMRadio : Error Allocation Scene Memory File\n");
        return 0;
    }
    
    scenel2=&list->scene;
    zerol((char *)scenel2,sizeof(struct Scene));
    
    initScene(scenel2);
    
    lines2 = CLines::CLinesOpen(scenel2,-1000);
    
    lines2->plotPutData(scenel2,range,dose,FFTlength,-1L);
    

    lines2->sceneSource=sceneOpen;
    lines2->wShift=0;

    lines2->lines->Plot->yLogScale=0;
    
    lines2->lines->Plot->gridHash=1;
/*
    lines2->lines->Plot->yAutoMaximum=FALSE;
    lines2->lines->Plot->yAutoMinimum=FALSE;
    lines2->lines->Plot->ySetMaximum=-20;
    lines2->lines->Plot->ySetMinimum=-120;
    lines2->lines->Plot->yMajorStep=20;
*/

    glutSetWindow(list->window);
    glutPositionWindow(20,444);
    glutReshapeWindow(600, 200);

    return 0;
}
static void setAudio(int item)
{
    PlayIQ();
}

void doFilterMenu(int item)
{
    int ifft = -1;
    
    switch(item){
        case FILTER_RECTANGULAR:
        case FILTER_HANN:
        case FILTER_HAMMING:
        case FILTER_FLATTOP:
        case FILTER_BLACKMANHARRIS:
        case FILTER_BLACKMANHARRIS7:
            ifft=item;
            break;
    }
    
    if(ifft == -1)return;
    
    struct SceneList *list;
    SdrFilePtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrFileWindow(glutGetWindow());
    }else{
        sdr=(SdrFilePtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    sdr->play.FFTfilter=ifft;
}


void doFFTMenu(int item)
{
    int ifft=0;
    
    switch(item){
        case FFT_1024:
        case FFT_2048:
        case FFT_4096:
        case FFT_8192:
        case FFT_16384:
        case FFT_32768:
            ifft=item;
            break;
    }
    
    if(ifft == 0)return;
    
    struct SceneList *list;
    SdrFilePtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrFileWindow(glutGetWindow());
    }else{
        sdr=(SdrFilePtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    sdr->play.FFTcount=ifft;
}

static void setMode(int item)
{
    struct SceneList *list;
    SdrFilePtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrFileWindow(glutGetWindow());
    }else{
        sdr=(SdrFilePtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;

    sdr->lines->wShift = 0;
    sdr->lines2->wShift = 0;
    sdr->play.wShift = 0;

    switch(item){
        case MODE_AM:
            sdr->play.decodemode = MODE_AM;
            break;
        case MODE_NAM:
            sdr->play.decodemode = MODE_NAM;
            break;
        case MODE_USB:
            sdr->lines->wShift = 1;
            sdr->lines2->wShift = 1;
            sdr->play.wShift = 1;
            sdr->play.decodemode = MODE_USB;
            break;
        case MODE_CW:
            sdr->lines->wShift = -1;
            sdr->lines2->wShift = -1;
            sdr->play.wShift = -1;
            sdr->play.decodemode = MODE_CW;
            break;
        case MODE_LSB:
            sdr->lines->wShift = -1;
            sdr->lines2->wShift = -1;
            sdr->play.wShift = -1;
            sdr->play.decodemode = MODE_LSB;
            break;
        case MODE_FM:
            sdr->play.decodemode = MODE_FM;
            break;
        case MODE_NBFM:
            sdr->play.decodemode = MODE_NBFM;
            break;
        }
        sdr->resetDemod();
}
static void palette_select2(int item)
{
    
    struct SceneList *list;
    SdrFilePtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrFileWindow(glutGetWindow());
    }else{
        sdr=(SdrFilePtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;

    getPalette(item,(char *)g.palname,(char *)g.palette);
    UpdateTemperatureColors(sdr->scene);
    glutPostRedisplay();
    
}

int SdrFile::resetDemod()
{
    
    SdrFilePtr myAppl=this;
/*
    for(int n=0;n<myAppl->FFTlength;++n){
        myAppl->lreal[n]=0;
        myAppl->limag[n]=0;
    }
*/
    for(int y=0;y<myAppl->water.ysize*2;++y){
        
        int ns=3*myAppl->water.xsize*y;
        
        for(int n=0;n<myAppl->water.xsize;++n){
            myAppl->water.data[ns+3*n]=255;
            myAppl->water.data[ns+3*n+1]=255;
            myAppl->water.data[ns+3*n+2]=255;
        }
    }
    
    myAppl->water.nline=0;

    myAppl->play.controlProcess  = -2;
    
    Sleep2(50);

    myAppl->setFrequency(&myAppl->play);
    
    return 0;
}
int SdrFile::mMenuSelectl(struct Scene *scene,int item)
{
    
    if(!scene)return 1;
    
    switch(item)
    {
            
        case SdrDialog:
            dialogSdrFile(scene);
            return 0;
            
        case ControlGetSelectionBox:
            return 0;
            
        case ControlQuit:
            dialogQuitC();
            break;
            
        case ControlClose:
            try{
                backGroundEvents=0;
                play.controlProcess=-1;
                Sleep2(100);
                if (FindScene(scenel))closeScene(lines->scene);
                if (FindScene(scenel2))closeScene(lines2->scene);
                if (FindScene(scene))closeScene(scene);
            }
            catch(...)
            {
                fprintf(stderr,"Exception Closing Scenes\n");
            }
            break;
            
    }
    
    menu_select(item);
    return 0;
}
static void getMousel(int button, int state, int x, int y)
{
    struct SceneList *list;
    SdrFilePtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrFileWindow(glutGetWindow());
        if(sdr){
            sdr->getMouse(button,state, x, y);
            return;
        }
    }
    
    sdr=(SdrFilePtr)FindScene(&list->scene);
    
    if(state == GLUT_DOWN)
    {

        // printf("getMousel %d %d fc %f f %f \n",x,y,sdr->rx.fc,sdr->rx.f);
        
        int up;
        
        if(y > 10){
            up=0;
        }else{
            up=1;
        }
    
        long long fl;
        
        fl=(long long)sdr->play.f;

        int start=119;
        
        for(int k=0;k<10;++k){
            if(x >= start+k*9 && x <= start+(k+1)*9){
                double value=pow(10.0,9-k);
                fl=(long long)fl/value;
                fl=(long long)fl*value;
                if(up == 1){
                    fl += value;
                }else{
                    fl -= value;
               }
                if(fl < 0)fl=-fl;
                sdr->play.f=fl;
                
                if(fabs(fl-sdr->play.fc) > 0.5*sdr->play.samplerate){
                    sdr->play.f=sdr->play.fc;
                }

                sdr->setFrequency(&sdr->play);
                break;
            }
        }
        
    }
}

void SdrFile::getMouse(int button, int state, int x, int y)
{
    
    if(button == 3){
        float fl,bw;
        bw=play.bw*0.5;
        if(play.wShift != 0)bw /= 2;
        fl=play.f-bw;
        if(fl < 0)fl=-fl;
        play.f=fl;
        if(fabs(fl-play.fc) > 0.5*play.samplerate){
            play.fc=fl;
        }
        setFrequency(&play);
        return;
    }else if(button == 4){
        float fl,bw;
        bw=play.bw*0.5;
        if(play.wShift != 0)bw /= 2;
        fl=play.f+bw;
        if(fl < 0)fl=-fl;
        play.f=fl;
        if(fabs(fl-play.fc) > 0.5*play.samplerate){
            play.fc=fl;
        }
        setFrequency(&play);
        return;
    }
    
   	if(state == GLUT_DOWN)
    {
        double fclick;
        
        fclick=play.fc-0.5*play.samplerate+x*(double)play.samplerate/(scene->xResolution);
        
        play.f=fclick;
        
        fdown=fclick;
        
        fcdown=play.fc;
        
        fcount=0;

        setFrequency(&play);
        
        // printf("fclick %f button %d state %d x %d y %d\n",fclick,button,state,x,y);
    }else{
        setFrequency(&play);
    }
}

int SdrFile::FindPoint(struct Scene *scene,int x,int y)
{
    return 0;
}

static void menu_selectl(int item)
{
    struct SceneList *list;
    SdrFilePtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(list == NULL){
        sdr=FindSdrFileWindow(glutGetWindow());
    }else{
        sdr=(SdrFilePtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    sdr->mMenuSelectl(sdr->scene,item);
}

static int initScene(struct Scene *scene)
{
    
    if(!scene)return 1;
    
    SceneInit(scene);
    
    scene->windowType=FileTypeLines;
    
    scene->scale.showPalette=1;
    
    scene->scale.updateTemperatureScale=1;
    
    scene->scale.logscale=1;
        
    return 0;
}
static int DrawString(int x, int y, char *out)
{
    int h;
    
    if(!out)return 1;
    
    glPushMatrix();
    glRasterPos2i( x, y);
    while((h=*out++))
    {
        
        
        //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, h);
        //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, h);
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, h);
        /* glutBitmapCharacter(GLUT_BITMAP_8_BY_13, h); */
        
    }
    
    glPopMatrix();
    
    return 0;
}

static void getMousePassive(int x, int y)
{
    struct SceneList *list;
    SdrFilePtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrFileWindow(glutGetWindow());
    }else{
        sdr=(SdrFilePtr)FindScene(&list->scene);
    }

    if(!sdr)return;
    
    sdr->box.x=0;
    sdr->box.y=0;
    sdr->box.xsize=0;
    sdr->box.ysize=0;
    
    int up;
    
    if(y > 20){
        return;
    }else if(y > 10){
        up=0;
    }else{
        up=1;
    }
    
    
    int start=119;
    
    for(int k=0;k<10;++k){
        if(x >= start+k*9 && x <= start+(k+1)*9){
            if(up == 1){
                //printf("1 getMousePassive %d %d\n",x,y);
                sdr->box.x=start+k*9;
                sdr->box.y=20;
                sdr->box.xsize=9;
                sdr->box.ysize=9;
                
            }else{
                //printf("2 getMousePassive %d %d\n",x,y);
                sdr->box.x=start+k*9;
                sdr->box.y=10;
                sdr->box.xsize=9;
                sdr->box.ysize=9;
            }
        }
    }
}
static void moveMouse(int x, int y)
{
    struct SceneList *list;
    struct Scene *scene;
    SdrFilePtr sdr;
    list=SceneFindByNumber(glutGetWindow());
    sdr=NULL;
    scene=NULL;
    if(!list){
        sdr=FindSdrFileWindow(glutGetWindow());
        scene=sdr->scene;
    }else{
        scene=&list->scene;
        sdr=(SdrFilePtr)FindScene(scene);
        //fprintf(stderr,"moveMouse %d %d \n",x,y);
        return;
    }
    
    if(!sdr)return;
    if(!scene)return;
    
    double fmove=sdr->fcdown-0.5*sdr->play.samplerate+x*(double)sdr->play.samplerate/(scene->xResolution);
    
    sdr->play.f=fmove;
    
    
}
static void keys2(unsigned char key, int x, int y)
{
    struct SceneList *list;
    SdrFilePtr sdr;
    
    // fprintf(stderr,"Sdrfile keys2 %d\n",key);
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrFileWindow(glutGetWindow());
    }else{
        sdr=(SdrFilePtr)FindScene(&list->scene);
    }
    
    if(sdr){
        if(key == 'm'){
            sdr->mute = !sdr->mute;
        }
    }

    // fprintf(stderr,"Sdrfile keys - key %d key %c sdr %p\n",key,key,sdr);

}
