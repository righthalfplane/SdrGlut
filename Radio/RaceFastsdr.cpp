#include "firstFile.h"
#include <SoapySDR/Version.h>
#include "Radio.h"
#include "RaceFastsdr.h"
#include "Utilities.h"
/*
g++ -O2 -Wno-deprecated -o raceFMCsdr raceFMCsdr.cpp mThread.cpp agc.cpp -pthread -lsndfile -framework OpenAL -lSoapySDR -lliquid -Wall

g++ -O2 -Wno-deprecated -o raceFMCsdr raceFMCsdr.cpp mThread.cpp agc.cpp -lopenal -lSoapySDR -lliquid -lpthread -Wall

g++ -O2 -Wno-deprecated -o raceFMCsdr -I"C:\Program Files (x86)\OpenAL 1.1 SDK\include" -I"C:\Program Files\PothosSDR\include" raceFMCsdr.cpp mThread.cpp agc.cpp -lopenal -lSoapySDR -lliquid -lpthread -Wall


./raceFMCsdr -fc 10.1e6 -f 10.0e6 -gain 1

./raceFMCsdr -fc 103.0e6 -f 103.7e6 -fm -gain 1

./raceFMCsdr -fc 162.0e6 -f 162.4e6 -nbfm -gain 1

./raceFMCsdr -fc 9.36e6 -f 9.35e6

*/


static int zerol(unsigned char *s,unsigned long n);

static int startPlay(struct playData *rx);

static int stopPlay(struct playData *rx);

static int doFilter(struct playData *rx,float *wBuff,float *aBuff,struct Filters *f);

static int doMix(struct playData *rx,float *wBuff,float *aBuff,struct Filters *f);

static int doAudio(float *aBuff,struct playData *rx);

static int setFilters(struct playData *rx,struct Filters *f);

static int findRadio(struct playData *rx);

static int playRadio(struct playData *rx);

int rxBuffer(void *rxv);

static int Process(void *rxv);

static int pushBuff(int nbuffer,struct playData *rx);
static int popBuff(struct playData *rx,struct Filters *f);

int AudioReset(struct playData *rx);

static int pushBuffa(int nbuffer,struct playData *rx);
static int popBuffa(struct playData *rx);

static int setBuffers(struct playData *rx, int numBuff);

static int StartIt(struct playData *rx);

static int backgroundPlay(struct playData *rx);

static int ProcessSound(void *rxv);

static int sdrDone(struct playData *rx);

static int sdrSetMode(struct playData *rx);

static int testRadio(struct playData *rx);

static int SetAudio(struct playData *rx,char *name,int type);

static int StartSend(struct playData *rx,char *name,int type);

int freeMemoryRadio(struct playData *rx);

int writeStat(SOCKET toServerSocket,struct playData *rx);

int rxSend(void *rxv);

