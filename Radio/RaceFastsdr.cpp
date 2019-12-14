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

static int rxBuffer(void *rxv);

static int Process(void *rxv);

static int pushBuff(int nbuffer,struct playData *rx);
static int popBuff(struct playData *rx,struct Filters *f);
static int AudioReset(struct playData *rx);

static int pushBuffa(int nbuffer,struct playData *rx);
static int popBuffa(struct playData *rx);

static int setBuffers(struct playData *rx, int numBuff);

static int StartIt(struct playData *rx);

static int bacgroundPlay(struct playData *rx);

static int ProcessSound(void *rxv);

static int sdrDone(struct playData *rx);

static int sdrSetMode(struct playData *rx);

static struct playData rx;

static int testRadio(struct playData *rx);

static int SetAudio(struct playData *rx,char *name,int type);

struct playData *RacePointer()
{
    return &rx;
}
int RadioStart(int argc, char * argv [],struct playData *rx)
{
	zerol((unsigned char *)rx,sizeof(*rx));
    

    rx->pstopPlay=stopPlay;
    rx->pstartPlay=startPlay;
    rx->pplayRadio=playRadio;
    rx->psdrDone=sdrDone;
    rx->psdrSetMode=sdrSetMode;
    rx->pSetAudio=SetAudio;

    rx->samplerate=2000000;
	rx->deviceNumber=0;
	rx->gain=0.25;
	rx->fc=1.0e6;
	rx->f=0.6e6;
    rx->scaleFactor=0.0;
    rx->audioThreads=0;

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
		
    bacgroundPlay(rx);
    
	return 0 ;
} /* main */

static int SetAudio(struct playData *rx,char *name,int type)
{
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
    }
    return 0;
}

