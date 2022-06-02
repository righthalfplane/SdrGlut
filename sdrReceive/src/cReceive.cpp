#include "cReceive.h"

#include <stdarg.h>

#include <unistd.h>

#include <algorithm>

static int GetTime(long *Seconds,long *milliseconds);

static class cDemod *dd;

extern volatile int threadexit; 

int doFFT2(double *x,double *y,long length,int direction);

int doWindow(double *x,double *y,long length,int type);

int fft(double *data,int nn,int isign);

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

int cDemod::rxBuffer(void *rxv)
{
	struct playData *rx=(struct playData *)rxv;
		
	while(1)
	{
	     switch(rx->doWhat){
	     case Wait:
	     	Sleep2(5);
	        break;
	     case Exit:
	        rx->r->mprint("Exit rxBuffer\n");
	        return 0;
		 case Work:
       		long long timeNs=0;
           
            float *buff=rx->buff[rx->witch % NUM_DATA_BUFF];
                       
            void *buffs[] = {buff};

            int toRead=rx->size;
            
            int count=0;
                 
            while(rx->doWhat == Work){
				int flags=0;
				
				buffs[0]=buff+2*count;
				
				int iread;
				
				iread=toRead;
				if(iread > 500000)iread=500000;
				
				int ret = rx->device->readStream(rx->rxStream, buffs, iread, flags, timeNs, 100000L);			 
						   
				if(ret <= 0){
				   fprintf(stderr,"ret=%d, flags=%d, timeNs=%lld b0 %f b1 %f witch %d\n", ret, flags, timeNs,buff[0],buff[1],rx->witch);
				   break;
				}else if(ret < toRead){
                    count += ret;
                    toRead=toRead-ret;
				}else{
					break;
				}
            }
	        if(rx->doWhat == Work){
	        	if(rx->cs)rx->cs->pushBuff(rx->witch,rx);
	        	
              	float *buff=rx->buff[rx->witch % NUM_DATA_BUFF];
              	if(rx->binary){
					int filenum=fileno(stdout);
					int count=(int)write(filenum,buff,rx->size*8);
					if(count != (int)(rx->size*8)){
						fprintf(stderr,"I/Q write error: count %ld rx->size*8 %ld\n",(long)count,(long)(rx->size*8));
					}
              	}
              	
				for(int k=0;k<rx->FFTcount;++k){
				    if(k < rx->size){
						rx->real[k]=buff[2*k];
						rx->imag[k]=buff[2*k+1];
					}else{
						rx->real[k]=0;
						rx->imag[k]=0;
					}
				}
				
            	++rx->witch; 
	        }
	        break;
	     }
	}
	return 0;
}
cDemod::~cDemod()
{
//	fprintf(stderr,"cDemod::~cDemod\n");
	if(bAudio)cFree((char *)bAudio);
	if(bRf)cFree((char *)bRf);

}
cDemod::cDemod()
{
	rx=NULL;
	bAudio=NULL;
	bRf=NULL;
}

int cDemod::sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData){
         
  ;
  
  short int *buffer = (short int *) outputBuffer;
    
  struct playData *rx=(struct playData *)userData;
  
  
  if ( status )fprintf(stderr,"Stream underflow detected!\n");
 
	int ibuff=-1;
	if(rx->cs){
		ibuff=rx->cs->popBuffa(rx);
	}
	if (ibuff > 0 && !rx->muteAudio){
		short int *buff= rx->buffa[ibuff % NUM_ABUFF];
	
		int n=0;
	
		for (unsigned int i=0; i<nBufferFrames; i++ ) {
			short int v=buff[i];
			buffer[n++] = v;
		}
		
	}else{
		for (unsigned int i=0; i<nBufferFrames; i++ ) {
			  *buffer++ = 0;
		}
	}
	
	if(rx->pipe == 1 && ibuff > 0 && rx->startSound > 20  && rx->r->iplay){
		int filenum=fileno(stdout);
		int count=(int)write(filenum,rx->buffa[ibuff % NUM_ABUFF],nBufferFrames*2);
		if(count != (int)(nBufferFrames*2)){
			fprintf(stderr,"pipe write error: count %ld nBufferFrames*2 %ld\n",(long)count,(long)(nBufferFrames*2));
		}

	}
 
  return 0;
            
}

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif
void cReceive::mprint(const char *fmt, ...)
{
	char buff[4096];
	va_list arg;
	
	
	//fprintf(stderr,"mprint rx->Debug %d\n",rx->Debug);
	
	if(rx->Debug < 1)return;
	
    va_start(arg, fmt);
    vsnprintf((char *)buff, sizeof(buff)-1, fmt, arg);
    va_end(arg);
	
	fprintf(stderr,"%s",buff);

}


int cReceive::processScan(struct playData *rx)
{
    if(!rx)return 0;
    fprintf(stderr,"Stations Above Cutoff %g db\n",rx->cutOFF);
    scanFrequencies.clear();
    int ns = -1;
    double peak=-160;
    double bw=rx->bw;
    double fStart=0;
    for(int n=20;n<rx->FFTcount;++n){
        if(ampitude[n] >= rx->cutOFF){
            if(ns == -1)fStart=frequencies[n];
            // fprintf(stderr,"n %d ns %d %g %g %g\n",n,ns,s->ampitude[n],s->rx->cutOFF,peak);
            if(ampitude[n] > peak){
                peak=ampitude[n];
                ns=n;
            }
        }else{
            if(ns >= 0){
                if(frequencies[n] < fStart+bw)continue;
                 scanFrequencies.push_back(frequencies[ns]);
    			fprintf(stderr,"frequency  %g MHZ peak %g\n",frequencies[ns]/1e6,peak);
                ns=-1;
                peak=-160;
            }
        }
    }
    
    fprintf(stderr,"\nNew Frequencies\n");
    for(std::vector<double>::size_type k=0;k<scanFrequencies.size();++k){
    		fprintf(stderr,"-f %g ",scanFrequencies[k]/1e6);
	}
    fprintf(stderr,"\n\n");
    
    fprintf(stderr,"Current Frequencies\n");
    
    fprintf(stderr,"sdrReceive.x -fc %g ",rx->fc/1e6);
    for(std::vector<double>::size_type k=0;k<frequency.size();++k){
    		if(frequency[k].flag)fprintf(stderr,"-f %g ",frequency[k].frequency/1e6);
	}
    fprintf(stderr,"\n");
    
    
    return 0;
}