int RadioStart(int argc, char * argv [],struct playData *rx)
{
    if(!rx)return 0;
    
    rx->pstopPlay=stopPlay;
    rx->pstartPlay=startPlay;
    rx->pplayRadio=playRadio;
    rx->psdrDone=sdrDone;
    rx->psdrSetMode=sdrSetMode;
    rx->pSetAudio=SetAudio;
    rx->pStartSend=StartSend;

    rx->samplerate=2000000;
	rx->deviceNumber=0;
    rx->cutOFF=-70;
    rx->cutOFFSearch=0;
    rx->gain=0.25;
	rx->fc=1.0e6;
	rx->f=0.6e6;
    rx->scaleFactor=0.0;
    rx->audioThreads=0;
    rx->FFTcount=4096;
    rx->FFTfilter=FILTER_BLACKMANHARRIS7;
    rx->controlSend = -1;


	for(int n=1;n<argc;++n){
	    if(!strcmp(argv[n],"-debug")){
		   rx->Debug = 1;
	    }else if(!strcmp(argv[n],"-am")){
		   rx->decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"-fm")){
		   rx->decodemode = MODE_FM;
        }else if(!strcmp(argv[n],"-nbfm")){
            rx->decodemode = MODE_NBFM;
        }else if(!strcmp(argv[n],"-usb")){
            rx->decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"-lsb")){
            rx->decodemode = MODE_LSB;
	    }else if(!strcmp(argv[n],"-gain")){
	         rx->gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-fc")){
	         rx->fc=atof(argv[++n]);
             rx->f=rx->fc;
        }else if(!strcmp(argv[n],"-f")){
            rx->f=atof(argv[++n]);
        }else if(!strcmp(argv[n],"-samplerate")){
            rx->samplerate=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-device")){
	         rx->deviceNumber=atoi(argv[++n]);
	    }else{
			// infilename = argv [n] ;
		}
	}
    
    rx->channel=rx->deviceNumber;

    backgroundPlay(rx);
    
	return 0 ;
} /* main */
static int StartSend(struct playData *rx,char *name,int type)
{
    if(!rx)return 0;

    if(rx->controlSend >= 0){
        printf("Already Running - Cannot Start New Transfer\n");
        return 0;
    }else{
        printf("name %s\n",name);
    }
    
    rx->send=(SOCKET)connectToServer((char *)name,&rx->Port);
    if(rx->send == -1){
        fprintf(stderr,"connect failed\n");
        return 1;
    }
    
    rx->dataType=type;
    rx->controlSend = 0;
    launchThread((void *)rx,rxSend);
    
    return 0;
}
int rxSend(void *rxv)
{
    
    struct playData *rx=(struct playData *)rxv;
    long size=2;
    
    if(!rx)return 0;
    
    rx->save=new saveData;
    
    rx->samplerate_save=-1;
    rx->fc_save=-1;

    int type=rx->dataType;
    while(rx->controlSend >= 0){
       if(rx->controlSend == 1){
            writeStat(rx->send,rx);
           if(type == 0){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*rx->size;++n){
                   double v=rx->sendBuff1[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //printf("r amin %g amax %g ",amin,amax);
               
               average /= 2*rx->size;
               
               amax -= average;
               
               amax -= average;
               
               if(rx->aminGlobal2 == 0.0)rx->aminGlobal2=amin;
               rx->aminGlobal2 = 0.8*rx->aminGlobal2+0.2*amin;
               amin=rx->aminGlobal2;
               
               if(rx->amaxGlobal2 == 0.0)rx->amaxGlobal2=amax;
               rx->amaxGlobal2 = 0.8*rx->amaxGlobal2+0.2*amax;
               amax=rx->amaxGlobal2;
               
               //printf("a amin %g amax %g ",amin,amax);

               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=65534.0/(amax-amin);
               }else{
                   dnom=65534.0;
               }
               
               double gain=0.9;
               
               float *data=(float *)rx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               long int count=0;

               for(int n=0;n<2*rx->size;++n){
                   double v;
                   v=rx->sendBuff1[n];
                   v=gain*((v-average)*dnom);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < -32768){
                       v = -32768;
                       ++count;
                   }else if(v > 32767){
                       v=32767;
                       ++count;
                   }
                   data[n]=(float)v;
               }
               //printf("f amin %g amax %g count %ld\n",amin,amax,count);

               size=(long)(rx->size*sizeof(float));
               if(writeLab(rx->send,(char *)"FLOA",size))return 1;
               if(netWrite(rx->send,(char *)rx->sendBuff2,size))return 1;
               if(writeLab(rx->send,(char *)"FLOA",size))return 1;
               if(netWrite(rx->send,(char *)&rx->sendBuff2[rx->size],size))return 1;
            }else if(type == 1){
                double amin =  1e33;
                double amax = -1e33;
                double average=0;
                for(int n=0;n<2*rx->size;++n){
                    double v=rx->sendBuff1[n];
                    average += v;
                    if(v > amax)amax=v;
                    if(v < amin)amin=v;
                }
                //printf("r amin %g amax %g ",amin,amax);
                
                average /= 2*rx->size;
                
                amax -= average;
                
                amax -= average;
                
                if(rx->aminGlobal2 == 0.0)rx->aminGlobal2=amin;
                rx->aminGlobal2 = 0.8*rx->aminGlobal2+0.2*amin;
                amin=rx->aminGlobal2;
                
                if(rx->amaxGlobal2 == 0.0)rx->amaxGlobal2=amax;
                rx->amaxGlobal2 = 0.8*rx->amaxGlobal2+0.2*amax;
                amax=rx->amaxGlobal2;
                
                //printf("a amin %g amax %g ",amin,amax);
                
                double dnom=0.0;
                if((amax-amin) > 0){
                    dnom=65534.0/(amax-amin);
                }else{
                    dnom=65534.0;
                }
                
                double gain=0.9;
                
                short int *data=(short int *)rx->sendBuff2;
                
                amin =  1e33;
                amax = -1e33;
                
                long int count=0;

                for(int n=0;n<2*rx->size;++n){
                    double v;
                    v=rx->sendBuff1[n];
                    v=gain*((v-average)*dnom);
                    if(v < amin)amin=v;
                    if(v > amax)amax=v;
                    if(v < -32768){
                        v = -32768;
                        ++count;
                    }else if(v > 32767){
                        v=32767;
                        ++count;
                   }
                    data[n]=(short int)v;
                }
               // printf(" f amin %g amax %g count %ld\n",amin,amax,count);
                size=rx->size*sizeof(short int);
                if(writeLab(rx->send,(char *)"SHOR",size))return 1;
                if(netWrite(rx->send,(char *)rx->sendBuff2,size))return 1;
                if(writeLab(rx->send,(char *)"SHOR",size))return 1;
                if(netWrite(rx->send,(char *)&data[rx->size],size))return 1;
           }else if(type == 2){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*rx->size;++n){
                   double v=rx->sendBuff1[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //printf("r amin %g amax %g ",amin,amax);
               
               average /= 2*rx->size;
               
               amax -= average;
               
               amax -= average;
               
               if(rx->aminGlobal2 == 0.0)rx->aminGlobal2=amin;
               rx->aminGlobal2 = 0.8*rx->aminGlobal2+0.2*amin;
               amin=rx->aminGlobal2;
               
               if(rx->amaxGlobal2 == 0.0)rx->amaxGlobal2=amax;
               rx->amaxGlobal2 = 0.8*rx->amaxGlobal2+0.2*amax;
               amax=rx->amaxGlobal2;

               //printf("a amin %g amax %g ",amin,amax);
               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=255.0/(amax-amin);
               }else{
                   dnom=255.0;
               }
               
               double gain=0.9;
               
               signed char *data=(signed char *)rx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               long int count=0;

               for(int n=0;n<2*rx->size;++n){
                   double v;
                   v=rx->sendBuff1[n];
                   v=gain*((v-average)*dnom);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < -128){
                       ++count;
                       v = -128;
                   }else if(v > 127){
                       v=127;
                       ++count;
                 }
                   data[n]=(signed char)v;
               }
               //printf(" f amin %g amax %g count %ld\n",amin,amax,count);
               size=rx->size*sizeof(signed char);
               if(writeLab(rx->send,(char *)"SIGN",size))return 1;
               if(netWrite(rx->send,(char *)rx->sendBuff2,size))return 1;
               if(writeLab(rx->send,(char *)"SIGN",size))return 1;
               if(netWrite(rx->send,(char *)&data[rx->size],size))return 1;
               
           }else if(type == 3){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*rx->size;++n){
                   double v=rx->sendBuff1[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //printf("r amin %g amax %g ",amin,amax);
               
               average /= 2*rx->size;
               
               amax -= average;
               
               amax -= average;
               
               if(rx->aminGlobal2 == 0.0)rx->aminGlobal2=amin;
               rx->aminGlobal2 = 0.8*rx->aminGlobal2+0.2*amin;
               amin=rx->aminGlobal2;
               
               if(rx->amaxGlobal2 == 0.0)rx->amaxGlobal2=amax;
               rx->amaxGlobal2 = 0.8*rx->amaxGlobal2+0.2*amax;
               amax=rx->amaxGlobal2;
               //printf("a a amin %g amax %g ",amin,amax);

               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=255.0/(amax-amin);
               }else{
                   dnom=255.0;
               }
                              
               double gain=0.9;
               
               unsigned char *data=(unsigned char *)rx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               
               long int count=0;
               
               for(int n=0;n<2*rx->size;++n){
                   double v;
                   v=rx->sendBuff1[n];
                   v=gain*((v-average)*dnom+141.0);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < 0){
                       v = 0;
                       ++count;
                   }else if(v > 255){
                       v=255;
                       ++count;
                  }
                   data[n]=(unsigned char)v;
               }
              // printf("f  amin %g amax %g count %ld\n",amin,amax,count);
               size=rx->size*sizeof(unsigned char);
               if(writeLab(rx->send,(char *)"USIG",size))return 1;
               if(netWrite(rx->send,(char *)rx->sendBuff2,size))return 1;
               if(writeLab(rx->send,(char *)"USIG",size))return 1;
               if(netWrite(rx->send,(char *)&data[rx->size],size))return 1;
           }
           rx->controlSend = 0;
        }else{
            Sleep2(20);
        }
    }
  

    if(rx->send >= 0){
        doEnd(rx->send);
        shutdown(rx->send,2);
        closesocket(rx->send);
    }
    
    delete rx->save;

    return 0;
}
int writeStat(SOCKET toServerSocket,struct playData *rx)
{
    double buff[2];
    
    if(!rx)return 0;
    
    if((rx->save->samplerate != rx->samplerate) || (rx->save->fc != rx->fc)){
        rx->save->fc=rx->fc;
        rx->save->samplerate=rx->samplerate;
    
        buff[0]=rx->fc;
        buff[1]=rx->samplerate;
    
        long size=(long)2*sizeof(double);
    
        if(writeLab(toServerSocket,(char *)"STAT",size))return 1;
    
        if(netWrite(toServerSocket,(char *)buff,size))return 1;
    
    }
    
    if(rx->frequencyFlag && (rx->save->f != rx->f)){
        rx->save->f = rx->f;
        buff[0]=rx->f;
        long size=(long)sizeof(double);
        if(writeLab(toServerSocket,(char *)"F   ",size))return 1;
        if(netWrite(toServerSocket,(char *)buff,size))return 1;
    }
    
    if(rx->demodulationFlag && (rx->save->decodemode != rx->decodemode)){
        rx->save->decodemode = rx->decodemode;
        buff[0]=rx->decodemode;
        long size=(long)sizeof(double);
        if(writeLab(toServerSocket,(char *)"DECO",size))return 1;
        if(netWrite(toServerSocket,(char *)buff,size))return 1;

    }
    
    return 0;
}

static int SetAudio(struct playData *rx,char *name,int type)
{
    if(!rx)return 0;
    
    if(type == START_AUDiO){
        if(rx->audioOutput)fclose(rx->audioOutput);
        rx->audioOutput=NULL;
        if(name){
            rx->audioOutput=fopen(name,"wb");
            if(!rx->audioOutput){
                fprintf(stderr,"Error Opening %s To Write\n",name);
            }
        }
    }else{
/*
        FILE *out;
        out=(FILE *)rx->iqOutput;
        if(out){
            rx->iqOutput=NULL;
            Sleep2(100);
            fclose(out);
        }
        rx->iqOutput=NULL;
        if(name){
            rx->iqOutput=fopen(name,"wb");
            if(!rx->iqOutput){
                fprintf(stderr,"Error Opening %s To Write\n",name);
            }
        }
 */
    }

    
    return 0;
}