static int bacgroundPlay(struct playData *rx)
{
    
    pthread_mutex_init(&rx->mutex,NULL);
    pthread_mutex_init(&rx->mutexa,NULL);
    pthread_mutex_init(&rx->mutexo,NULL);
    
    rx->channels=2;

    rx->fOut=48000;
    
    startPlay(rx);
    
    playRadio(rx);
    
    return 0;
}
static int StartIt(struct playData *rx)
{
	ALenum error;
    

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
    
    alSourcePlay(rx->source);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError((unsigned char *)"StartIt alSourcePlay : ", error);
    }
   
	return 0;
}
static int playRadio(struct playData *rx)
{
    rx->controlRF =  1;
    rx->controlAudio  = -1;
    rx->controlProcess  = -1;

    Sleep2(100);
    
    //double rate=rx->samplerate;
    
    double rate=SoapySDRDevice_getSampleRate(rx->device,SOAPY_SDR_RX, 0);
    
    // SoapySDRDevice_setSampleRate(rx->device,SOAPY_SDR_RX, 0, rate);
    
    SoapySDRDevice_setFrequency(rx->device,SOAPY_SDR_RX, 0, rx->fc,NULL);
    
    if(rx->bandwidth > 0){
        int retv=SoapySDRDevice_setBandwidth(rx->device, SOAPY_SDR_RX, 0, rx->bandwidth);
        if(retv > 0)printf("SoapySDRDevice_setBandwidth returned %d\n",retv);
    }
    
    double bw=SoapySDRDevice_getBandwidth(rx->device, SOAPY_SDR_RX, 0);
    
    int size=(int)(0.5+rate/10.0);
    
    rx->size=size;
    
    printf("Device %s samplerate %.0f rx->size %d Bandwidth %.0f\n",rx->driveName,rate,rx->size,bw);
    
    size += 256;  // bug in rfspace NetSDR
    
    for(int k=0;k<NUM_DATA_BUFF5;++k){
        if(rx->buff[k])cFree((char *)rx->buff[k]);
        rx->buff[k]=(float *)cMalloc(2*size*sizeof(float),8887);
        if(!rx->buff[k]){
            fprintf(stderr,"1 malloc Errror %ld\n",(long)(2*size*sizeof(float)));
            return 1;
        }
        zerol((unsigned char *)rx->buff[k],2*size*sizeof(float));
        rx->buffStack[k]=-1;
    }
    
    for(int k=0;k<NUM_ABUFF5;++k){
        if(rx->buffa[k])cFree((char *)rx->buffa[k]);
        rx->buffa[k]=(short int *)cMalloc(2*rx->fOut*4,8888);
        if(!rx->buffa[k]){
            fprintf(stderr,"1 malloc Errror %ld\n",(long)(2*rx->fOut*4));
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
static int ProcessSound(void *rxv)
{
    struct playData *rx=(struct playData *)rxv;
    
    ALenum error;
    
    ALint processed;
    
    unsigned long count=0;
    
    double start=rtime();
    
    double end=rtime();
    
    //fprintf(stderr,"ProcessSound start\n");
    
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
                    //printf("finish bufferd %d\n",fbuff[k]);
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
    
    rx->controlRF=0;
    
    Sleep2(100);
    
    rx->controlRF=1;
    
    Sleep2(100);
    
    rx->controlAudio=-1;

    Sleep2(100);
    
    rx->controlRF=1;

    //fprintf(stderr,"ProcessSound end\n");
    
    return 0;
}
static int sdrSetMode(struct playData *rx)
{
    rx->controlAudio = -1;
    
    Sleep2(100);

    rx->controlAudio = 0;

    launchThread((void *)rx,Process);
    
    return 0;
}
static int sdrDone(struct playData *rx)
{

    stopPlay(rx);
    

    if(rx->agc)agc_rrrf_destroy(rx->agc);
    rx->agc=NULL;


    for(int k=0;k<NUM_DATA_BUFF5;++k){
        if(rx->buff[k])cFree((char *)rx->buff[k]);
        rx->buff[k]=NULL;
    }
    
    for(int k=0;k<NUM_ABUFF5;++k){
        if(rx->buffa[k])cFree((char *)rx->buffa[k]);
        rx->buffa[k]=NULL;
    }

    
    pthread_mutex_destroy(&rx->mutex);
    pthread_mutex_destroy(&rx->mutexa);
    pthread_mutex_destroy(&rx->mutexo);
    
    //if(rx->antenna)free((char *)rx->antenna);
    rx->antenna=NULL;
    //if(rx->gains)free((char *)rx->gains);
    rx->gains=NULL;
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
static int setBuffers(struct playData *rx, int numBuff)
{
    ALenum error;
    ALuint buffer;
    
    buffer=getbuffAudio(audio);
    if(buffer == NO_MORE_SPACE){
        fprintf(stderr,"getbuffAudio out of buffers\n");
        return 1;
    }

    if(rx->audioOutput){
        size_t ret=fwrite(rx->buffa[numBuff % NUM_ABUFF5], 2, 4800,rx->audioOutput);
        if(ret == 0){
            ;
        }
    }
    
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
    
    f.thread=rx->threadNumber++;
	
	setFilters(rx,&f);
    
    int bsize=2*rx->size*sizeof(float);
    if(bsize < 200000)bsize=200000;
	
	float *wBuff=(float *)cMalloc(bsize,8889);
    if(!wBuff){
        fprintf(stderr,"2 malloc Errror %ld\n",(long)(bsize));
       	 return 1;
    }
    zerol((unsigned char *)wBuff,bsize);

   // printf("Process Start rx->frame %d Thread %d\n",rx->controlAudio,f.thread);
	
	float *aBuff=(float *)cMalloc(2*rx->fOut*4,8890);
    if(!aBuff){
        fprintf(stderr,"3 malloc Errror %ld\n",(long)(2*rx->fOut*4));
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

static int AudioReset(struct playData *rx)
{
    
	ALint processed;
	ALenum error;

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
    }
    
    rx->Ratio = (float)(rx->bw/ rx->samplerate);
    ratio= (float)(48000.0/rx->bw);
    
    f->demod=freqdem_create(0.5);
    
    f->demodAM = ampmodem_create(0.5, mode, iflag);
    
  //  f->demodAM = ampmodem_create(0.5, 0.0, mode, iflag);

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
    }else if(rx->decodemode < MODE_LSB){
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
    
    double scale=pow(10.0,rx->scaleFactor/20.0);

    double sint,cost;
    
    for (int k = 0 ; k < rx->size ; k++){
        float r = (float)(scale*buf[k * rx->channels]);
        float i = (float)(scale*buf[k * rx->channels + 1]);
        //r = 0.001*(rand() % 100);
        //i = 0.001*(rand() % 100);
        if(rx->dt > 0){
            buf2[k * rx->channels] = (float)(r*rx->coso - i*rx->sino);
            buf2[k * rx->channels + 1] = (float)(i*rx->coso + r*rx->sino);
            sint=rx->sino*rx->cosdt+rx->coso*rx->sindt;
            cost=rx->coso*rx->cosdt-rx->sino*rx->sindt;
            rx->coso=cost;
            rx->sino=sint;
        }else{
            buf2[k * rx->channels] = r;
            buf2[k * rx->channels + 1] = i;
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

	pthread_mutex_lock(&rx->mutexa);
//	fprintf(stderr,"pushBuffa in %d\n",rx->bufftopa);
	
    if(rx->bufftopa >= NUM_ABUFF5){
        rx->bufftopa=NUM_ABUFF5;
        int small,ks;
        small=1000000000;
        ks=-1;
        for(int k=0;k<NUM_ABUFF5;++k){
             if(rx->buffStacka[k] < small){
             	small=rx->buffStacka[k];
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
	pthread_mutex_unlock(&rx->mutexa);
	
	return 0;
}

static int popBuffa(struct playData *rx)
{
	int ret;
	
	
	pthread_mutex_lock(&rx->mutexa);
//	fprintf(stderr,"popBuffa in %d\n",rx->bufftopa);
	
	ret=-1;
	
 	if(rx->bufftopa < 1)goto Out;
 	
 	if(rx->bufftopa == 1){
 		ret=rx->buffStacka[0];
 		rx->bufftopa=0;
 		goto Out;
 	}
 	
       int small,ks;
        small=1000000000;
        ks=-1;
        for(int k=0;k<rx->bufftopa;++k){
             if(rx->buffStacka[k] < small){
             	small=rx->buffStacka[k];
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
	pthread_mutex_unlock(&rx->mutexa);
	return ret;
}

static int pushBuff(int nbuffer,struct playData *rx)
{

	pthread_mutex_lock(&rx->mutex);
	//fprintf(stderr,"p in\n");
	
    if(rx->bufftop >= NUM_DATA_BUFF5){
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
   }else{
    	rx->buffStack[rx->bufftop++]=nbuffer;
    }
	//fprintf(stderr,"p %d out\n",rx->bufftop);
    
	pthread_mutex_unlock(&rx->mutex);
	
	return 0;
}

static int popBuff(struct playData *rx,struct Filters *f)
{
	int ret;
	
	
	pthread_mutex_lock(&rx->mutex);
	
	//fprintf(stderr,"popBuff %d in\n",f->thread);
	ret=-1;
	
 	if(rx->bufftop < 1)goto Out;
 	
 	if(rx->bufftop == 1){
 		ret=rx->buffStack[0];
 		rx->bufftop=0;
 		goto Out;
 	}
 	
       int small,ks;
        small=1000000000;
        ks=-1;
        for(int k=0;k<rx->bufftop;++k){
             if(rx->buffStack[k] < small){
             	small=rx->buffStack[k];
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
	pthread_mutex_unlock(&rx->mutex);
	return ret;
}

static int findRadio(struct playData *rx)
{
    
    std::string argStr;

   size_t length;
    
    SoapySDRKwargs *results = SoapySDRDevice_enumerate(NULL, &length);
    
    printf("\nNumber of Devices: %d Looking for Device: %d\n",(int)length,rx->deviceNumber);
    if(length < 1)return 1;
    
    rx->device = NULL;
    
    SoapySDRKwargs deviceArgs;
    
    size_t driver=0;
    for(unsigned int k=0;k<length;++k){
        
        if(k == rx->deviceNumber){

 			deviceArgs = results[k];
			
            printf("Found device #%d:",(int)k);
			for (size_t j = 0; j < deviceArgs.size; j++)
			{
				printf("%s=%s, ", deviceArgs.keys[j], deviceArgs.vals[j]);
                if(!strcmp("driver",deviceArgs.keys[j])){
                    driver=j;
                }
			}
			printf("\n");		
   
            //printf("Device %d selected %s\n",k,deviceArgs.vals[0]);
            
            mstrncpy(rx->driveName,(char *)deviceArgs.vals[driver],sizeof(rx->driveName));
            
            
			rx->device = SoapySDRDevice_make(&deviceArgs);
            
/*
            SoapySDRKwargs args = {};
            SoapySDRKwargs_set(&args, "driver", "netsdr");
            SoapySDRKwargs_set(&args, "netsdr","192.168.1.7:50000");
            rx->device = SoapySDRDevice_make(&args);
*/
            
            testRadio(rx);
            
			SoapySDRDevice_setSampleRate(rx->device,SOAPY_SDR_RX, 0, rx->samplerate);
            
			SoapySDRDevice_setFrequency(rx->device,SOAPY_SDR_RX, 0, rx->fc,NULL);
			
        	// std::cout << "rx->samplerate " << rx->samplerate << std::endl;
			
          SoapySDRDevice_setupStream(rx->device,&rx->rxStream,SOAPY_SDR_RX, SOAPY_SDR_CF32, NULL,0,NULL);
            
            // rx->rxStream=SoapySDRDevice_setupStream(rx->device,SOAPY_SDR_RX, SOAPY_SDR_CF32, NULL,0,NULL);
            
			SoapySDRDevice_activateStream(rx->device,rx->rxStream, 0, 0, 0);
            
            // std::cout << "SoapySDRDevice_getStreamMTU: " << SoapySDRDevice_getStreamMTU(rx->device,rx->rxStream) << std::endl;
            
			//std::cout << "getGainMode: " <<SoapySDRDevice_getGainMode( rx->device,SOAPY_SDR_RX, 0) << " ";

			//std::cout << std::endl;
		}
    
}
    
    return 0;
    
}
static int rxBuffer(void *rxv)
{

	struct playData *rx=(struct playData *)rxv;
	
	while(1)
	{
	     switch(rx->controlRF){
	     case 0:
	     	Sleep2(5);
	        break;
	     case 1:
	        //fprintf(stderr,"Exit rxBuffer\n");
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
                 
            int ret = -1;
                 
            while(rx->controlRF == 2){
				int flags=0;
				
				buffs[0]=buff+2*count;
				
				size_t iread;
				
				iread=toRead;
                if(iread > 500000)iread=500000;

				ret = SoapySDRDevice_readStream(rx->device,rx->rxStream, buffs, iread, &flags, &timeNs, 100000L);
			 
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
                
                if(file){
                    printf("rx->witchRFBuffer %d rx->size %d\n",rx->witchRFBuffer,rx->size);
                    size_t ret=fwrite(buff, 2*sizeof(float), rx->size,file);
                    if(ret == 0){
                        ;
                    }
                }

                double scale=pow(10.0,rx->scaleFactor/20.0);
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
                
                rx->count=4096;
                for(int k=0;k<4096;++k){
                    if(k < rx->size){
                        rx->real[k]=buff[2*k]*scale;
                        rx->imag[k]=buff[2*k+1]*scale;
                    }else{
                        rx->real[k]=0;
                        rx->imag[k]=0;
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


static int startPlay(struct playData *rx)
{
    
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
    
    if(rx->agc)agc_rrrf_destroy(rx->agc);
    
    rx->agc = agc_rrrf_create();
    
    agc_rrrf_set_bandwidth(rx->agc, 0.25);
    
    if(findRadio(rx) || rx->device == NULL){
        fprintf(stderr,"Error No SDR Found\n");
        return 1;
    }
    
    // fprintf(stderr,"startPlay done\n");
    
	return 0;
}

static int stopPlay(struct playData *rx)
{
    
    rx->controlProcess = -1;
    
    Sleep2(300);
    
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

    if(rx->device){
        
        SoapySDRDevice_deactivateStream(rx->device,rx->rxStream, 0, 100000L);
        
        SoapySDRDevice_closeStream(rx->device,rx->rxStream);
        
        SoapySDRDevice_unmake(rx->device);
        
        rx->device=NULL;
        
        rx->rxStream=NULL;
    }
    
    
    // fprintf(stderr,"Out Of stopPlay\n");

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

	
	pthread_mutex_lock(&rx->mutexo);
	audioOut=rx->witchAudioBuffer;
	//fprintf(stderr,"audioOut %d\n",audioOut);
	data=rx->buffa[rx->witchAudioBuffer++ % NUM_ABUFF5];
	pthread_mutex_unlock(&rx->mutexo);
	

		
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
    
	for (int i=0; i<BLOCK_SIZE5; i++ ) {
		double v;
        v=buff[i];
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}

	//fprintf(stderr,"doAudio amin %f amax %f \n",amin,amax);

    if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;
    rx->aminGlobal = 0.9*rx->aminGlobal+0.1*amin;
    amin=rx->aminGlobal;
    
    if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;
    rx->amaxGlobal = 0.9*rx->amaxGlobal+0.1*amax;
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
		v=gain*((v-dmin)*dnom-32768);
        if(rx->mute)v=0.0;
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
    
    
    rx->getRadioAttributes=1;
    
    //if(rx->antenna)free((char *)rx->antenna);
    rx->antenna=NULL;
    //if(rx->gains)free((char *)rx->gains);
    rx->gains=NULL;
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
    
    try
    {
        //std::cout << "driver=" << SoapySDRDevice_getDriverKey(rx->device) << std::endl;
        
        //std::cout << "hardware=" << SoapySDRDevice_getHardwareKey(rx->device) << std::endl;
        
        SoapySDRKwargs info=SoapySDRDevice_getHardwareInfo(rx->device);
        
        for (size_t j = 0; j < info.size; j++)
        {
            //printf("keys %s vals %s, ", info.keys[j], info.vals[j]);
        }
       // printf("\n");

        //size_t length;
        rx->antenna=SoapySDRDevice_listAntennas(rx->device, SOAPY_SDR_RX, 0, &rx->antennaCount);
        
        for (size_t j = 0; j < rx->antennaCount; j++)
        {
            //printf("Antenna %lu %s\n",j, rx->antenna[j]);
        }
        
        rx->gains=SoapySDRDevice_listGains(rx->device, SOAPY_SDR_RX, 0, &rx->gainsCount);
        
        rx->gainsMinimum=(double *)cMalloc((unsigned long)(rx->gainsCount*sizeof(double)),8891);
        
        rx->gainsMaximum=(double *)cMalloc((unsigned long)(rx->gainsCount*sizeof(double)),8892);

        for (size_t j = 0; j < rx->gainsCount; j++)
        {
           // printf("Gains %lu %s ",j, rx->gains[j]);
            
            SoapySDRRange range3=SoapySDRDevice_getGainElementRange(rx->device, SOAPY_SDR_RX, 0, rx->gains[j]);
			//double el=SoapySDRDevice_getGainElement(rx->device, SOAPY_SDR_RX, 0, rx->gains[j]);
            // printf("range max %g min %g\n",range3.maximum,range3.minimum);
            rx->gainsMinimum[j]=range3.minimum;
            rx->gainsMaximum[j]=range3.maximum;
        }

        SoapySDRRange range=SoapySDRDevice_getGainRange(rx->device, SOAPY_SDR_RX, 0);
        
       // printf("range max %g min %g\n",range.maximum,range.minimum);
        
        rx->gainsMin=range.minimum;
        rx->gainsMax=range.maximum;

        rx->hasGainMode=SoapySDRDevice_hasGainMode(rx->device, SOAPY_SDR_RX, 0);

        //printf("hasGainMode %d\n",rx->hasGainMode);

        if(rx->hasGainMode){
            rx->gainMode=SoapySDRDevice_getGainMode(rx->device, SOAPY_SDR_RX, 0);
            //printf("GainMode %d\n",rx->gainMode);
        }

        SoapySDRRange *rlist=SoapySDRDevice_getFrequencyRange(rx->device, SOAPY_SDR_RX, 0, &rx->frequencyCount);
        rx->frequencyMinimum=(double *)cMalloc((unsigned long)(rx->frequencyCount*sizeof(double)),8894);
        rx->frequencyMaximum=(double *)cMalloc((unsigned long)(rx->frequencyCount*sizeof(double)),8895);

        for (size_t j = 0; j < rx->frequencyCount; j++)
        {
            printf("FrequencyRange max %g min %g\n",rlist[j].maximum,rlist[j].minimum);
            rx->frequencyMinimum[j]=rlist[j].minimum;
            rx->frequencyMaximum[j]=rlist[j].maximum;
       }

        rx->hasDCOffset=SoapySDRDevice_hasDCOffset(rx->device, SOAPY_SDR_RX, 0);

        printf("hasDCOffset %d\n",rx->hasDCOffset);
        
        if(rx->hasDCOffset){
            rx->DCOffset=SoapySDRDevice_getDCOffsetMode(rx->device, SOAPY_SDR_RX, 0);
            printf("DCOffset %d\n",rx->DCOffset);
        }
        
        double *band=SoapySDRDevice_listBandwidths(rx->device, SOAPY_SDR_RX, 0, &rx->bandwidthsCount);
        rx->bandwidths=(double *)cMalloc((unsigned long)(rx->bandwidthsCount*sizeof(double)),8896);
        for (size_t j = 0; j < rx->bandwidthsCount; j++)
        {
            //printf("bandwidth %lu max %g\n",j,band[j]);
            rx->bandwidths[j]=band[j];
        }
/*
        rlist=SoapySDRDevice_getBandwidthRange(rx->device, SOAPY_SDR_RX, 0, &length);
        for (size_t j = 0; j < length; j++)
        {
            printf("getBandwidthRange max %g min %g\n",rlist[j].maximum,rlist[j].minimum);
        }
*/
        char **list=SoapySDRDevice_getStreamFormats(rx->device, SOAPY_SDR_RX, 0, &rx->streamFormatCount);
        rx->streamFormat=(char **)cMalloc((unsigned long)(rx->streamFormatCount*sizeof(double)),8898);
        for (size_t j = 0; j < rx->streamFormatCount; j++)
        {
          //  printf("StreamFormats %lu %s\n",j, list[j]);
            rx->streamFormat[j]=list[j];
        }
/*
        bool hasIQBalance=SoapySDRDevice_hasIQBalance(rx->device, SOAPY_SDR_RX, 0);
        printf("hasIQBalance %d\n",hasIQBalance);
        if(hasIQBalance){
            double balanceI;
            double balanceQ;
            int ret=SoapySDRDevice_getIQBalance(rx->device, SOAPY_SDR_RX, 0, &balanceI, &balanceQ);
            printf("ret %d balanceI %g balanceQ %g\n",ret,balanceI,balanceQ);
        }
*/
        double *rate=SoapySDRDevice_listSampleRates(rx->device, SOAPY_SDR_RX, 0, &rx->sampleRatesCount);
        rx->sampleRates=(double *)cMalloc((unsigned long)(rx->sampleRatesCount*sizeof(double)),8890);
        for (size_t j = 0; j < rx->sampleRatesCount; j++)
        {
            //printf("SampleRates %lu max %g\n",j,rate[j]);
            rx->sampleRates[j]=rate[j];
        }
/*
        rlist=SoapySDRDevice_getSampleRateRange(rx->device, SOAPY_SDR_RX, 0, &length);
        for (size_t j = 0; j < length; j++)
        {
            printf("SampleRateRange max %g min %g\n",rlist[j].maximum,rlist[j].minimum);
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