int cReceive::setFrequency3(double frequency)
{

	if(frequency == rx->f)return 0;

	//fprintf(stderr,"\n setFrequency3 f %g MHZ\n",frequency/1e6);
	
	rx->f=frequency;
	
	double pi;
	pi=4.0*atan(1.0);
	rx->dt=1.0/(double)rx->samplerate;
	rx->sino=0;
	rx->coso=1;
	rx->w=2.0*pi*(rx->fc - rx->f);
	rx->sindt=sin(rx->w*rx->dt);
	rx->cosdt=cos(rx->w*rx->dt);
	//fprintf(stderr,"fc %f f %f dt %g samplerate %d\n",rx->fc,rx->f,rx->dt,rx->samplerate);


	return 0;
}
int cReceive::updateLine()
{
    double *real,*imag;
    double amin,amax,v;
    
    if(!rx)return 0;
    
    if(rtime() < lineTime)return 0;

    lineTime=rtime()+lineDumpInterval;
    
    
    if(rx->FFTcount > FFTlength){
        fprintf(stderr," FFTlength %ld\n",FFTlength);
        return 1;
    }
    
    int length=rx->FFTcount;
    
    for(int k=0;k<length;++k){
        rx->reals[k]=rx->real[k];
        rx->imags[k]=rx->imag[k];
    }

    real=rx->reals;
    imag=rx->imags;
    
     
    doWindow(real,imag,length,rx->FFTfilter);
    
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
        double mag=(1.0-lineAlpha)*magnitude[length-n-1]+v*lineAlpha;
        amin +=  mag;
        ++nn;
     }
    
    amin /= nn;
    
    double shift=-90-amin;
    rx->shiftGlobal = amin;
    if(rx->aminGlobal3 == 0.0)rx->aminGlobal3=0.25*shift;
    rx->aminGlobal3 = 0.9*rx->aminGlobal3+0.1*shift;
 //   shift=rx->aminGlobal3;

   // fprintf(stderr,"shift %g amin %g \n",shift,amin);
   
    amin =  1e33;
    amax = -1e33;

    
    float dx=rx->samplerate;
    double ddx=(double)rx->samplerate/(double)(length);
    long nf=0;
    for(int n=0;n<length;++n){
        double r;
        r=rx->fc-0.5*rx->samplerate+n*ddx;
        range[n]=r;
        if(abs(range[n]-rx->f) < dx)
        {
            dx=abs(range[n]-rx->f);
            nf=n;
        }
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        magnitude[length-n-1]=((1.0-lineAlpha)*magnitude[length-n-1]+v*lineAlpha)+shift;
        v=magnitude[length-n-1];
        magnitude2[length-n-1]=v/*+rx->scaleFactor*/;
        if(v < amin)amin=v;
        if(v > amax)amax=v;
    }
    
    (void)nf;
    
   // fprintf(stderr,"a amin %g amax %g \n",amin,amax);
  
	if(rx->cutOFFSearch){
		for(int k=0;k<length;++k){
			frequencies[k]=range[k];
			if(magnitude2[k] > ampitude[k]){
				ampitude[k]=magnitude2[k];
			}
		}
	}

	if(scanRun == 1 && scanWait != 1){
		int ifound=0;
		for(std::vector<double>::size_type k=0;k<scanFrequencies.size();++k){
		   scanFound[k]=0;
			int n1=fftIndex(scanFrequencies[k]-0.3*rx->bw);
			int n2=fftIndex(scanFrequencies[k]+0.3*rx->bw);
			if(n1 < 0 || n2 < 0)continue;
			for(int m=n1;m<=n2;++m){
				if(magnitude2[m] > rx->cutOFF){
					scanFound[k]=1;
					ifound=1;
					break;
				}
			}
		}
		if(ifound){
			rx->muteScan = 0;
			if(scanFound[pauseChannel]){
				if(rtime() < pauseTime)goto FoundTime;
			}
			
			pauseTime=rtime()+pauseTimeDelta;
			int lastChannel=pauseChannel;
			if(++pauseChannel >= (int)scanFrequencies.size())pauseChannel=0;
			for(std::vector<double>::size_type k=pauseChannel;k<scanFrequencies.size();++k){
				if(scanFound[k]){
					pauseChannel=(int)k;
					//fprintf(stderr,"Select channel %d\n",pauseChannel);
					rx->f=scanFrequencies[k];
					//fprintf(stderr,"k %d 1 Frequency Selected %.4f\n",(int)k,scanFrequencies[k]/1e6);
					//setFrequency3(rx);
					goto FoundTime;
				}
			}
			for(std::vector<double>::size_type k=0;(int)k<pauseChannel;++k){
				if(scanFound[k]){
					pauseChannel=(int)k;
					//fprintf(stderr,"Select channel %d\n",pauseChannel);
					rx->f=scanFrequencies[k];
					//fprintf(stderr,"k %d  2 Frequency Selected %.4f\n",(int)k,scanFrequencies[k]/1e6);
					//setFrequency3(rx);
					goto FoundTime;
				}
			}
			pauseChannel=lastChannel;
		}else{
			rx->muteScan = 1;
		}
	}

FoundTime:        
    
    iplay=0;

	int ip=0;
	for (size_t n = 0; n < frequency.size(); n++)
	{
		
		double f=frequency[n].frequency;

		long nf=fftIndex(f);
		
		if(nf < 0 || !frequency[n].flag){
			fmult[n].gain=0;
			continue;
		}

		long n1=fftIndex(f-0.5*rx->bw);
		if(n1 < 0)n1=nf-5;
		long n2=fftIndex(f+0.5*rx->bw);
		if(n2 < 0)n2=nf+5;

		double meterMax=magnitude2[nf];
		for(int n=0;n<length;++n){
			if(n >= n1 && n <= n2){
				if(magnitude2[n] > meterMax)meterMax=magnitude2[n];
			}
		}
		rx->meterMax=meterMax;
	
		if(meterMax > rx->cutOFF){
			fmult[n].gain=1;
		}else{
			fmult[n].gain=0;
		}

		if(fmult[n].gain > 0){
		   if(rx->Debug > 1)fprintf(stderr,"n %ld f %g db %g ",(long)n,f/1e6,meterMax);
		   ++rx->startSound;
		   if(ip == 0){
				setFrequency3(f);
				iplay=1;
		   }
		   if(ip == 1)fmult[n].gain=0;
		   ip=1;
		}
    }
    
 	if(ip)if(rx->Debug > 1)fprintf(stderr,"\n");
       

	return 0;
}

int cReceive::stopPlay(struct playData *rx)
{

    rx->doWhat=Exit;
    
    Sleep2(100);
    
    if(rx->device){
        rx->device->deactivateStream(rx->rxStream, 0, 0);
    
        rx->device->closeStream(rx->rxStream);
    
        SoapySDR::Device::unmake(rx->device);
    }
        	       	
    if(rx->antenna){
        for (size_t i=0;i<(size_t)rx->antennaCount;++i){
            cFree(rx->antenna[i]);
            rx->antenna[i]=NULL;
        }
        cFree((char *)rx->antenna);
        rx->antenna=NULL;
        rx->antennaCount=0;
	}
	
	
	for(int k=0;k<NUM_DATA_BUFF;++k){
		if(rx->buff[k])cFree((char *)rx->buff[k]);
		rx->buff[k]=NULL;
	}
	
	for(int k=0;k<NUM_ABUFF;++k){
		if(rx->buffa[k])cFree((char *)rx->buffa[k]);
		rx->buffa[k]=NULL;
	}

	
	if(rx->cs)delete rx->cs;
	rx->cs=NULL;
	    
	return 0;
}