static int backgroundPlay(struct playData *rx)
{
    if(!rx)return 0;
    
    rx->fOut=48000;
    
    startPlay(rx);
    
    playRadio(rx);
    
    return 0;
}
static int startPlay(struct playData *rx)
{
    if(!rx)return 0;
    
    rx->controlRF=0;
    
    rx->witchRFBuffer=0;
    
    rx->witchAudioBuffer=0;
    
    rx->source=getsourceAudio(audio);
    if(rx->source== NO_MORE_SPACE){
        fprintf(stderr,"getsourceAudio out of sources\n");
        return 1;
    }
    
    {
        double pi;
        pi=4.0*atan(1.0);
        rx->dt=1.0/(double)rx->samplerate;
        rx->sino=0;
        rx->coso=1;
        rx->w=2.0*pi*(rx->fc - rx->f);
        rx->sindt=sin(rx->w*rx->dt);
        rx->cosdt=cos(rx->w*rx->dt);
        // printf("fc %f f %f dt %g samplerate %.0f\n",rx->fc,rx->f,rx->dt,rx->samplerate);
    }
    
    
    if(findRadio(rx) || rx->device == NULL){
        fprintf(stderr,"Error No SDR Found\n");
        return 1;
    }
    
    // fprintf(stderr,"startPlay done\n");
    
    return 0;
}
static int playRadio(struct playData *rx)
{
    if(!rx)return 0;
    
    rx->controlRF =  1;
    rx->controlAudio  = -1;
    rx->controlProcess  = -1;

    Sleep2(100);
    
    //double rate=rx->samplerate;
    
    double rate=rx->device->getSampleRate(SOAPY_SDR_RX, rx->channel);
    
    // SoapySDRDevice_setSampleRate(rx->device,SOAPY_SDR_RX, rx->channel, rate);
    
    
    fprintf(stderr,"playRadio frequency %g channel %d\n",rx->fc,rx->channel);
    rx->device->setFrequency(SOAPY_SDR_RX, rx->channel, rx->fc-rx->foffset);
    
    if(rx->bandwidth > 0){
        rx->device->setBandwidth( SOAPY_SDR_RX, rx->channel, rx->bandwidth);
        //if(retv > 0)printf("SoapySDRDevice_setBandwidth returned %d\n",retv);
    }
    
    double bw=rx->device->getBandwidth(SOAPY_SDR_RX, rx->channel);
    int size=(int)(0.5+rate/10.0);
    
    rx->size=size;
    
    printf("Device %s samplerate %.0f rx->size %d Bandwidth %.0f\n",rx->driveName,rate,rx->size,bw);
    
    size += 256;  // bug in rfspace NetSDR
    
    if(rx->sendBuff1)cFree((char *)rx->sendBuff1);
    rx->sendBuff1=(float *)cMalloc(2*size*sizeof(float),18887);
    if(rx->sendBuff2)cFree((char *)rx->sendBuff2);
    rx->sendBuff2=(float *)cMalloc(2*size*sizeof(float),18888);
    
    for(int k=0;k<NUM_DATA_BUFF5;++k){
        if(rx->buff[k])cFree((char *)rx->buff[k]);
        rx->buff[k]=(float *)cMalloc(2*size*sizeof(float),8887);
        if(!rx->buff[k]){
            fprintf(stderr,"1 cMalloc Errror %ld\n",(long)(2*size*sizeof(float)));
            return 1;
        }
        zerol((unsigned char *)rx->buff[k],2*size*sizeof(float));
        rx->buffStack[k]=-1;
    }
    
    for(int k=0;k<NUM_ABUFF5;++k){
        if(rx->buffa[k])cFree((char *)rx->buffa[k]);
        rx->buffa[k]=(short int *)cMalloc(2*rx->fOut*4,8888);
        if(!rx->buffa[k]){
            fprintf(stderr,"1 cMalloc Errror %ld\n",(long)(2*rx->fOut*4));
            return 1;
        }
        zerol((unsigned char *)rx->buffa[k],2*rx->fOut*4);
        rx->buffStacka[k]=-1;
    }

    rx->controlRF=2;
    
    rx->witchRFBuffer=0;
    
    rx->controlAudio=0;
    
    rx->controlProcess=0;

    launchThread((void *)rx,rxBuffer);
    
    launchThread((void *)rx,Process);
    for(int n=0;n<rx->audioThreads;++n){
        launchThread((void *)rx,Process);
    }
        
    Sleep2(100);
    
    StartIt(rx);
    
    launchThread((void *)rx,ProcessSound);
        
    return 0;
}
static int StartIt(struct playData *rx)
{
    ALenum error;
    
    //double start=rtime();
    
    if(!rx)return 0;
    
    for(int i=0;i<3;){
        int ibuff;
        ibuff=popBuffa(rx);
        if (ibuff >= 0){
            ++i;
            if(setBuffers(rx, ibuff)){
                ;
            }
        }
    }
    //double end=rtime();
    
    //fprintf(stderr,"time = %g\n",end-start);

    //exit(0);
    
    alSourcePlay(rx->source);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError((unsigned char *)"StartIt alSourcePlay : ", error);
    }
    
    return 0;
}

static int ProcessSound(void *rxv)
{
    struct playData *rx=(struct playData *)rxv;
    
    if(!rx)return 0;
    
    ALenum error;
    
    ALint processed;
    
    unsigned long count=0;
    
    double start=rtime();
    
    double end=rtime();
    
    fprintf(stderr,"ProcessSound start\n");
    
    int ibuff;
    ibuff=3;
    while(rx->controlProcess >= 0){
        Sleep2(20);
        {
            processed=0;
            
            alGetSourcei(rx->source, AL_BUFFERS_PROCESSED, &processed);
            
            if(processed){
                ALuint *fbuff= new ALuint[audio->numbuff];
                
                alSourceUnqueueBuffers(rx->source, processed, fbuff);
                if ((error = alGetError()) != AL_NO_ERROR)
                {
                    fprintf(stderr,"Device %s Error in ",rx->driveName);
                    DisplayALError((unsigned char *)"ProcessSound alSourceUnqueueBuffers : ", error);
                }
                
                // if(processed > 1)printf("%s processed %d count %lu\n",rx->driveName,processed,count);
                
                for(ALint k=0;k<processed;++k){
                    //fprintf(stderr,"finish bufferd %d\n",fbuff[k]);
                    freebuffAudio(audio,fbuff[k]);
                    
                    while((ibuff=popBuffa(rx)) < 0)Sleep2(10);
                    
                    if(setBuffers(rx, ibuff)){
                        ;
                    }
                    
                    if(count % 5){
                        alGetSourcei(rx->source, AL_SOURCE_STATE, &rx->al_state);
                        if(rx->al_state == AL_STOPPED){
                            alSourcePlay(rx->source);
                            printf("%s count %lu sound restarted\n",rx->driveName,count);
                        }
                    }

                    
                    if(!(++count % 100)){
                        end=rtime();
                        /*
                        printf("%s count %lu time %.2f ",rx->driveName,count,end-start);
                        alGetSourcei(rx->source, AL_SOURCE_STATE, &rx->al_state);
                        fprintf(stderr,"al state %d %x AL_STOPPED %d AL_INITIAL %d\n",rx->al_state,rx->al_state,AL_STOPPED,AL_INITIAL);
                        
                        if(rx->al_state == AL_STOPPED){
                            alSourcePlay(rx->source);
                        }
                        
                        if ((error = alGetError()) != AL_NO_ERROR){
                            fprintf(stderr,"Device %s count %lu ",rx->driveName,count);
                            DisplayALError((unsigned char *)"in ProcessSound alBufferData 1 : ", error);
                        }
                         */
                        
                        start=end;
                    }
                    
                }
                
                
                
            }else{
                alGetSourcei(rx->source, AL_SOURCE_STATE, &rx->al_state);
                //fprintf(stderr,"count %lu al state %d %x AL_STOPPED %d AL_INITIAL %d\n",count,rx->al_state,rx->al_state,AL_STOPPED,AL_INITIAL);
                if(rx->al_state != AL_PLAYING && rx->al_state != AL_INITIAL){
                    if(rx->al_state == AL_STOPPED){
                        AudioReset(rx);
                    }else{
                        printf("al state %d %x\n",rx->al_state,rx->al_state);
                    }
                }
                if(rx->al_state == AL_INITIAL){
                    StartIt(rx);
                }
            }
            
        }
        
    }
    
    rx->controlRF=1;
    
    int count2=0;
    while(rx->controlRF == 1){
        Sleep2(10);
        if(++count2 > 200)break;
    }
    
    rx->controlRF=1;

    rx->controlAudio=-1;

    Sleep2(100);
    
    
    rx->controlProcess = -2;

    fprintf(stderr,"ProcessSound end\n");
    
    return 0;
}

static int stopPlay(struct playData *rx)
{
    if(!rx)return 0;
    
    if(rx->controlProcess >= 0){

        rx->controlProcess = -1;
        
        int count=0;
        while(rx->controlProcess == -1){
            Sleep2(10);
            if(++count > 200)break;
        }
        
    }
    
    rx->controlProcess = -1;
    
    if(rx->controlAudio  != -1)
    {
        rx->controlAudio = -1;
        Sleep2(100);
        fprintf(stderr,"stopPlay ControlAudio Sleep\n");
    }
    if(rx->controlRF !=  1)
    {
        rx->controlRF = 1;
        Sleep2(100);
        fprintf(stderr,"stopPlay controlRF Sleep\n");
    }
    
    
    
    ALint processed;
    ALenum error;
    
    alSourceStopv(1, &rx->source);
    
    alGetSourcei(rx->source, AL_SOURCE_STATE, &rx->al_state);
    fprintf(stderr,"stopPlay source %d al state %d %x\n",rx->source,rx->al_state,rx->al_state);
    
    processed=0;
    alGetSourcei(rx->source, AL_BUFFERS_PROCESSED, &processed);
    
    while(processed){
        ALuint fbuff;
        alSourceUnqueueBuffers(rx->source, 1, &fbuff);
        if ((error = alGetError()) != AL_NO_ERROR)
        {
            fprintf(stderr,"Device %s Error in ",rx->driveName);
            DisplayALError((unsigned char *)"stopPlay alSourceUnqueueBuffers : ", error);
        }
        freebuffAudio(audio,fbuff);
        --processed;
    }
    
    freesourceAudio(audio,rx->source);
    
    freeMemoryRadio(rx);
    
    if(rx->device){
        
        rx->device->deactivateStream(rx->rxStream, 0, 100000L);
        
        rx->device->closeStream(rx->rxStream);
        
        if(rx->channel == 0){
            SoapySDR::Device::unmake(rx->device);
            rx->device=NULL;
        }
        
        rx->device=NULL;
        
        rx->rxStream=NULL;
    }
    
    
    // fprintf(stderr,"Out Of stopPlay\n");
    
    return 0;
}

static int sdrSetMode(struct playData *rx)
{
    if(!rx)return 0;
    
    rx->controlAudio = -1;
    
    Sleep2(100);

    rx->controlAudio = 0;

    launchThread((void *)rx,Process);
    
    return 0;
}
int freeMemoryRadio(struct playData *rx)
{
    if(!rx)return 0;
    
    if(rx->antenna){
        for (size_t i=0;i<rx->antennaCount;++i){
            cFree((char *)rx->antenna[i]);
        }
        cFree((char *)rx->antenna);
        rx->antenna=NULL;
    }
    
    if(rx->gains){
        for (size_t j = 0; j < rx->gainsCount; j++)
        {
            cFree((char *)rx->gains[j]);
        }
        cFree((char *)rx->gains);
        rx->gains=NULL;
    }
    
    if(rx->streamFormat){
        for (size_t j = 0; j < rx->streamFormatCount; j++)
        {
            cFree((char *)rx->streamFormat[j]);
        }
    }
    
    if(rx->gainsMinimum)cFree((char *)rx->gainsMinimum);
    rx->gainsMinimum=NULL;
    if(rx->gainsMaximum)cFree((char *)rx->gainsMaximum);
    rx->gainsMaximum=NULL;
    if(rx->frequencyMinimum)cFree((char *)rx->frequencyMinimum);
    rx->frequencyMinimum=NULL;
    if(rx->frequencyMaximum)cFree((char *)rx->frequencyMaximum);
    rx->frequencyMaximum=NULL;
    if(rx->bandwidths)cFree((char *)rx->bandwidths);
    rx->bandwidths=NULL;
    if(rx->streamFormat)cFree((char *)rx->streamFormat);
    rx->streamFormat=NULL;
    if(rx->sampleRates)cFree((char *)rx->sampleRates);
    rx->sampleRates=NULL;
    return 0;
}
static int sdrDone(struct playData *rx)
{
    if(!rx)return 0;
    
    RadioPtr r=FindSdrRadioWindow(rx);

    r->stopPlay(rx);
    
    for(int k=0;k<NUM_DATA_BUFF5;++k){
        if(rx->buff[k])cFree((char *)rx->buff[k]);
        rx->buff[k]=NULL;
    }
    
    for(int k=0;k<NUM_ABUFF5;++k){
        if(rx->buffa[k])cFree((char *)rx->buffa[k]);
        rx->buffa[k]=NULL;
    }

    
    return 0;

}
static int setBuffers(struct playData *rx, int numBuff)
{
    ALenum error;
    ALuint buffer;
    
    if(!rx)return 0;
    
    buffer=getbuffAudio(audio);
    if(buffer == NO_MORE_SPACE){
        fprintf(stderr,"getbuffAudio out of buffers\n");
        return 1;
    }

    if(rx->audioOutput && !rx->muteScan){
        size_t ret=fwrite(rx->buffa[numBuff % NUM_ABUFF5], 2, 4800,rx->audioOutput);
        if(ret == 0){
            ;
        }
    }
    
    int mute=rx->mute+rx->muteScan;
    if(mute)zerol((unsigned char *)rx->buffa[numBuff % NUM_ABUFF5],(long)(4800*sizeof(short)));

    alBufferData(buffer,
                 AL_FORMAT_MONO16,
                 rx->buffa[numBuff % NUM_ABUFF5],
                 4800  * sizeof(short),
                 48000);
    
    if ((error = alGetError()) != AL_NO_ERROR){
        fprintf(stderr,"Device %s Error on buffer # %d ",rx->driveName,buffer);
        DisplayALError((unsigned char *)"in SetBuffers alBufferData 1 : ", error);
        return 1;
    }
    
    alSourceQueueBuffers(rx->source, 1, &buffer);
    
    if ((error = alGetError()) != AL_NO_ERROR){
        fprintf(stderr,"Device %s Error on buffer # %d ",rx->driveName,buffer);
        DisplayALError((unsigned char *)"in SetBuffers alSourceQueueBuffers 1 : ", error);
        return 1;
    }
    

	return 0;
}