int cReceive::playRadio(struct playData *rx)
{

	double rate=rx->device->getSampleRate(SOAPY_SDR_RX, rx->channel);
  

	int size=(int)rate/rx->ncut;

	rx->size=size;

	mprint("rate %f rx->size %d\n",rate,rx->size);
	
	
	for(int k=0;k<NUM_DATA_BUFF;++k){
		if(rx->buff[k])cFree((char *)rx->buff[k]);
		rx->buff[k]=(float *)cMalloc(2*rx->size*4*8,5789);
		if(!rx->buff[k]){
			mprint("5 cMalloc Errror %ld\n",(long)(2*rx->size*4));
			return 1;
		}
		zerol((char *)rx->buff[k],2*rx->size*4);
		rx->buffStack[k]=-1;
	}
	
	for(int k=0;k<NUM_ABUFF;++k){
		if(rx->buffa[k])cFree((char *)rx->buffa[k]);
		rx->buffa[k]=(short int *)cMalloc((size_t)(2*rx->faudio*4),5272);
		if(!rx->buffa[k]){
			mprint("10 cMalloc Errror %ld\n",(long)(2*rx->faudio*4));
			return 1;
		}
		zerol((char *)rx->buffa[k],(unsigned long)(2*rx->faudio*4));
		rx->buffStacka[k]=-1;
	}
	
	RtAudio dac;

	rx->faudioCount=rx->faudio/rx->ncut;
	
	if(!rx->muteAudio){

		RtAudio::StreamParameters parameters;
		if(rx->audiodevice > 0){
			parameters.deviceId = rx->audiodevice;
		}else{
			parameters.deviceId = dac.getDefaultOutputDevice();
		}
		parameters.nChannels = 2;
		parameters.nChannels = 1;
		parameters.firstChannel = 0;

		unsigned int bufferFrames = (unsigned int)(rx->faudioCount);


		try {
			dac.openStream( &parameters, NULL, RTAUDIO_SINT16,
							(unsigned int)rx->faudio, &bufferFrames, &sound, (void *)rx );
			dac.startStream();
		}
		catch ( RtAudioError& e ) {
			e.printMessage();
			exit( 0 );
		}
	
	}	
	
	
	rx->cs=new cStack(rx);
		
	for(int k=0;k<NUM_BUFFERS;++k){
		rx->bufferState[k]=0;
	}

	rx->doWhat=Work;

	rx->witch=0;

	rx->frame=0;
	
	//fprintf(stderr,"rx->faudioCount %f\n",rx->faudioCount);

	launchThread((void *)rx,Process);   	

	//launchThread((void *)rx,Process); 

	//launchThread((void *)rx,Process);   	        

	Sleep2(100);
 
    mprint("Start playing\n");
        
	rx->timestart=rtime();
  	while(!threadexit){
  		Sleep2(5);
		if(rx->muteAudio){	
			int ibuff=-1;
			ibuff=rx->cs->popBuffa(rx);
			if(rx->pipe == 1 && ibuff > 0 && rx->startSound > 20  && rx->r->iplay){
				int filenum=fileno(stdout);
				int count=(int)write(filenum,rx->buffa[ibuff % NUM_ABUFF],rx->faudioCount*2);
				if(count != (int)(rx->faudioCount*2)){
					fprintf(stderr,"pipe write error: count %ld rx->faudioCount*2 %ld\n",(long)count,(long)(rx->faudioCount*2));
				}
			}
		}
		if(rx->timeout > 0 && rtime() > rx->timeout+rx->timestart){
			break;
		}
		
   	}      
    double end=rtime();
    
    double total=end-rx->timestart;
    
    mprint(" Seconds %.2f\n",total);
    
    rx->doWhat=Wait;
        
    rx->frame=-1;

	Sleep2(100);

	if(!rx->muteAudio){
		try {
		// Stop the stream
			dac.stopStream();
		}
		catch (RtAudioError& e) {
			e.printMessage();
		}
		if (dac.isStreamOpen()) dac.closeStream();
	}
    return 0;

}
int cReceive::initPlay(struct playData *rxi)
{
    
	rx->doWhat=Wait;
	
    rx->witch=0;
    
    rx->audioOut=0;
    
	if(rx->Debug > 1)mprint("    frequency.size %ld\n",(long)frequency.size());

	for (size_t n = 0; n < frequency.size(); n++)
	{
		struct fmultStruct fm;
		double pi;
		double f=frequency[n].frequency;
		pi=4.0*atan(1.0);
		fm.dt=1.0/(double)rx->samplerate;
		fm.sino=0;
		fm.coso=1;
		fm.w=2.0*pi*(rx->fc - f);
		fm.sindt=sin(fm.w*fm.dt);
		fm.cosdt=cos(fm.w*fm.dt);
		if(rx->Debug > 1)mprint("fc %f f %f dt %g samplerate %d\n",rx->fc,f,fm.dt,rx->samplerate);
		fm.gain=1.0;
		fmult.push_back(fm);
	} 
	
	//if(rx->Debug)fprintf(stderr,"fmult.Size() %ld\n",(long)fmult.size());   
    
    if(rx->fc != rx->f){
    	double pi;
    	pi=4.0*atan(1.0);
    	rx->dt=1.0/(double)rx->samplerate;
    	rx->sino=0;
    	rx->coso=1;
    	rx->w=2.0*pi*(rx->fc - rx->f);
    	rx->sindt=sin(rx->w*rx->dt);
    	rx->cosdt=cos(rx->w*rx->dt);
    	if(rx->Debug > 1)mprint("fc %f f %f dt %g samplerate %d\n",rx->fc,rx->f,rx->dt,rx->samplerate);
    }
    
	if(findRadio(rx) || rx->device == NULL){
	    mprint("Error Opening SDR\n");
		return 1;
	}
	
	rx->doWhat = Wait;

	std::thread(&cDemod::rxBuffer, d, (void *)rx).detach(); 

	if(rx->Debug){
		printAudioInfo(rx);
		printInfo(rx);
	}
    
	return 0;
}

cReceive::~cReceive()
{
	//fprintf(stderr,"cReceive::~cReceive\n");
	if(d)delete d;

}

int cReceive::fftIndex(double frequency)
{
    if(!rx)return -1;
    int index=(int)(0.5+rx->FFTcount*((frequency - rx->fc)+0.5*rx->samplerate)/rx->samplerate);
    if(index >= 0 && index < rx->FFTcount-1)return index;
    return -1;
}

void usage()
{
	fprintf(stderr,"Usage:\n");
	fprintf(stderr," sdrReceive.x [options]\n");
	fprintf(stderr," sdrReceive.x -h\n\n");
	fprintf(stderr,"Mode:\n");
	fprintf(stderr,"  -am            Select AM mode\n");
	fprintf(stderr,"  -nam           Select narrow band AM mode\n");
	fprintf(stderr,"  -fm            Select FM mode\n");
	fprintf(stderr,"  -nbfm          Select narrow band FM mode\n");
	fprintf(stderr,"  -usb           Select upper side band\n");
	fprintf(stderr,"  -lsb           Select lower side band\n");
	fprintf(stderr,"  -cw            Select CW mode\n");
	fprintf(stderr,"\nAdjustments:\n");
	fprintf(stderr,"  -gain 0.5                Set volume to one half maximum\n");
	fprintf(stderr,"  -rf_gain 30              Set RF gain to 30\n");
	fprintf(stderr,"  -fc 162.0                Set center frequency to 162.0 MHZ\n");
	fprintf(stderr,"  -f  162.4                Set radio frequency to  162.4 MHZ\n");
	fprintf(stderr,"  -mute                    Set the volume to zero\n");
	fprintf(stderr,"  -samplerate 10e6         Set sample rate to 10 MHZ\n");
	fprintf(stderr,"  -device 2                Use SDR device number two\n");
	fprintf(stderr,"  -audiodevice 1           Use audio output device one\n");
	fprintf(stderr,"  -cutoff -70              Set squelch level to -70 db (background -90)\n");
	fprintf(stderr,"  -pipe                    Pipe the audio output to sdtout\n");
	fprintf(stderr,"  -faudio  48000           Set the audio sample rate to 48000 HZ\n");
	fprintf(stderr,"  -antenna  Hi-z           Use the Hi-z antenna\n");
	fprintf(stderr,"  -x  5                    Skip frequency 5 from the frequency scan list\n");
	fprintf(stderr,"  -binary                  Write I/Q data to stdout\n");
	fprintf(stderr,"\nSetting Info:\n");
	fprintf(stderr,"  -set rfnotch_ctrl true   Turn on notch filter\n");
	fprintf(stderr,"  -set biasT_ctrl true     Turn on voltage to line\n");
	fprintf(stderr,"  -set direct_samp 2       Enable direct sample mode for rtl devices\n");
	fprintf(stderr,"  These values are from the Setting Info list\n");
	fprintf(stderr,"\nExamples:\n");
	fprintf(stderr,"  sdrReceive.x -f 101.5 -fm\n");
	fprintf(stderr,"  sdrReceive.x -f 162.4 -nbfm\n");
	fprintf(stderr,"  sdrReceive.x -f 10 -am\n");
	fprintf(stderr,"  sdrReceive.x -fc 1 -f 0.6 -device 3 -am\n");
	fprintf(stderr,"  sdrReceive.x -fc 854.0 -f 854.36 -f 854.636 -nbfm -samplerate 10e6\n");
	fprintf(stderr,"\nLong Examples:\n");
	fprintf(stderr,"  sdrReceive.x -fc 770 -f 769.31875 -f 769.50625 -f 769.55625 -f 769.75625 -f 769.81875 -f 770.01875 -f 770.25625 -f 770.26875 -f 770.51875 -f 770.75625 -f 770.76875 -f 771.05625 -f 771.06875 -f 771.26875  -nbfm -samplerate 10e6 -print 2 -cutoff -80 -pipe -mute -x 10 -x 7 | dsd -i - -o pa:1 \n");
	fprintf(stderr,"  sdrReceive.x -fc 854.0 -f 854.3600 -f 854.6360 -f 854.6608 -f 854.7360 -f 854.9620 -f 855.0620 -f 855.0860 -f 855.2620 -f 855.5850 -f 855.9120 -f 856.8360 -f 856.8380 -f 856.8860 -f 857.0860 -nbfm -gain 1 -samplerate 10e6\n");
	fprintf(stderr,"  sdrReceive.x -fc 770 -f 769.31875 -f 769.50625 -f 769.55625 -f 769.75625 -f 769.81875 -f 770.01875 -f 770.25625 -f 770.26875 -f 770.51875 -f 770.75625 -f 770.76875 -f 771.05625 -f 771.06875 -f 771.26875  -nbfm -samplerate 10e6 -print 2 -cutoff -80 -pipe -mute -x 10 -x 7 | dsd -i - -o - | play -q -t s16 -r 8k -c 1 -\n");
	fprintf(stderr,"\nI/O Redirection:\n");
	fprintf(stderr,"   bash     > junk1.txt     Redirect stdout to junk1.txt\n");
	fprintf(stderr,"   bash    2> junk2.txt     Redirect stderr to junk2.txt\n");
	fprintf(stderr,"   tch      > junk1.txt     Redirect stdout to junk1.txt\n");
	fprintf(stderr,"   tch     >& junk2.txt     Redirect stderr to junk2.txt\n");
	fprintf(stderr,"End Usage\n");
	
}