static int Process(void *rxv)
{
	struct playData *rx=(struct playData *)rxv;
	
	struct Filters f;
    
	zerol((unsigned char *)&f,sizeof(f));
    
    if(!rx)return 0;
    
    f.thread=rx->threadNumber++;
	
	setFilters(rx,&f);
    
    int bsize=2*rx->size*sizeof(float);
    if(bsize < 200000)bsize=200000;
	
	float *wBuff=(float *)cMalloc(bsize,8889);
    if(!wBuff){
        fprintf(stderr,"2 cMalloc Errror %ld\n",(long)(bsize));
       	 return 1;
    }
    zerol((unsigned char *)wBuff,bsize);

   // printf("Process Start rx->frame %d Thread %d\n",rx->controlAudio,f.thread);
	
	float *aBuff=(float *)cMalloc(2*rx->fOut*4,8890);
    if(!aBuff){
        fprintf(stderr,"3 cMalloc Errror %ld\n",(long)(2*rx->fOut*4));
       	 return 1;
    }
    zerol((unsigned char *)aBuff,2*rx->fOut*4);
	
	while(rx->controlAudio >= 0){
		if(doFilter(rx,wBuff,aBuff,&f)){
			Sleep2(40);
		}else{
			doAudio(aBuff,rx);
		}
	}
	//printf("Process Done rx->frame %d Thread %d\n",rx->controlAudio,f.thread);
	
	if(wBuff)cFree((char *)wBuff);
	
	if(aBuff)cFree((char *)aBuff);
	
	if (f.iqSampler)msresamp_crcf_destroy(f.iqSampler);
	
	if (f.iqSampler2)msresamp_rrrf_destroy(f.iqSampler2);
	
	if(f.lowpass)iirfilt_crcf_destroy(f.lowpass);
	
    if(f.dcFilter)iirfilt_crcf_destroy(f.dcFilter);
	
    if(f.fShift)nco_crcf_destroy(f.fShift);
    
    if(f.demod)freqdem_destroy(f.demod);
    
    if(f.demodAM)ampmodem_destroy(f.demodAM);
    
	return 0;
}

int AudioReset(struct playData *rx)
{
    
	ALint processed;
	ALenum error;

    if(!rx)return 0;
    
	alSourceStopv(1, &rx->source);

	alGetSourcei(rx->source, AL_SOURCE_STATE, &rx->al_state);
    
	fprintf(stderr, "Device %s AudioReset source %d al state %d %x\n",rx->driveName, rx->source, rx->al_state, rx->al_state);

	processed = 0;
	alGetSourcei(rx->source, AL_BUFFERS_PROCESSED, &processed);

	while (processed) {
		ALuint fbuff;
		alSourceUnqueueBuffers(rx->source, 1, &fbuff);
		if ((error = alGetError()) != AL_NO_ERROR)
		{
            fprintf(stderr,"Device %s Error in ",rx->driveName);
			DisplayALError((unsigned char *)"AudioReset alSourceUnqueueBuffers : ", error);
		}
		freebuffAudio(audio, fbuff);
		fprintf(stderr, "Device %s AudioReset free buffer %d\n",rx->driveName, fbuff);
		--processed;
	}

    int ibuff;

   while((ibuff=popBuffa(rx)) < 0)Sleep2(20);

    if (ibuff >= 0){
        if (setBuffers(rx, ibuff)) {
            fprintf(stderr, "Device %s AudioReset setBuffers failed\n",rx->driveName);
        }
    }
	alSourcePlay(rx->source);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
        fprintf(stderr,"Device %s Error in ",rx->driveName);
		DisplayALError((unsigned char *)"AudioReset alSourcePlay : ", error);
	}

    alGetSourcei(rx->source, AL_SOURCE_STATE, &rx->al_state);
    
    fprintf(stderr,"Device %s AudioReset source %d al state %d %x\n",rx->driveName,rx->source,rx->al_state,rx->al_state);

    return 0;
}
static int setFilters(struct playData *rx,struct Filters *f)
{
    
    // double shift=rx->f-rx->fc;
    
    if(!rx)return 0;
    
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
        rx->bw=3000.0;
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
    
    f->fShift = nco_crcf_create(LIQUID_NCO);
    
    f->amHistory=0;
    
    return 0;
    
}

static int doFilter(struct playData *rx,float *wBuff,float *aBuff,struct Filters *f)
{
    if(!rx)return 0;
    
 	int ip=popBuff(rx,f);
 	if(ip < 0){
 	     return 1;
 	}
 	 	
 	int witch=ip % NUM_DATA_BUFF5;
 	
 	//fprintf(stderr,"doFilter witch %d ip %d start \n",witch,ip);
	
    float *buf=rx->buff[witch];
    float *buf2=wBuff;
    
    doMix(rx,buf,buf2,f);
    
	buf=aBuff;
		
    unsigned int num;
    unsigned int num2;
    
    num=0;
    num2=0;
    
    msresamp_crcf_execute(f->iqSampler, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf, &num);  // decimate
    
    rx->m_SMeter.ProcessData(num, (TYPECPX *)buf, (TYPEREAL)rx->bw);
        
    if(rx->decodemode < MODE_AM){
		freqdem_demodulate_block(f->demod, (liquid_float_complex *)buf, (int)num, (float *)buf2);
        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
        //printf("2 rx->size %d num %u num2 %u\n",rx->size,num,num2);
    }else if(rx->decodemode < MODE_USB){
        #define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

        for(unsigned int n=0;n<num;++n){
            double mag=sqrt(buf[2*n]*buf[2*n]+buf[2*n+1]*buf[2*n+1]);
            double z0=mag + (f->amHistory * DC_ALPHA);
            buf2[n]=(float)(z0-f->amHistory);
            f->amHistory=z0;
        }
        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
    }else{
        ampmodem_demodulate_block(f->demodAM,  (liquid_float_complex *)buf, (int)num, (float *)buf2);
        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
   }
	//iirfilt_crcf_execute_block(f->dcFilter, (liquid_float_complex *)buf, num, (liquid_float_complex *)buf);
	
	//iirfilt_crcf_execute_block(f->lowpass, (liquid_float_complex *)buf, num2, (liquid_float_complex *)buf);

	//fprintf(stderr,"doFilter witch %d end num %d Ratio %f size %d num2 %d \n",witch,num,rx->Ratio,rx->size,num2);
    
	return 0;
}
static int doMix(struct playData *rx,float *buf,float *buf2,struct Filters *f)
{
    if(!rx)return 0;

    double scale=pow(10.0,rx->scaleFactor/20.0);

    double sint,cost;
    
    for (int k = 0 ; k < rx->size ; k++){
        float r = (float)(scale*buf[k * 2]);
        float i = (float)(scale*buf[k * 2 + 1]);
        //r = 0.001*(rand() % 100);
        //i = 0.001*(rand() % 100);
        if(rx->dt > 0){
            //if(k == 0)printf("b %g ",atan2(i,r));
            buf2[k * 2] = (float)(r*rx->coso - i*rx->sino);
            buf2[k * 2 + 1] = (float)(i*rx->coso + r*rx->sino);
            sint=rx->sino*rx->cosdt+rx->coso*rx->sindt;
            cost=rx->coso*rx->cosdt-rx->sino*rx->sindt;
            rx->coso=cost;
            rx->sino=sint;
            //if(k == 0)printf("b %g diff %g\n",atan2(buf2[k * 2],buf2[k * 2 + 1]),atan2(buf2[k * 2],buf2[k * 2 + 1])-atan2(i,r));
        }else{
            buf2[k * 2] = r;
            buf2[k * 2 + 1] = i;
        }
    }
    
    double r=sqrt(rx->coso*rx->coso+rx->sino*rx->sino);
    rx->coso /= r;
    rx->sino /= r;

    /*
     float shift=rx->fc-rx->f;
     
     nco_crcf_set_frequency(f->fShift,(float) ((2.0 * M_PI) * (((double) abs(shift)) / ((double) rx->samplerate))));
     
     if (shift >= 0) {
     nco_crcf_mix_block_up(f->fShift, (liquid_float_complex *)buf, (liquid_float_complex *)buf2, rx->size);
     } else {
     nco_crcf_mix_block_down(f->fShift, (liquid_float_complex *)buf, (liquid_float_complex *)buf2,rx->size);
     }
    */
    
    return 0;
    
}
static int pushBuffa(int nbuffer,struct playData *rx)
{

    if(!rx)return 0;
    
	rx->mutexa.lock();
    rx->bufftopa=0;
    
	//fprintf(stderr,"pushBuffa in %d\n",rx->bufftopa);
	
    if(rx->bufftopa >= NUM_ABUFF5){
        rx->bufftopa=NUM_ABUFF5;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_ABUFF5;++k){
             if(rx->buffStacka[k] < small2){
             	small2=rx->buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	rx->buffStacka[ks]=nbuffer;
        }
   }else{
    	rx->buffStacka[rx->bufftopa++]=nbuffer;
    }
    
//    fprintf(stderr,"pushBuffa nbuffer %d top %d\n",nbuffer,rx->bufftopa);
    
//	fprintf(stderr,"pushBuffa out\n");
    rx->mutexa.unlock();

	return 0;
}