cReceive::cReceive(int argc, char * argv [])
{	

	zerol((unsigned char *)&rxs,(&rxs.end-&rxs.start)+1);
	
	rx=&rxs;
	
	rx->samplerate=2000000;
	rx->gain=0.5;
	rx->fc=-1.0;
	rx->f=0.6e6;
    rx->antennaUse=NULL;
    rx->channel=0;
    rx->setcount=0;
    rx->faudio=48000;
    rx->timeout=0;
    rx->timestart=0;
    rx->dumpbyminute=0;
    rx->idump=0;
    rx->PPM=0;
    rx->aminGlobal=0;
    rx->amaxGlobal=0;
    rx->averageGlobal=0;
    rx->decodemode = MODE_AM;
    rx->Debug = 1;
    rx->ncut = 20;
    rx->rf_gain=0;
    rx->cutOFF=-70;
    rx->pipe=0;
    rx->muteAudio=0;
    
    rx->audiodevice=-1;
	rx->deviceNumber=-1;
	rx->binary=0;
  
		
	struct frequencyStruct fs;

	for(int n=1;n<argc;++n){
	    if(!strcmp(argv[n],"-print")){
		   rx->Debug=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-dumpbyminute")){
		   char filename[256];
		   sprintf(filename,"minute-%08d.raw",rx->idump++);
		   mprint("filename %s\n",filename);
		   rx->dumpbyminute = 1;
	    }else if(!strcmp(argv[n],"-binary")){
    		rx->binary=1;
	    }else if(!strcmp(argv[n],"-mute")){
    		rx->muteAudio=1;
	    }else if(!strcmp(argv[n],"-pipe")){
    		rx->pipe=1;
	    }else if(!strcmp(argv[n],"-am")){
		   rx->decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"-nam")){
		   rx->decodemode = MODE_NAM;
	    }else if(!strcmp(argv[n],"-fm")){
		   rx->decodemode = MODE_FM;
	    }else if(!strcmp(argv[n],"-nbfm")){
		   rx->decodemode = MODE_NBFM;
        }else if(!strcmp(argv[n],"-usb")){
            rx->decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"-lsb")){
            rx->decodemode = MODE_LSB;
        }else if(!strcmp(argv[n],"-cw")){
            rx->decodemode = MODE_CW;
	    }else if(!strcmp(argv[n],"-gain")){
	         rx->gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-PPM")){
	         rx->PPM=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-rf_gain")){
	         rx->rf_gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-fc")){
	         rx->fc=atof(argv[++n]);
	         rx->fc *= 1e6;
	    }else if(!strcmp(argv[n],"-f")){
	         rx->f=atof(argv[++n]);
	         rx->f *= 1e6;
	         fs.frequency=rx->f;
	         fs.flag=1;
	         frequency.push_back(fs);
	    }else if(!strcmp(argv[n],"-x")){
	         int nkill=(int)atof(argv[++n]);
	         kill.push_back(nkill);
	    }else if(!strcmp(argv[n],"-channel")){
	         rx->channel=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-file")){
	    }else if(!strcmp(argv[n],"-faudio")){
	         rx->faudio=(float)atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-device")){
	         rx->deviceNumber=atoi(argv[++n]);
	    }else if(!strcmp(argv[n],"-audiodevice")){
	         rx->audiodevice=atoi(argv[++n]);
	    }else if(!strcmp(argv[n],"-samplerate")){
            rx->samplerate=(int)atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-timeout")){
            rx->timeout=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-cutoff")){
            rx->cutOFF=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-h")){
	         usage();
	         exit(0);
	    }else if(!strcmp(argv[n],"-antenna")){
            rx->antennaUse=strsave(argv[++n],9870);
	    }else if(!strcmp(argv[n],"-set")){
	     rx->set[rx->setcount]=argv[++n];
	     rx->value[rx->setcount++]=argv[++n];
	    }else{
	    	mprint("Unknown Command = \"%s\"\n",argv[n]);
			// infilename = argv [n] ;
		}
	}
	
	if(rx->fc < 0)rx->fc=rx->f+20000;
	
	if(rx->binary){
		if(rx->pipe){
			fprintf(stderr,"-binary and -pipe conflict STOP\n");
			exit(1);
		}
		fprintf(stderr,"Binary File Tag\n");
		fprintf(stderr,"atag_IQ_%lu_%lu_fc.raw\n",(long)rx->fc,(long)rx->samplerate);
	}
	
	if(frequency.size() <= 0){
		fs.frequency=rx->f;
	    fs.flag=1;
	    frequency.push_back(fs);
	}
	
	if(kill.size() > 0){
	    sort(kill.begin(), kill.end());
		for(int n=0;n<(int)kill.size();++n){
			int nn=kill[n];
			if(nn >= 0 && nn < (int)frequency.size()){
				frequency[nn].flag=0;
			}
		}
	}
		
	rx->FFTfilter=FILTER_BLACKMANHARRIS7;

	rx->channels=2;
	
	d = new cDemod();
	d->rx=rx;		
	rx->r=this;			
	dd=d;
	
	lineDumpInterval=0.1;
    lineTime=rtime()+lineDumpInterval;
    
    lineAlpha=0.25;
    
    FFTlength=32768;
    rx->FFTcount=4096;
    
    rx->cutOFFSearch=0;
    
    rx->muteScan=0;
    
    
    range=(double *)cMalloc(FFTlength*sizeof(double),9851);
    range3=(double *)cMalloc(FFTlength*sizeof(double),9851);
    magnitude=(double *)cMalloc(FFTlength*sizeof(double),9851);
    magnitude2=(double *)cMalloc(FFTlength*sizeof(double),9851);
    magnitude3=(double *)cMalloc(FFTlength*sizeof(double),9851);

    frequencies=(double *)cMalloc(FFTlength*sizeof(double),9851);
    ampitude=(double *)cMalloc(FFTlength*sizeof(double),9851);
    
    if(!range || !magnitude || !frequencies || !ampitude || !magnitude2 || !range3 || !magnitude3)return;
    
    zerol((char *)range,FFTlength*sizeof(double));
    zerol((char *)range3,FFTlength*sizeof(double));
    zerol((char *)magnitude,FFTlength*sizeof(double));
    zerol((char *)magnitude3,FFTlength*sizeof(double));

    zerol((char *)frequencies,FFTlength*sizeof(double));
    zerol((char *)ampitude,FFTlength*sizeof(double));

	scanRun=0;
    scanWait=0;
    
    pauseTimeDelta=3.0;
    
    pauseTime=rtime();
    
    pauseChannel=0;

	rx->startSound=0;
	
	iplay=0;
	
}

int doWindow(double *x,double *y,long length,int type)
{
    //double w[length];
    double w[32768];
    int i;
    
    if(!x || !y)return 1;
    
    switch(type){
            
        case FILTER_RECTANGULAR:

            for(i=0; i<length; i++)
                w[i] = 1.0;
            
            break;
            

            
        case FILTER_HANN:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hann(i, (int)length);
#else
                w[i]=hann(i, (int)length);
#endif
            }
            break;

            
            
        case FILTER_HAMMING:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hamming(i, (int)length);
#else
                w[i]=hamming(i, (int)length);
#endif
            }
            break;
            
        case FILTER_FLATTOP:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_flattop(i, (int)length);
#else
                w[i]=flattop(i, (int)length);
#endif
            }
            break;
            
            
        case FILTER_BLACKMANHARRIS:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris(i, (int)length);
#else
                w[i]=blackmanharris(i, (int)length);
#endif
            }
            break;
            
        case FILTER_BLACKMANHARRIS7:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris7(i, (int)length);
#else
                w[i]=blackmanharris7(i, (int)length);
#endif
            }
            break;
    }
    
    for(i=0; i<length; i++){
        double amp;
        amp=w[i];
        x[i]=amp*x[i];
        y[i]=amp*y[i];
    }
    
    return 0;
    
}


int cReceive::printAudioInfo(struct playData *rx)
{
	RtAudio dac;
	
	
	if(rx->muteAudio)return 0;

	int deviceCount=dac.getDeviceCount();

	if (deviceCount < 1 ) {
		mprint("\nNo audio devices found!\n");
		exit(0);
	}

	mprint("\nAudio device Count %d default output device %d audiodevice %d\n",deviceCount,dac.getDefaultOutputDevice(),rx->audiodevice);

	RtAudio::DeviceInfo info;
	for (int i=0; i<deviceCount; i++) {

		try {
			info=dac.getDeviceInfo(i);
			if(info.outputChannels > 0){
			// Print, for example, the maximum number of output channels for each device
				mprint("audio device = %d : output  channels = %d Device Name = %s",i,info.outputChannels,info.name.c_str());
				if(info.sampleRates.size()){
					mprint(" sampleRates = ");
					for (int ii = 0; ii < (int)info.sampleRates.size(); ++ii){
						mprint(" %d ",info.sampleRates[ii]);
				   }
				}
				mprint("\n");
			 }
 
			if(info.inputChannels > 0){
			// Print, for example, the maximum number of output channels for each device
				mprint("audio device = %d : input   channels = %d Device Name = %s",i,info.inputChannels,info.name.c_str());
				 if(info.sampleRates.size()){
					mprint(" sampleRates = ");
					for (int ii = 0; ii < (int)info.sampleRates.size(); ++ii){
						mprint(" %d ",info.sampleRates[ii]);
				   }
				}
				mprint("\n");
		   }

		}
		catch (RtAudioError &error) {
			error.printMessage();
			break;
		}

	}

	mprint("\n");

	return 0;
}
int cReceive::printInfo(struct playData *rx)
{
	mprint("%s\n","SoapySDR Library");
	
	mprint("Lib Version: v%s\n",SoapySDR::getLibVersion().c_str());
	mprint("API Version: v%s\n",SoapySDR::getAPIVersion().c_str());
	mprint("ABI Version: v%s\n",SoapySDR::getABIVersion().c_str());
	mprint("Install root:  %s\n",SoapySDR::getRootPath().c_str());
    
    std::vector<std::string> path=SoapySDR::listSearchPaths();
    for(size_t i=0;i<path.size();++i){
 	    mprint("Search path:  %s\n",path[i].c_str());
   }

    std::vector<std::string> mod=SoapySDR::listModules();
    
    for (size_t k=0;k<mod.size();++k)
    {
   	    mprint("Module found: %s ",mod[k].c_str());
      /*
        const auto &errMsg = SoapySDR::loadModule(mod[k]);
        if (not errMsg.empty())mprint("Module found: %s ",errMsg.c_str());
        */
    	mprint("\n");
    
    }
    if (mod.empty())mprint("No modules found!\n");
    
    mprint("\n");

    return 0;
}
int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
	return dd->sound(outputBuffer,inputBuffer,nBufferFrames,streamTime,status,userData);
}

int Process1(void *rxv);

int Process2(void *rxv);

int Process(void *rxv)
{
	return Process2(rxv);
}

int Process2(void *rxv)
{
	struct playData *rx=(struct playData *)rxv;
	
	class cDemod d;
	
	return d.process(rx);
}

int cDemod::process(struct playData *rxi)
{
	rx=rxi;	
	
	zerol((char *)&f,sizeof(f));
	
	setFilters(rx,&f);

	bAudio=(float *)cMalloc((size_t)(2*rx->faudio*4),9837);
    if(!bAudio){
    	rx->r->mprint("3 cMalloc Errror %ld\n",(long)(2*rx->faudio*4));
       	return 1;
    }
    

	bRf=(float *)cMalloc(2*rx->size*4,4567);
    if(!bRf){
    	rx->r->mprint("5 cMalloc Errror %ld\n",(long)(2*rx->size*4));
       	return 1;
    }
    zerol((char *)bRf,2*rx->size*4);
	
	Sleep2(1000);
	
	rx->cutOFFSearch=1;
	for(int n=0;n<rx->r->FFTlength;++n){
		rx->r->frequencies[n]=0;
		rx->r->ampitude[n] = -160;
	}

		
	while(rx->frame >= 0){
		processAll();
		rx->r->updateLine();
		Sleep2(5);
	}
	
	rx->r->iplay=0;
	
	rx->r->processScan(rx);
	
	rx->frame=-1;

	return 0;
}