static int popBuffa(struct playData *rx)
{
	int ret;
	
	if(!rx)return -1;
    
	rx->mutexa.lock();
    
    //fprintf(stderr,"popBuffa in %d\n",rx->bufftopa);
	
	ret=-1;
	
 	if(rx->bufftopa < 1)goto Out;
 	
 	if(rx->bufftopa == 1){
 		ret=rx->buffStacka[0];
 		rx->bufftopa=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<rx->bufftopa;++k){
             if(rx->buffStacka[k] < small2){
             	small2=rx->buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	ret=rx->buffStacka[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<rx->bufftopa;++k)
        	{
        		if(k == ks)continue;
        		rx->buffStacka[kk++]=rx->buffStacka[k];
        	}
        	rx->bufftopa--;
        }
	
	
Out:
//    if(ret > 0)fprintf(stderr,"popBuffa ret %d top %d\n",ret,rx->bufftopa);
//	fprintf(stderr,"popBuffa out\n");
	rx->mutexa.unlock();
	return ret;
}

static int pushBuff(int nbuffer,struct playData *rx)
{

    if(!rx)return 0;

	rx->mutex.lock();
    
	//fprintf(stderr,"pushBuff pushBuff %d\n",rx->bufftop);
	
    if(rx->bufftop >= NUM_DATA_BUFF5){
        rx->bufftop=0;
        rx->buffStack[rx->bufftop++]=nbuffer;
/*
        rx->bufftop=NUM_DATA_BUFF5;
        int small,ks;
        small=1000000000;
        ks=-1;
        for(int k=0;k<NUM_DATA_BUFF5;++k){
             if(rx->buffStack[k] < small){
             	small=rx->buffStack[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	rx->buffStack[ks]=nbuffer;
        }
 */
   }else{
    	rx->buffStack[rx->bufftop++]=nbuffer;
    }
	//fprintf(stderr,"p %d out\n",rx->bufftop);
    
	rx->mutex.unlock();
	
	return 0;
}

static int popBuff(struct playData *rx,struct Filters *f)
{
	int ret;
	
    if(!rx)return -1;

	rx->mutex.lock();
	
	//fprintf(stderr,"popBuff bufftop %d %d in\n",rx->bufftop,f->thread);
    
	ret=-1;
	
 	if(rx->bufftop < 1)goto Out;
 	
 	if(rx->bufftop == 1){
 		ret=rx->buffStack[0];
 		rx->bufftop=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<rx->bufftop;++k){
             if(rx->buffStack[k] < small2){
             	small2=rx->buffStack[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	ret=rx->buffStack[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<rx->bufftop;++k)
        	{
        		if(k == ks)continue;
        		rx->buffStack[kk++]=rx->buffStack[k];
        	}
        	rx->bufftop--;
        }
	
	
Out:
	//fprintf(stderr,"popBuff ret %d out\n",ret);
	rx->mutex.unlock();
	return ret;
}

static int findRadio(struct playData *rx)
{
    
    static SoapySDR::Device *deviceSave=NULL;
    
    std::string argStr;
    
    if(!rx)return 0;
    
    rx->device = NULL;
    
    SoapySDR::Kwargs deviceArgs;
    
    deviceArgs = rx->deviceToOpen;

    for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
        printf("%s=%s\n",it->first.c_str(),it->second.c_str());
        if (it->first == "driver") {
            //dev->setDriver(it->second);
            mstrncpy(rx->driveName,(char *)it->second.c_str(),sizeof(rx->driveName));
        } else if (it->first == "label") {
            mstrncpy(rx->driveName,(char *)it->second.c_str(),sizeof(rx->driveName));
            if(rx->channel == 0){
                mstrncat(rx->driveName,(char *)" (0)",sizeof(rx->driveName));
            }else{
                mstrncat(rx->driveName,(char *)" (1)",sizeof(rx->driveName));
            }
           //dev->setName(it->second);
        }
    }
    
    
    if(rx->channel == 0){
/*
        if(deviceArgs.count("frequency") == 0){
            fprintf(stderr,"frequency not found\n");
        }
        deviceArgs["frequency"]=rx->fc;
 */
        rx->device = SoapySDR::Device::make(deviceArgs);
        fprintf(stderr,"device %p\n",rx->device);
        deviceSave=rx->device;
    }else{
       rx->device=deviceSave;
   }
    
    testRadio(rx);
    
    rx->device->setSampleRate(SOAPY_SDR_RX, rx->channel, rx->samplerate);
     
    const std::vector<size_t> channels = {(unsigned long)rx->channel};

     rx->rxStream=rx->device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32,channels);
    if(rx->rxStream == NULL){
        fprintf(stderr,"setupStream returned NULL\n");
        return 1;
    }
    
    rx->device->activateStream(rx->rxStream, 0, 0, 0);
   
   // fprintf(stderr,"findRadio frequency %g channel %d\n",rx->fc,rx->channel);
    
    rx->device->setFrequency(SOAPY_SDR_RX, rx->channel, rx->fc-rx->foffset);

    
    return 0;
    
}
int rxBuffer(void *rxv)
{

	struct playData *rx=(struct playData *)rxv;
    
    int ret = -1;
	
    if(!rx)return 0;

	while(1)
	{
	     switch(rx->controlRF){
	     case 0:
	     	Sleep2(5);
	        break;
	     case 1:
	        //fprintf(stderr,"Exit rxBuffer\n");
            rx->controlRF=0;
	        return 0;
		 case 2:
	       // fprintf(stderr,"rxBuffer case 2\n");
	       	        
        	long long timeNs=0;
           
            float *buff=rx->buff[rx->witchRFBuffer % NUM_DATA_BUFF5];
             
            void *buffs[] = {buff};
            
            int toRead=rx->size;
/*
            unsigned char *end;
            end=(unsigned char *)buff;
            unsigned int r=2*rx->size*sizeof(float);
            if(end[r] != 0xff || end[r+1] != 0xff || end[r+2] != 0xff || end[r+3] != 0xff){
                fprintf(stderr,"1 free error data corrupt Buffer %d\n",rx->witchRFBuffer);
            }
*/
            FILE *file =(FILE *)rx->iqOutput;
                 
            int count=0;
                 
            ret = -1;
                 
            while(rx->controlRF == 2){
				int flags=0;
				
				buffs[0]=buff+2*count;
				
				size_t iread;
				
				iread=toRead;
                if(iread > 500000)iread=500000;

				ret = rx->device->readStream(rx->rxStream, buffs, iread, flags, timeNs, 100000L);
			 
                timeNs++;
						   
				if(ret <= 0){
				   printf("Device %s ret=%d, flags=%d, timeNs=%lld b0 %f b1 %f witch %d\n",rx->driveName, ret, flags, timeNs,buff[0],buff[1],rx->witchRFBuffer);
				   break;
				}else if(ret < toRead){
                    count += ret;
                    toRead=toRead-ret;
					//printf("ret=%d, count %d flags=%d, timeNs=%lld b0 %f b1 %f toRead %d witch %d\n", ret, count, flags, timeNs,buff[0],buff[1],toRead,rx->witchRFBuffer);
				}else{
                    count += ret;

                    //printf("Device %s ret=%d, count %d  size %d flags=%d, b0 %f toRead %d witch %d\n",rx->driveName, ret, count,rx->size, flags, buff[0],toRead,rx->witchRFBuffer);

					break;
				}
            }
	        if(rx->controlRF == 2){
/*
                if(end[r] != 0xff || end[r+1] != 0xff || end[r+2] != 0xff || end[r+3] != 0xff){
                    fprintf(stderr,"2 free error data corrupt Buffer %d count %d rx->size %d\n",rx->witchRFBuffer,count,rx->size);
                }
 */
        	    pushBuff(rx->witchRFBuffer,rx);
                
                if(rx->controlSend == 0){
                    for(int n=0;n<rx->size*2;++n){
                       rx->sendBuff1[n]=buff[n];
                    }
                    rx->controlSend = 1;
                }
                
                if(file){
                    printf("rx->witchRFBuffer %d rx->size %d\n",rx->witchRFBuffer,rx->size);
                    size_t ret=fwrite(buff, 2*sizeof(float), rx->size,file);
                    if(ret == 0){
                        ;
                    }
                }

               // double scale=pow(10.0,rx->scaleFactor/20.0);
/*
                double amin=1e30;
                double amax=-1e30;
                double v;
                
                for (int k = 0 ; k < rx->size ; k++){
                    float r = (float)(buff[k * 2]*scale);
                    float i = (float)(buff[k * 2 + 1]*scale);
                    buff[k * 2] = r;
                    buff[k * 2 + 1] = i;
                    v=sqrt(i*i+r*r);
                    if(v > amax)amax=v;
                    if(v < amin)amin=v;
                }
*/
                //printf("amin %g amax %g witch %d count %d\n",amin,amax,rx->witchRFBuffer,count);
                if(ret != -1){
                    //double as=(double)rx->size/(double)rx->FFTcount;
                    for(int k=0;k<rx->FFTcount;++k){
                        //int ks=k*as;
                        if(k < rx->size){
                           // rx->real[k]=buff[2*ks]*scale;
                           // rx->imag[k]=buff[2*ks+1]*scale;
                            rx->real[k]=buff[2*k];
                            rx->imag[k]=buff[2*k+1];
                        }else{
                            rx->real[k]=0;
                            rx->imag[k]=0;
                        }
                    }
                }
             	++rx->witchRFBuffer;
				//fprintf(stderr,"out rxBuffer rx->witch %d\n", rx->witch);
	        	//rx->doWhat=0;
	        }
	        break;
		     
	     }
	     
	     //fprintf(stderr,"rx->doWhat %d\n",rx->doWhat);
	}
	return 0;
}




static int zerol(unsigned char *s,unsigned long n)
{
    if(!s || (n <= 0))return 1;


	while(n-- > 0)*s++ = 0;
	
	return 0;
}

static int doAudio(float *aBuff,struct playData *rx)
{
	int short *data;
	int audioOut;

    if(!rx)return 0;
	
	rx->mutexo.lock();
	audioOut=rx->witchAudioBuffer;
	//fprintf(stderr,"witchAudioBuffer %d\n",audioOut);
	data=rx->buffa[rx->witchAudioBuffer++ % NUM_ABUFF5];
	rx->mutexo.unlock();
		
	float *buff=aBuff;

	
	double dmin,dnom,gain;
    
/*

	for (int i=0; i<BLOCK_SIZE; i++) {
        agc_rrrf_execute(rx->agc, buff[i], &buff[i]);
    }
*/
    
	gain=rx->gain;
	
	if(gain <= 0.0)gain=1.0;
    
    double amin=1e30;
    double amax=-1e30;
    double average=0;
	for (int i=0; i<BLOCK_SIZE5; i++ ) {
		double v;
        v=buff[i];
        average += v;
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}
    
    average /= BLOCK_SIZE5;
    
    amin -= average;
    amax -= average;
    
	//fprintf(stderr,"doAudio amin %f amax %f average %f\n",amin,amax,average);

    if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;
    rx->aminGlobal = 0.8*rx->aminGlobal+0.2*amin;
    amin=rx->aminGlobal;
    
    if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;
    rx->amaxGlobal = 0.8*rx->amaxGlobal+0.2*amax;
    amax=rx->amaxGlobal;

   // fprintf(stderr,"amax %g amaxg %g amin %g aming %g\n",amax,rx->amaxGlobal,amin,rx->aminGlobal);

    if((amax-amin) > 0){
        dnom=65535.0/(amax-amin);
    }else{
        dnom=65535.0;
    }
		
	dmin=amin;
    //amin=1e30;
   // amax=-1e30;
    

	for(int k=0;k<BLOCK_SIZE5;++k){
		double v;
        v=buff[k];
		v=gain*((v-average)*dnom);
        if(v < -32765){
            v = -32765;
        }else if(v > 32765){
            v=32765;
        }
		data[k]=(short int)v;
       // if(v < amin)amin=v;
       // if(v > amax)amax=v;
	}
	
   // fprintf(stderr,"doAudio amin %f amax %f \n",amin,amax);

	pushBuffa(audioOut,rx);

	return 0;
}
int testRadio(struct playData *rx)
{
    if(!rx)return 0;
    
    rx->getRadioAttributes=1;
    
    try
    {
        //std::cout << "driver=" << SoapySDRDevice_getDriverKey(rx->device) << std::endl;
        
        //std::cout << "hardware=" << SoapySDRDevice_getHardwareKey(rx->device) << std::endl;
        
        rx->nreceive=(int)rx->device->getNumChannels(SOAPY_SDR_RX);
        
        rx->ntransmit=(int)rx->device->getNumChannels(SOAPY_SDR_TX);
        
        printf("receive channels %d transmit channels %d\n",rx->nreceive,rx->ntransmit);

        SoapySDR::Kwargs it=rx->device->getHardwareInfo();
        
        for( SoapySDR::Kwargs::iterator  ii=it.begin(); ii!=it.end(); ++ii)
        {
            printf("Hardware %s: %s\n",(*ii).first.c_str(),(*ii).second.c_str());
        }
        
        it=rx->device->getChannelInfo(SOAPY_SDR_RX,rx->channel);
        
        for( SoapySDR::Kwargs::iterator  ii=it.begin(); ii!=it.end(); ++ii)
        {
            printf("Channel %s: %s\n",(*ii).first.c_str(),(*ii).second.c_str());
        }

        std::vector<std::string> names=rx->device->listAntennas( SOAPY_SDR_RX, rx->channel);
        
        rx->antennaCount=names.size();
        rx->antenna=(char **)cMalloc((unsigned long)(rx->antennaCount*sizeof(char *)),8833);
        for (size_t i=0;i<names.size();++i){
            rx->antenna[i]=strsave((char *)names[i].c_str(),5555);
        }

        names = rx->device->listGains(SOAPY_SDR_RX, rx->channel);
        rx->gainsCount=names.size();
        rx->gains=(char **)cMalloc((unsigned long)(rx->gainsCount*sizeof(char *)),8833);
        rx->gainsMinimum=(double *)cMalloc((unsigned long)(rx->gainsCount*sizeof(double)),8891);
        rx->gainsMaximum=(double *)cMalloc((unsigned long)(rx->gainsCount*sizeof(double)),8892);
        
        for (size_t j = 0; j < rx->gainsCount; j++)
        {
            rx->gains[j]=strsave((char *)names[j].c_str(),5555);
            //printf("Gains %lu %s ",j, rx->gains[j]);
            
            SoapySDR::Range range3=rx->device->getGainRange(SOAPY_SDR_RX, rx->channel, rx->gains[j]);
            //double el=SoapySDRDevice_getGainElement(rx->device, SOAPY_SDR_RX, rx->channel, rx->gains[j]);
            //printf("range max %g min %g\n",range3.maximum(),range3.minimum());
            rx->gainsMinimum[j]=range3.minimum();
            rx->gainsMaximum[j]=range3.maximum();
        }
        

        SoapySDR::Range range=rx->device->getGainRange(SOAPY_SDR_RX, rx->channel);
        
      //  printf("range RX max %g min %g\n",range.maximum(),range.minimum());
        
        rx->gainsMin=range.minimum();
        rx->gainsMax=range.maximum();

        rx->hasGainMode=rx->device->hasGainMode(SOAPY_SDR_RX, rx->channel);
        
       // printf("hasGainMode %d\n",rx->hasGainMode);
        
        if(rx->hasGainMode){
            rx->gainMode=rx->device->getGainMode(SOAPY_SDR_RX, rx->channel);
           // printf("GainMode %d\n",rx->gainMode);
        }

        
        SoapySDR::RangeList rlist=rx->device->getFrequencyRange(SOAPY_SDR_RX, rx->channel);
        rx->frequencyCount=rlist.size();
        rx->frequencyMinimum=(double *)cMalloc((unsigned long)(rx->frequencyCount*sizeof(double)),8894);
        rx->frequencyMaximum=(double *)cMalloc((unsigned long)(rx->frequencyCount*sizeof(double)),8895);
        
        for (size_t j = 0; j < rx->frequencyCount; j++)
        {
         //   printf("FrequencyRange max %g min %g\n",rlist[j].maximum(),rlist[j].minimum());
            rx->frequencyMinimum[j]=rlist[j].minimum();
            rx->frequencyMaximum[j]=rlist[j].maximum();
        }

        
        
        rx->hasDCOffset=rx->device->hasDCOffset(SOAPY_SDR_RX, rx->channel);
        
       // printf("hasDCOffset %d\n",rx->hasDCOffset);
        
        if(rx->hasDCOffset){
            rx->DCOffset=rx->device->getDCOffsetMode(SOAPY_SDR_RX, rx->channel);
           // printf("DCOffset %d\n",rx->DCOffset);
        }

        std::vector<double> band=rx->device->listBandwidths(SOAPY_SDR_RX, rx->channel);
        rx->bandwidthsCount=band.size();
        rx->bandwidths=(double *)cMalloc((unsigned long)(rx->bandwidthsCount*sizeof(double)),8896);
        for (size_t j = 0; j < rx->bandwidthsCount; j++)
        {
           // printf("bandwidth %lu max %g\n",j,band[j]);
            rx->bandwidths[j]=band[j];
        }

        
        
/*
        rlist=SoapySDRDevice_getBandwidthRange(rx->device, SOAPY_SDR_RX, rx->channel, &length);
        for (size_t j = 0; j < length; j++)
        {
            printf("getBandwidthRange max %g min %g\n",rlist[j].maximum,rlist[j].minimum);
        }
*/
        
        std::vector<std::string> list=rx->device->getStreamFormats(SOAPY_SDR_RX, rx->channel);
        rx->streamFormatCount=list.size();
        rx->streamFormat=(char **)cMalloc((unsigned long)(rx->streamFormatCount*sizeof(double)),8898);
        for (size_t j = 0; j < rx->streamFormatCount; j++)
        {
            printf("StreamFormats %lu %s\n",j, list[j].c_str());
            rx->streamFormat[j]=strsave((char *)list[j].c_str(),95695);
        }

        
        
/*
        bool hasIQBalance=SoapySDRDevice_hasIQBalance(rx->device, SOAPY_SDR_RX, rx->channel);
        printf("hasIQBalance %d\n",hasIQBalance);
        if(hasIQBalance){
            double balanceI;
            double balanceQ;
            int ret=SoapySDRDevice_getIQBalance(rx->device, SOAPY_SDR_RX, rx->channel, &balanceI, &balanceQ);
            printf("ret %d balanceI %g balanceQ %g\n",ret,balanceI,balanceQ);
        }
*/
        
        std::vector<double> rate=rx->device->listSampleRates(SOAPY_SDR_RX, rx->channel);
        rx->sampleRatesCount=rate.size();
        rx->sampleRates=(double *)cMalloc((unsigned long)(rx->sampleRatesCount*sizeof(double)),8890);
        for (size_t j = 0; j < rx->sampleRatesCount; j++)
        {
           // printf("SampleRates %lu max %g\n",j,rate[j]);
            rx->sampleRates[j]=rate[j];
        }


        rx->directSampleMode=0;
        rx->biasMode="";
        SoapySDR::ArgInfoList args = rx->device->getSettingInfo();
        if (args.size()) {
            for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
                SoapySDR::ArgInfo arg = (*args_i);

                printf("key %s value %s read %s type %d min %g max %g step %g\n",arg.key.c_str(),arg.value.c_str(),rx->device->readSetting(arg.key).c_str(),
                       (int)arg.type,arg.range.minimum(),arg.range.maximum(),arg.range.step());

                if(arg.key == "direct_samp")rx->directSampleMode=1;
                if(arg.key == "bias_tx")rx->biasMode=arg.key;
                if(arg.key == "biasT_ctrl")rx->biasMode=arg.key;
            }
        }
        
        printf("rx->biasMode '%s'\n",rx->biasMode.c_str());
        
        args = rx->device->getSettingInfo(SOAPY_SDR_RX,rx->channel);
        if (args.size()) {
            for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
                SoapySDR::ArgInfo arg = (*args_i);
                printf("rec key %s value %s read %s\n",arg.key.c_str(),arg.value.c_str(),rx->device->readSetting(arg.key).c_str());
            }
        }
        
        args = rx->device->getStreamArgsInfo(SOAPY_SDR_RX,rx->channel);
        if (args.size()) {
            for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
                SoapySDR::ArgInfo arg = (*args_i);
                printf("stream key %s value %s \n",arg.key.c_str(),arg.value.c_str());
            }
        }
        


        // printf("rx->directSampleMode %d\n",rx->directSampleMode);
/*
 size_t length;
 rlist=rx->device->getSampleRateRange(SOAPY_SDR_RX, rx->channel);
 length=rlist.size();
 for (size_t j = 0; j < length; j++)
 {
 printf("SampleRateRange max %g min %g\n",rlist[j].maximum(),rlist[j].minimum());
 }
*/
    }

    catch (const std::exception &ex)
    {
        std::cerr << "Error making device: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    if(rx->device == NULL){
        std::cerr << "Error : device == NULL" << std::endl;
        return 1;
    }

    return 0;
    
}
#ifdef junk001
static void list_audio_devices(const ALCchar *devices)
{
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;
    
    fprintf(stdout, "Devices list:\n");
    fprintf(stdout, "----------\n");
    while (device && *device != '\0' && next && *next != '\0') {
        fprintf(stdout, "%s\n", device);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
    fprintf(stdout, "----------\n");
}
#endif