void cDemod::processAll()
{

	int ip=-1;
	if(rx->cs){
		ip=rx->cs->popBuff(rx);
	}
	if(ip < 0){
	 	return;
	}
		
	int witch=ip % NUM_DATA_BUFF;
	
	float *buf=rx->buff[witch];
	 
	float *buf2=bRf;
	
	double sint,cost;

 //fprintf(stderr,"rx->r->fmult.size() %ld\n",(long)rx->r->fmult.size());
 
 
		
	
	if(rx->r->iplay == 1){
		for (int k = 0 ; k < rx->size ; k++){
			float r = buf[k * 2];
			float i = buf[k * 2 + 1];
			if(rx->dt > 0){
				buf2[k * 2] = (float)(r*rx->coso - i*rx->sino);
				buf2[k * 2 + 1] = (float)(i*rx->coso + r*rx->sino);				
				sint=rx->sino*rx->cosdt+rx->coso*rx->sindt;
				cost=rx->coso*rx->cosdt-rx->sino*rx->sindt;
				rx->coso=cost;
				rx->sino=sint;
			 }else{
				buf2[k * 2] = r;
				buf2[k * 2 + 1] = i;
			}
		}
		  
		double r=sqrt(rx->coso*rx->coso+rx->sino*rx->sino);
		rx->coso /= r;
		rx->sino /= r;
    }else{
		for (int k = 0 ; k < rx->size ; k++){
			buf2[k * 2] = 0;
			buf2[k * 2 + 1] = 0;
		}
    }
    

	buf=bAudio;
		
    unsigned int num;
    unsigned int num2;
    
    num=0;
    num2=0;
    
    msresamp_crcf_execute(f.iqSampler, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf, &num);  // decimate
        
    if(rx->decodemode < MODE_AM){

		freqdem_demodulate_block(f.demod, (liquid_float_complex *)buf, (int)num, (float *)buf2);

    }else if(rx->decodemode < MODE_USB){
        #define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

        for(unsigned int n=0;n<num;++n){
            double mag=sqrt(buf[2*n]*buf[2*n]+buf[2*n+1]*buf[2*n+1]);
            double z0=mag + (f.amHistory * DC_ALPHA);
            buf2[n]=(float)(z0-f.amHistory);
            f.amHistory=z0;
        }
    }else{
        ampmodem_demodulate_block(f.demodAM,  (liquid_float_complex *)buf, (int)num, (float *)buf2);
   }
   
   msresamp_rrrf_execute(f.iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
   
/*
	if(rx->decodemode == MODE_NBFM){
		for(unsigned int n=0;n<num2;++n){
			double mag=buf[n];
			double z0=mag + (f.amHistory * DC_ALPHA);
			buf[n]=(float)(z0-f.amHistory);
			f.amHistory=z0;
		}
	}
*/

	float *buff=bAudio;

	int short *data;
	
	int audioOut;


	rx->mutexo.lock();
	audioOut=rx->audioOut;
	//mprint("audioOut %d\n",audioOut);
	data=rx->buffa[rx->audioOut++ % NUM_ABUFF];
	rx->mutexo.unlock();

	double amin=1e30;
	double amax=-1e30;
	double average=0;


	double dnom,gain;

	gain=rx->gain;

	if(gain <= 0.0)gain=1.0;


	for (int i=0; i<rx->faudioCount; i++ ) {
		double v;
		v=buff[i];
		average += v;
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}
	

	average /= rx->faudioCount;
		
	//mprint("1 doAudio faudioCount %f amin %f amax %f audioOut %d ",rx->faudioCount,amin,amax,audioOut);
	//mprint(" aminGlobal %f amaxGlobal %f average %g\n",rx->aminGlobal ,rx->amaxGlobal,average);
	
	double tt=0.8;

/*
	if(rx->averageGlobal == 0.0)rx->averageGlobal=average;
	rx->averageGlobal = tt*rx->averageGlobal+(1.0-tt)*average;
	average=rx->averageGlobal;
*/

	amin -= average;

	amax -= average;

	if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;

	rx->aminGlobal = tt*rx->aminGlobal+(1.0-tt)*amin;

	amin=rx->aminGlobal;

	if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;

	rx->amaxGlobal = tt*rx->amaxGlobal+(1.0-tt)*amax;

	amax=rx->amaxGlobal;


	//mprint("3 doAudio faudioCount %f amin %f amax %f audioOut %d\n",rx->faudioCount,amin,amax,audioOut);

	if((amax-amin) > 0){
		dnom=65535.0/(amax-amin);
	}else{
		dnom=65535.0;
	}


	for(int k=0;k<rx->faudioCount;++k){
		double v;

		v=buff[k];

		v=gain*((v-average)*dnom);

		if(v < -32765){
			v = -32765;
		}else if(v > 32765){
			v=32765;
		}

		data[k]=(short int)v;
	}	

	if(rx->cs){
		rx->cs->pushBuffa(audioOut,rx);
   	}
   
}
int doAudio(float *aBuff,struct playData *rx);
int doFilter(struct playData *rx,float *wBuff,float *aBuff,struct Filters *f);

int Process1(void *rxv)
{
	struct playData *rx=(struct playData *)rxv;
	
	struct Filters f;
	
	zerol((char *)&f,sizeof(f));

	f.thread=rx->thread++;
	
	setFilters(rx,&f);
	
	float *wBuff=(float *)cMalloc(2*rx->size*4,4567);
    if(!wBuff){
        fprintf(stderr,"2 cMalloc Errror %ld\n",(long)(2*rx->size*4));
       	 return 1;
    }
    zerol((char *)wBuff,2*rx->size*4);

	//mprint("Process Start rx->frame %d\n",rx->frame);
	
	float *aBuff=(float *)cMalloc((size_t)(2*rx->faudio*4),9837);
    if(!aBuff){
        fprintf(stderr,"3 cMalloc Errror %ld\n",(long)(2*rx->faudio*4));
       	 return 1;
    }
    zerol((char *)aBuff,(unsigned long)(2*rx->faudio*4));
	
	while(rx->frame >= 0){
		if(doFilter(rx,wBuff,aBuff,&f)){
			Sleep2(5);
		}else{
			rx->r->updateLine();
			doAudio(aBuff,rx);
		}
	}
	//mprint("Process return rx->frame %d\n",rx->frame);
	
	if(wBuff)cFree((char *)wBuff);
	
	if(aBuff)cFree((char *)aBuff);
	
	if (f.iqSampler)msresamp_crcf_destroy(f.iqSampler);
	
	if (f.iqSampler2)msresamp_rrrf_destroy(f.iqSampler2);
	
	
    if(f.fShift)nco_crcf_destroy(f.fShift);
    
    if(f.demod)freqdem_destroy(f.demod);
    
    if(f.demodAM)ampmodem_destroy(f.demodAM);
    
    rx->frame=-1;

	return 0;
}
int doFilter(struct playData *rx,float *wBuff,float *aBuff,struct Filters *f)
{
	int ip= -1;
	if(rx->cs){
 		ip=rx->cs->popBuff(rx);
	}
 	if(ip < 0){
 	// mprint("wait thread %d\n",f->thread);
 	     return 1;
 	}
 	
 	//mprint("ip %d thread %d\n",ip,f->thread);
 	
 	int witch=ip % NUM_DATA_BUFF;
 	
 //	printf("doFilter witch %d ip %d thread %d\n",witch,ip,f->thread);
 	
 	//mprint("doFilter witch %d ip %d start \n",witch,ip);
	
 	
 	// mprint("shift %f size %d fShift %p\n",shift,rx->size,rx->fShift);
 /*
 
 	float shift=rx->fc-rx->f;
 
	if (shift != 0) {
		if (shift > 0) {
			nco_crcf_mix_block_up(f->fShift, (liquid_float_complex *)rx->buff[witch], (liquid_float_complex *)wBuff, rx->size);
		} else {
			nco_crcf_mix_block_down(f->fShift, (liquid_float_complex *)rx->buff[witch], (liquid_float_complex *)wBuff,rx->size);
		}
	 }
*/


		float *buf=rx->buff[witch];
		float *buf2=wBuff;
				
		double sint,cost;
		

	if(rx->r->iplay){
        for (int k = 0 ; k < rx->size ; k++){
            float r = buf[k * rx->channels];
            float i = buf[k * rx->channels + 1];
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
    }else{
        for (int k = 0 ; k < rx->size ; k++){
			buf2[k * rx->channels] = 0;
			buf2[k * rx->channels + 1] = 0;
        }
    
    }
        
    double r=sqrt(rx->coso*rx->coso+rx->sino*rx->sino);
    rx->coso /= r;
    rx->sino /= r;
      

	//float *buf=aBuff;
	buf=aBuff;
	
//	float *buf2=rx->wBuff;
	
    unsigned int num;
    unsigned int num2;
    
    num=0;
    num2=0;
    
    msresamp_crcf_execute(f->iqSampler, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf, &num);  // decimate
        
    if(rx->decodemode < MODE_AM){

		freqdem_demodulate_block(f->demod, (liquid_float_complex *)buf, (int)num, (float *)buf2);

    }else if(rx->decodemode < MODE_USB){
        #define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

        for(unsigned int n=0;n<num;++n){
            double mag=sqrt(buf[2*n]*buf[2*n]+buf[2*n+1]*buf[2*n+1]);
            double z0=mag + (f->amHistory * DC_ALPHA);
            buf2[n]=(float)(z0-f->amHistory);
            f->amHistory=z0;
        }
    }else{
        ampmodem_demodulate_block(f->demodAM,  (liquid_float_complex *)buf, (int)num, (float *)buf2);
   }

   msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
   
   // printf("num %d num2 %d faudioCount %g\n",num,num2,rx->faudioCount);
      
	return 0;
}
int doAudio(float *aBuff,struct playData *rx)
{
	int short *data;
	int audioOut;

	
	rx->mutexo.lock();
	audioOut=rx->audioOut;
	//mprint("audioOut %d\n",audioOut);
	data=rx->buffa[rx->audioOut++ % NUM_ABUFF];
	rx->mutexo.unlock();

	

	double amin=1e30;
	double amax=-1e30;
		
	float *buff=aBuff;

	
	double dnom,gain;
	
	gain=rx->gain;
	
	if(gain <= 0.0)gain=1.0;
	
	double average=0;
	
	for (int i=0; i<rx->faudioCount; i++ ) {
		double v;
		v=buff[i];
        average += v;
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}
	
	average /= rx->faudioCount;
	
    amin -= average;

    amax -= average;
	

    if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;

    rx->aminGlobal = 0.8*rx->aminGlobal+0.2*amin;

    amin=rx->aminGlobal;

    if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;

    rx->amaxGlobal = 0.8*rx->amaxGlobal+0.2*amax;

    amax=rx->amaxGlobal;


	//mprint("doAudio size %f amin %f amax %f audioOut %d\n",rx->faudioCount,amin,amax,audioOut);
	
	
    if((amax-amin) > 0){

        dnom=65535.0/(amax-amin);
    }else{

        dnom=65535.0;
    }
		

	for(int k=0;k<rx->faudioCount;++k){
		double v;

        v=buff[k];

		v=gain*((v-average)*dnom);

        if(v < -32765){
            v = -32765;
        }else if(v > 32765){
            v=32765;
        }

		data[k]=(short int)v;
	}	

	if(rx->cs){
	    rx->cs->pushBuffa(audioOut,rx);
	}

	return 0;
}
int cReceive::findRadio(struct playData *rx)
{
    
    std::string argStr;
        
    std::vector<SoapySDR::Kwargs> results;
    
    results = SoapySDR::Device::enumerate();
    
    mprint("Number of Devices Found: %ld\n",(long)results.size());
    
    if(results.size() < 1)return 1;
    
    rx->device = NULL;
    
    SoapySDR::Kwargs deviceArgs;
    
    for(unsigned int k=0;k<results.size();++k){
		deviceArgs = results[k];
		for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
		    if(it->first == "driver"){
				// mprint("SDR device =  %ld ",(long)k);
			    // mprint(" %s = %s\n",it->first.c_str(), it->second.c_str());
		        if(it->second == "audio")break;
		    }
			if (it->first == "label"){
				mprint("SDR device =  %ld ",(long)k);
			    mprint(" %s = %s\n",it->first.c_str(), it->second.c_str());
			    if(rx->deviceNumber < 0)rx->deviceNumber=k;
			}
		}
    }
    
    mprint("\n");
    
    for(int k=0;k<(int)results.size();++k){
    
    	if(k == rx->deviceNumber){
        
			deviceArgs = results[k];
		
	    		mprint("device =  %ld selected\n",(long)k);

			for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
				mprint("%s = %s ",it->first.c_str(), it->second.c_str());
				if (it->first == "driver") {
					//dev->setDriver(it->second);
				} else if (it->first == "label" || it->first == "device") {
					//dev->setName(it->second);
				}
			}
		
    		mprint("\n\n");

			rx->device = SoapySDR::Device::make(deviceArgs);
			
			mprint("driver = %s\n",rx->device->getDriverKey().c_str());
			mprint("hardware = %s\n",rx->device->getHardwareKey().c_str());
        
        
			SoapySDR::Kwargs it=rx->device->getHardwareInfo();
		
			for( SoapySDR::Kwargs::iterator  ii=it.begin(); ii!=it.end(); ++ii)
			{
					mprint("%s = %s ",ii->first.c_str(), ii->second.c_str());
			}

			
    		mprint("\n\n");
			
			        //query device info
        	std::vector<std::string> names = rx->device->listAntennas(SOAPY_SDR_RX,  rx->channel);
        	mprint("Rx antennas: \n",(long)k);

        
        	for (std::vector<std::string>::const_iterator ii = names.begin(); ii != names.end(); ++ii){
       			mprint("%s\n",(*ii).c_str());
        	}
			        
        	rx->antennaCount=(int)names.size();
        	rx->antenna=(char **)cMalloc((unsigned long)(rx->antennaCount*sizeof(char *)),8833);
        	for (size_t i=0;i<names.size();++i){
            	rx->antenna[i]=strsave((char *)names[i].c_str(),5555);
        	}

			
        	rx->r->mprint("\n");
        	
        	mprint("Setting Info: \n");
						
        	SoapySDR::ArgInfoList args = rx->device->getSettingInfo();
        	if (args.size()) {
            	for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
                	SoapySDR::ArgInfo arg = (*args_i);

                	mprint("key %s value %s read %s type %d min %g max %g step %g\n",arg.key.c_str(),arg.value.c_str(),rx->device->readSetting(arg.key).c_str(),
                       	(int)arg.type,arg.range.minimum(),arg.range.maximum(),arg.range.step());

            	}
        	}
        	mprint("\n");
        	
			
	        if(rx->setcount){          
             	mprint("setcount %d\n",rx->setcount);
           		for(int k=0;k<rx->setcount;++k){
              		mprint("%s %s\n",rx->set[k].c_str(),rx->value[k].c_str());
               		rx->device->writeSetting(rx->set[k],rx->value[k]);
                }
        		mprint("\n");
            }
                
	
            if(rx->antennaUse){
            	mprint("Use antenna \"%s\"\n",rx->antennaUse);
            	rx->device->setAntenna(SOAPY_SDR_RX, rx->channel, rx->antennaUse);
            }
            
                
        	SoapySDR::Range range=rx->device->getGainRange(SOAPY_SDR_RX, rx->channel);
        
            mprint("RF Gain range RX min %g max %g \n",range.minimum(),range.maximum());
            
            if(rx->rf_gain > 0){
              	mprint("RF Gain set to %g \n",rx->rf_gain);
              	rx->device->setGain(SOAPY_SDR_RX, rx->channel,rx->rf_gain);
            }
   
            SoapySDR::RangeList rlist=rx->device->getFrequencyRange(SOAPY_SDR_RX, rx->channel);
            
        	for (size_t j = 0; j < rlist.size(); j++)
        	{
         	    mprint("FrequencyRange min %g max %g \n",rlist[j].minimum(),rlist[j].maximum());
        	}

        	std::vector<double> band=rx->device->listBandwidths(SOAPY_SDR_RX, rx->channel);
        	if(band.size()){
                mprint("\nBandwidth MHZ ");  		
				for (size_t j = 0; j <band.size(); j++)
				{
				   mprint(" %.2f ",band[j]/1.0e6);
				}
				mprint("\n\n");
            }
            

			std::vector<double> rate=rx->device->listSampleRates(SOAPY_SDR_RX, rx->channel);
        	if(rate.size()){
                 mprint("SampleRates MHZ ");
      		}
			for (size_t j = 0; j < rate.size(); j++)
        	{
           		mprint(" %.6f ",rate[j]/1.0e6);
         	}
            mprint("\n\n");

            mprint("Gains: \n");  		
			names=rx->device->listGains( SOAPY_SDR_RX, rx->channel);
			for (size_t j = 0; j < names.size(); j++)
			{
				mprint("%lu %s ",j, names[j].c_str());
			
				SoapySDR::Range range3=rx->device->getGainRange(SOAPY_SDR_RX, rx->channel, names[j].c_str());
				mprint("range max %g min %g\n",range3.maximum(),range3.minimum());
			}

           mprint("\n");

			//rx->device->setFrequency(SOAPY_SDR_RX, rx->channel, rx->fc);
			
			rx->device->setFrequency(SOAPY_SDR_RX,rx->channel,"RF",rx->fc);
				
			rx->device->setSampleRate(SOAPY_SDR_RX, rx->channel, rx->samplerate);
			
			
        	mprint("rx->samplerate %d\n",rx->samplerate);
			
			//const std::vector<size_t> channels = {(size_t)0,(size_t)1};
			
			const std::vector<size_t> channels = {(size_t)0};
						
			rx->rxStream = rx->device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, channels);

			rx->device->activateStream(rx->rxStream, 0, 0, 0); 

        	mprint("getGainMode: %d\n",rx->device->getGainMode(SOAPY_SDR_RX, rx->channel));
			
			int hasFrequencyCorrection= rx->device->hasFrequencyCorrection(SOAPY_SDR_RX, rx->channel);
			
        	mprint("hasFrequencyCorrection: %d\n",hasFrequencyCorrection);
			
			if(hasFrequencyCorrection && rx->PPM){
			    rx->device->setFrequencyCorrection(SOAPY_SDR_RX, rx->channel,rx->PPM);
			}
             
		}
    
}
    
    return 0;
    
}

int setFilters(struct playData *rx,struct Filters *f)
{
    
    if(!rx)return 0;
    
    float As = 60.0f;
    
    float ratio=(float)(rx->faudio / rx->samplerate);
    
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
    
    ratio= (float)(rx->faudio/rx->bw);
    
    if(rx->Debug)fprintf(stderr,"rx->Ratio %g ratio %g\n",rx->Ratio,ratio);
    
    f->demod=freqdem_create(0.5);
    
#ifdef LIQUID_VERSION_4
    f->demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
 #else
    f->demodAM = ampmodem_create(0.5, mode, iflag);
#endif

    f->iqSampler  = msresamp_crcf_create(rx->Ratio, As);
    
    f->iqSampler2 = msresamp_rrrf_create(ratio, As);
    
    //msresamp_crcf_print(f->iqSampler);
    
    
    f->fShift = nco_crcf_create(LIQUID_NCO);
    
    f->amHistory=0;
    
    return 0;
    	
}



double rtime(void)
{
	long milliseconds;
	long Seconds;
	double ret;


	GetTime(&Seconds, &milliseconds);

	ret = (double)Seconds + (double)milliseconds / 1000.;

	return ret;

}
static int GetTime(long *Seconds, long *milliseconds)
{
	struct timeb t;

	if (!Seconds || !milliseconds)return 1;


	ftime(&t);

	*Seconds = (long)t.time;
	*milliseconds = t.millitm;

	return 0;
}


int zerol(unsigned char *s,unsigned long n)
{
    if(!s || (n <= 0))return 1;


	while(n-- > 0)*s++ = 0;
	
	return 0;
}

int mstrncpy(char *out,char *in,long n)
{
	if(!out || !in || (n <= 0))return 1;
	
	while(n-- > 0){
	    if(*in == 0){
			*out = 0;
			break;
	    }else{
			*out++ = *in++;
	    }
	}
	
	return 0;
}
char *strsave(char *s,int tag)
{
	long length;
	char *p;
	
	if(!s)return NULL;
	
	length=(long)strlen(s)+1;
	
	if((p=(char *)cMalloc(length+1,tag)) != NULL)
		mstrncpy(p,s,length);
	return(p);
}

int doFFT2(double *x,double *y,long length,int direction)
{
	double *datar;
	long n,n2;
	int ifound;
	
	if(!x || !y)return 1;
	
	n2=2;
	ifound=FALSE;
	for(n=0;n<29;++n){
		if(length == n2){
			ifound=TRUE;
			break;
		}
		n2 *= 2;
	}
	
	if(!ifound){
	    fprintf(stderr,"doFFT Did not find power of 2 length %ld\n",length);
	    return 1;
	}
	
	datar=(double *)cMalloc(2*length*sizeof(double),9092);
	if(!datar){
	    fprintf(stderr,"doFFT out of Memory\n");
	    return 1;
	}
	
	for(n=0;n<length;++n){
		datar[2*n]=x[n];
		datar[2*n+1]=y[n];
	}
	
	fft(datar,(int)length,direction);
	
	for(n=0;n<length;++n){
		x[n]=datar[2*n];
		y[n]=datar[2*n+1];
	}
	
	if(datar)cFree((char *)datar);
	
	return 0;
	
}
int fft(double *data,int nn,int isign)
{
	double twopi,tempr,tempi,wstpr,wstpi;
	double wr,wi,theta,sinth,fni;
	int i,j,n,m,mmax,istep;
	
      data -= 1;
      j=1;
      n=2*nn;
      twopi=8.*atan(1.);
       for(i=1;i<=n;i += 2){
       if(i-j >= 0)goto L200;
       tempr=data[j];
       tempi=data[j+1];
       data[j]=data[i];
       data[j+1]=data[i+1];
       data[i]=tempr;
       data[i+1]=tempi;
L200:    m=n/2;
L300:    if(j-m > 0)goto L400;
		goto L500;
L400:    j=j-m;
       m=m/2;
       if(m-2 >= 0)goto L300;
L500:  j=j+m;
       }
      mmax=2;
L600:   if(mmax-n >= 0)goto L1000;
	  istep=2*mmax;
      theta=twopi/(double)(isign*mmax);
      sinth=sin(theta/2.);
      wstpr=-2.*sinth*sinth;
      wstpi=sin(theta);
      wr=1.;
      wi=0.;
		for(m=1;m<=mmax;m+=2){
			for( i=m;i<=n;i+=istep){
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i]=data[i]+tempr;
				data[i+1]=data[i+1]+tempi;
			}
			tempr=wr;
			wr=wr*wstpr-wi*wstpi+wr;
			wi=wi*wstpr+tempr*wstpi+wi;
		}
      mmax=istep;
      goto L600;
L1000: 

	if(isign > 0){
		fni=2.0/(double)nn;
	}else{
		fni=0.5;
	}
	for( i=1;i<=2*nn;++i){
	    data[i]=data[i]*fni;
	}
	return 0;
}


//ALvoid DisplayALError(unsigned char *szText, ALint errorCode);


//static void list_audio_devices(const ALCchar *devices);
//static void list_audio(void);


/*
static void list_audio()
{

    ALboolean enumeration;
    
    enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if (enumeration == AL_FALSE){
        mprint("enumeration not supported\n");
    }else{
        list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
        list_audio_devices(alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));       
    }

}
static void list_audio_devices(const ALCchar *devices)
{
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;

    mprint( "OpenAL Devices list:\n");
    mprint( "----------\n");

    while (device && *device != '\0' && next && *next != '\0') {
        mprint( "%s\n", device);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
    mprint( "----------\n\n");
}
*/

	
	