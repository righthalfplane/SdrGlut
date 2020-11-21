#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif


#ifndef  _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS  1
#endif

#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>


#include <cstdio>
#include <chrono>

#include <complex>

#include <iostream>

#include <string.h>

#include <stdio.h>

#include <stdlib.h>

#include <liquid/liquid.h>

#include <rtaudio/RtAudio.h>

// #include <unistd.h>


using namespace std;

//g++ -O2 -o Race Race.cpp mThread.cpp  cMalloc.c -lliquid -lrtaudio -lpthread -lSoapySDR

//Race -fc 1e6 -f 0.76e6 -gain 1

//Race -fc 162.0e6 -f 162.4e6 -nbfm -gain 1

//Race -fc 103.0e6 -f 103.7e6 -fm -gain 1

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6
#define MODE_NAM2 7



#include <stdlib.h>

#include "SocketDefs.h"

#include "mThread.h"

void *cMalloc(unsigned long r, int tag);

char *strsave(char *s,int tag);

int findRadio(class Listen *rx);


struct SoapyNetSDR_SocketInit
{
    SoapyNetSDR_SocketInit(void)
    {
        #ifdef _MSC_VER
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif
    }
    ~SoapyNetSDR_SocketInit(void)
    {
        #ifdef _MSC_VER
        WSACleanup();
        #endif
    }
};


 struct Filters2{
    int np;
    ampmodem demodAM;
    freqdem demod;
    msresamp_crcf iqSampler;
    msresamp_rrrf iqSampler2;
    int thread;
    double amHistory;
};


class Listen {
    
public:
    int wShift;
    int Debug;
    float gain;
    double fc;
    double f;
    double dt;
    double sino;
    double coso;
    double sindt;
    double cosdt;
    double w;
    int channels;
    int size;
    volatile int ibuff;
	complex<float> *output;
	complex<float> *buff1;
	float Ratio;
	double fOut;
    int samplerate;
    double aminGlobal;
    double amaxGlobal;
	double bw;
	long ncommand;
	
    Listen();
    ~Listen();
    
    SOCKET connectToServer(char *serverName,unsigned short *Port);

	int getPortAndName(char *in,unsigned int *hostAddr,unsigned short *Port);
	
	SOCKET createService(unsigned short *Port);
	
	int CheckSocket(SOCKET serverSocket,int *count,int ms);
	
	int ListenSocket(SOCKET clientSocket);
	
	int netRead(SOCKET clientSocket,char *buff,long n);
	
	int readCommand(SOCKET clientSocket,char *buff,long *size);
	
	int getLong(SOCKET clientSocket,long *n);

	int readString(SOCKET clientSocket,char *buff,long nbyte);

	SOCKET waitForService(char *name);
	
	int setCenterFrequency(double frequency,double sampleRate);
	
	int setFrequency(double frequency);

	SOCKET startService(char *name);
	
	int mix(float *buf1,float *buf2);

	unsigned int hostAddr;

	long Bytes;

	unsigned short Port;
	
	SOCKET serverSocket;

	SOCKET clientSocket;

	struct sockaddr_in clientSocketAddr;
	
	socklen_t addrLen;
	
	socklen_t namelen;

	long buffsize;
	
	int decodemode;

	struct Filters2 filter;

	SoapyNetSDR_SocketInit socket_init;

	int pipe;
	
	int binary;
	
	SoapySDR::Stream *rxStream;
	SoapySDR::Device *device;

    int antennaCount;
    char **antenna;
    char *antennaUse;
    int channel;
    
    unsigned int deviceNumber;

 	std::string set[20];
 	std::string value[20];
 	int setcount;


};

static int copyl(char *p1,char *p2,long n);

static int zerol(char *p,long n);

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );

int launchThread(void *data,int (*sageThread)(void *data));

int ListenSocket(void *rxv);

int rxBuffer(void *rxv);

static int freeFilters(struct Filters2 *f);

static int setFilters(class Listen *rx,struct Filters2 *f);

Listen::Listen()
{
	ncommand=0;
	Debug=0;
	buffsize=0;
    fc = 1e6;
    f  = 760000;
    channels=2;
    output=NULL;
    buff1=NULL;
    ibuff=-1;
    fOut=48000;
    aminGlobal=0.0;
    amaxGlobal=0.0;
    
    gain=0.5;
    
    decodemode = MODE_AM;

    Port=3700;	
	
    filter.iqSampler=0;

    filter.iqSampler2=0;

    filter.demod=0;

    filter.demodAM=0;
    	
	pipe=0;
	
	binary=0;
	
	samplerate=2000000;
	
	deviceNumber=0;
	
}

Listen::~Listen()
{
	closesocket(clientSocket);
}

int main(int argc,char *argv[])
{

    class Listen *l=new Listen;
    
    int audiodevice=-2;
    
	for(int n=1;n<argc;++n){
	    if(!strcmp(argv[n],"-debug")){
		   l->Debug = 1;
	    }else if(!strcmp(argv[n],"-am")){
		   l->decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"-fm")){
		   l->decodemode = MODE_FM;
        }else if(!strcmp(argv[n],"-nbfm")){
            l->decodemode = MODE_NBFM;
        }else if(!strcmp(argv[n],"-usb")){
            l->decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"-lsb")){
            l->decodemode = MODE_LSB;
	    }else if(!strcmp(argv[n],"-gain")){
	         l->gain=(float)atof(argv[++n]);
        }else if(!strcmp(argv[n],"-f")){
            l->f=atof(argv[++n]);
        }else if(!strcmp(argv[n],"-p")){
            l->Port=(unsigned int)atof(argv[++n]);
        }else if(!strcmp(argv[n],"-audiodevice")){
            audiodevice=(int)atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-device")){
	         l->deviceNumber=atoi(argv[++n]);
        }else if(!strcmp(argv[n],"-pipe")){
            l->pipe=1;
        }else if(!strcmp(argv[n],"-binary")){
            l->binary=1;
		}
	}
       
	RtAudio dac;
	
	int deviceCount=dac.getDeviceCount();
		
	if (deviceCount  < 1 ) {
		fprintf(stderr,"\nNo audio devices found!\n");
		return 1;
	}
	
	fprintf(stderr,"deviceCount %d default output device %d\n",deviceCount,dac.getDefaultOutputDevice());
	
    RtAudio::DeviceInfo info;
    for (int i=0; i<deviceCount; i++) {
        
        try {
            info=dac.getDeviceInfo(i);
            if(info.outputChannels > 0){
            // Print, for example, the maximum number of output channels for each device
                fprintf(stderr,"device = %d : maximum output  channels = %d Device Name = %s\n",i,info.outputChannels,info.name.c_str());
             }
             
            if(info.inputChannels > 0){
            // Print, for example, the maximum number of output channels for each device
                fprintf(stderr,"device = %d : maximum output  channels = %d Device Name = %s\n",i,info.inputChannels,info.name.c_str());
            }

        }
        catch (RtAudioError &error) {
            error.printMessage();
            break;
        }
        
    }

	RtAudio::StreamParameters parameters;
	
	if(audiodevice == -2){
	    parameters.deviceId = dac.getDefaultOutputDevice();
	}else{
	    parameters.deviceId = audiodevice;
	}
	parameters.nChannels = 1;
	parameters.firstChannel = 0;
	unsigned int sampleRate = 48000;
	//unsigned int bufferFrames = 4096;
	unsigned int bufferFrames = 4800/2;


	try {
		dac.openStream( &parameters, NULL, RTAUDIO_SINT16,
						sampleRate, &bufferFrames, &sound, (void *)l);
		dac.startStream();
	}
	catch ( RtAudioError& e ) {
		e.printMessage();
		exit( 0 );
	}
    	
    	
    if(findRadio(l) || l->device == NULL){
	    fprintf(stderr,"Error Opening SDR\n");
		return 1;
	}

    l->ibuff=-2;
	launchThread((void *)l,rxBuffer);   	
    	
/*
	SOCKET ret = l->waitForService(argv[1]);
	if(ret < 0){
		return 1;
	}	

	l->ibuff=-2;
	launchThread((void *)l,ListenSocket);
*/


	while(l->ibuff != -1){
		Sleep2(10);
	}

	try {
    	// Stop the stream
    	dac.stopStream();
  	}
  	catch (RtAudioError& e) {
    	e.printMessage();
  	}
  	
  	if ( dac.isStreamOpen() ) dac.closeStream();



    delete l;

	return 0;
}
int rxBuffer(void *rxv)
{

    class Listen *rx=(class Listen *)rxv;


	rx->setFrequency(rx->f);
	rx->setCenterFrequency(rx->fc,rx->samplerate);
	
	rx->buff1=(complex<float> *)cMalloc(rx->size*8,5646);
	rx->buffsize=rx->size;
	
	rx->output=(complex<float> *)cMalloc(rx->size*8,4567);


	int doWhat=2;


	while(1)
	{
	     switch(doWhat){
	     case 0:
	     	;
	        break;
	     case 1:
	        fprintf(stderr,"Exit rxBuffer\n");
	        return 0;
		 case 2:
	       // fprintf(stderr,"rxBuffer case 2\n");
	       	        
        	long long timeNs=0;
           
            float *buff=(float *)rx->buff1;
             
            void *buffs[] = {buff};
            
            int toRead=rx->size;
            
            
            int count=0;
                 
            while(doWhat == 2){
				int flags=0;
				
				buffs[0]=buff+2*count;
				
				int iread;
				
				iread=toRead;
				if(iread > 500000)iread=500000;
				
				int ret = rx->device->readStream(rx->rxStream, buffs, iread, flags, timeNs, 100000L);
			 
				 timeNs++;
						   
				if(ret <= 0){
				   printf("ret=%d, flags=%d, timeNs=%lld b0 %f b1 %f \n", ret, flags, timeNs,buff[0],buff[1]);
				   break;
				}else if(ret < toRead){
                    count += ret;
                    toRead=toRead-ret;
					//printf("ret=%d, flags=%d, timeNs=%lld b0 %f b1 %f toRead %d witch %d\n", ret, flags, timeNs,buff[0],buff[1],toRead,rx->witch);
				}else{
					break;
				}
            }
	        if(doWhat == 2){
	            rx->mix((float *)rx->buff1,(float *)rx->output);
	           // fprintf(stderr,"rxBuffer wait\n");
            	rx->ibuff=1;
            	while(rx->ibuff==1)Sleep2(10);
	        }
	        break;
		     
	     }
	     
	     //fprintf(stderr,"rx->doWhat %d\n",rx->doWhat);
	}

	return 0;
}

static int freeFilters(struct Filters2 *f)
{
    if (f->iqSampler)msresamp_crcf_destroy(f->iqSampler);
    f->iqSampler=0;

    if (f->iqSampler2)msresamp_rrrf_destroy(f->iqSampler2);
    f->iqSampler2=0;
 
    if(f->demod)freqdem_destroy(f->demod);
    f->demod=0;

    if(f->demodAM)ampmodem_destroy(f->demodAM);
    f->demodAM=0;
    
    return 0;

}
static int setFilters(class Listen *rx,struct Filters2 *f)
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
      //  rx->bw=15000.0;
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
    } else if(rx->decodemode == MODE_NAM2){
        rx->bw=5000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    }
    rx->Ratio = (float)(rx->bw/ rx->samplerate);
    ratio= (float)(48000.0/rx->bw);

    
    f->demod=freqdem_create(0.5);
//#define LIQUID_VERSION_4 1
#ifdef LIQUID_VERSION_4
    f->demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
#else
    f->demodAM = ampmodem_create(0.5, mode, iflag);
#endif

    f->iqSampler  = msresamp_crcf_create(rx->Ratio, As);
    
    f->iqSampler2 = msresamp_rrrf_create(ratio, As);
    
    //msresamp_crcf_print(f->iqSampler);
   
    
    double rate=rx->samplerate;
    
    int size=(int)(0.5+(rate/20.0));
    
    rx->size=size;
    
    fprintf(stderr,"setFilters done size %d\n",size);

    return 0;
    
}

int Listen::mix(float *buf1,float *buf2)
{

    double sint,cost;
    
    for (int k = 0 ; k < size ; k++){
        float r = buf1[k * channels];
        float i = buf1[k * channels + 1];
        if(dt > 0){
            buf2[k * channels] = (float)(r*coso - i*sino);
            buf2[k * channels + 1] = (float)(i*coso + r*sino);
            sint=sino*cosdt+coso*sindt;
            cost=coso*cosdt-sino*sindt;
            coso=cost;
            sino=sint;
        }else{
            buf2[k * channels] = r;
            buf2[k * channels + 1] = i;
        }
    }
    
    double rr=sqrt(coso*coso+sino*sino);
    coso /= rr;
    sino /= rr;
    
	return 0;
}

int ListenSocket(void *rxv)
{
    
    class Listen *l=(class Listen *)rxv;
    
	time_t start,total;
	time_t ship;
	char buff[256];
	long size;
	
	//FILE *in=NULL;
	
	//if(!in)in=fopen("junk.raw","wb");

	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 727 - COPYRIGHT 2020. Start **\n");
	fprintf(stderr,"******************************************************\n");

	start=time(&ship);
	
	l->Bytes=0;
	
    l->ncommand=0;

	while(1){
	    if(l->readCommand(l->clientSocket,buff,&size))return 1;
		if(l->Debug)fprintf(stderr,"buff %s size %ld ncommand %ld\n",buff,size,l->ncommand);
		l->ncommand++;
	    if(!strcmp(buff,"ENDT")){
	        if(l->Debug){
				fprintf(stderr,"ENDT\n");
		    }
	        break;
	    }else if(!strcmp(buff,"STAT")){
	        if(l->Debug){
				fprintf(stderr,"STAT\n");
		    }
		    long n=2*sizeof(double);
		    double buff[2];
		    l->netRead(l->clientSocket,(char *)buff,n);
		    l->setCenterFrequency(buff[0],buff[1]);
		    if(l->Debug)fprintf(stderr,"fc %g samplerate %d\n",l->fc,l->samplerate);
	    }else if(!strcmp(buff,"F   ")){
	        if(l->Debug){
				fprintf(stderr,"F   \n");
		    }
		    double buff[2];
		    l->netRead(l->clientSocket,(char *)buff,size);
		    l->setFrequency(buff[0]);
		    if(l->Debug)fprintf(stderr,"f %g \n",l->f);
	    }else if(!strcmp(buff,"DECO")){
	        if(l->Debug){
				fprintf(stderr,"DECO\n");
		    }
		    double buff[2];
		    l->netRead(l->clientSocket,(char *)buff,size);
		    l->decodemode=(int)buff[0];
		    freeFilters(&l->filter);
		    setFilters(l,&l->filter);
		    if(l->Debug)fprintf(stderr,"decodemode %d \n",l->decodemode);
	    }else if(!strcmp(buff,"FLOA")){
	        if(l->Debug){
				fprintf(stderr,"FLOA\n");
		    }
		    if(size > l->buffsize){
		       if(l->output)free(l->output);
		       l->output=(complex<float> *)malloc(size);
		       if(l->buff1)free(l->buff1);
		       l->buff1=(complex<float> *)malloc(size);
		       l->buffsize=size;
		    }
		    l->Bytes += size;
		    l->netRead(l->clientSocket,(char *)l->buff1,size);
		    if(l->binary)fwrite((char *)l->buff1,size,1,stdout);
		    l->size=size/(2*sizeof(float));
            l->mix((float *)l->buff1,(float *)l->output);
            l->ibuff=1;
            while(l->ibuff==1)Sleep2(10);
         }else if(!strcmp(buff,"SHOR")){
            if(l->Debug){
                fprintf(stderr,"SHOR\n");
            }
            if(size > l->buffsize){
                if(l->output)free(l->output);
                l->output=(complex<float> *)malloc(size*2);
                if(l->buff1)free(l->buff1);
                l->buff1=(complex<float> *)malloc(size*2);
                l->buffsize=size;
            }
            l->Bytes += size;
            l->netRead(l->clientSocket,(char *)l->buff1,size);
		    if(l->binary)fwrite((char *)l->buff1,size,1,stdout);
            l->size=size/(2*sizeof(short int));
            short int *in=(short int *)l->buff1;
            float *out=(float *)l->buff1;
            for(int n=0;n<l->size*2;++n){
                int kk=l->size*2-1-n;
                out[kk]=in[kk];
            }
            l->mix((float *)l->buff1,(float *)l->output);
            l->ibuff=1;
            while(l->ibuff==1)Sleep2(10);
       }else if(!strcmp(buff,"SIGN")){
            if(l->Debug){
                fprintf(stderr,"SIGN\n");
           }
            if(size > l->buffsize){
                if(l->output)free(l->output);
                l->output=(complex<float> *)malloc(size*8);
                if(l->buff1)free(l->buff1);
                l->buff1=(complex<float> *)malloc(size*8);
                l->buffsize=size;
            }
            l->Bytes += size;
            l->netRead(l->clientSocket,(char *)l->buff1,size);
 		    if(l->binary)fwrite((char *)l->buff1,size,1,stdout);
            l->size=size/(2*sizeof(signed char));
            signed char *in=(signed char *)l->buff1;
            float *out=(float *)l->buff1;
            for(int n=0;n<l->size*2;++n){
                int kk=l->size*2-1-n;
                out[kk]=(float)(in[kk]*256.0+0.5);
            }
            l->mix((float *)l->buff1,(float *)l->output);
            l->ibuff=1;
            while(l->ibuff==1)Sleep2(10);            
       }else if(!strcmp(buff,"USIG")){
            if(l->Debug){
                fprintf(stderr,"USIG\n");
           }
            if(size > l->buffsize){
                if(l->output)free(l->output);
                l->output=(complex<float> *)malloc(size*8);
                if(l->buff1)free(l->buff1);
                l->buff1=(complex<float> *)malloc(size*8);
                l->buffsize=size;
            }
            l->Bytes += size;
            l->netRead(l->clientSocket,(char *)l->buff1,size);
 		    if(l->binary)fwrite((char *)l->buff1,size,1,stdout);
 		   // if(in)fwrite((char *)l->buff1,size,1,in);
            l->size=size/(2*sizeof(unsigned char));
            unsigned char *in=(unsigned char *)l->buff1;
            float *out=(float *)l->buff1;
            for(int n=0;n<l->size*2;++n){
                int kk=l->size*2-1-n;
                float v=in[kk];
                out[kk]=(float)((v-128.0)*256.0+0.5);
            }
            l->mix((float *)l->buff1,(float *)l->output);
            l->ibuff=1;
            while(l->ibuff==1)Sleep2(10);            
	    }else{
	        fprintf(stderr,"Unknown Command (%s) %d %d %d %d Skiping\n",
	                buff,buff[0],buff[1],buff[2],buff[3]);
	        if(size > l->buffsize){
                if(l->output)free(l->output);
                l->output=(complex<float> *)malloc(size*8);
            }
            l->Bytes += size;
            l->netRead(l->clientSocket,(char *)l->output,size);
	    }
	}
	
	//if(in)fclose(in);

	l->ibuff= -1;

    total=time(&ship)-start;
	if(!total)total=1;
    fprintf(stderr,"%ld Seconds To Receive %ld Bytes (%ld Bytes/s)\n",
                 (long)total,l->Bytes,(long)(l->Bytes/total));
	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 649 - COPYRIGHT 2020. Done  **\n");
	fprintf(stderr,"******************************************************\n");

    return 1;
}

/*
int launchThread(void *data,int (*sageThread)(void *data))
{
    
    std::thread(sageThread,data).detach();
    
    return 0;
}
*/

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i;
  
  short int *buffer = (short int *) outputBuffer;
    
  class Listen *rx=(class Listen *)userData;
  
  if ( status )fprintf(stderr,"Stream underflow detected!");
    
    //int nskip=rx->size/nBufferFrames;
  
	if (rx->ibuff >= 0){
	
		unsigned int num;
		unsigned int num2;
	
		num=0;
		num2=0;
		
		float *buf1=(float *)rx->buff1;
		float *buf2=(float *)rx->output;
	
		msresamp_crcf_execute(rx->filter.iqSampler, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf1, &num);  // decimate
	
		if(rx->decodemode < MODE_AM){
			freqdem_demodulate_block(rx->filter.demod, (liquid_float_complex *)buf1, (int)num, (float *)buf2);
			msresamp_rrrf_execute(rx->filter.iqSampler2, (float *)buf2, num, (float *)buf1, &num2);  // interpolate
			//fprintf(stderr,"2 rx->size %d num %u num2 %u\n",rx->size,num,num2);
		}else if(rx->decodemode < MODE_USB){
	#define DC_ALPHA 0.99
		
			for(unsigned int n=0;n<num;++n){
				double mag=sqrt(buf1[2*n]*buf1[2*n]+buf1[2*n+1]*buf1[2*n+1]);
				double z0=mag + (rx->filter.amHistory * DC_ALPHA);
				buf2[n]=(float)(z0-rx->filter.amHistory);
				rx->filter.amHistory=z0;
			}
			msresamp_rrrf_execute(rx->filter.iqSampler2, (float *)buf2, num, (float *)buf1, &num2);  // interpolate
		}else{
			ampmodem_demodulate_block(rx->filter.demodAM,  (liquid_float_complex *)buf1, (int)num, (float *)buf2);
			msresamp_rrrf_execute(rx->filter.iqSampler2, (float *)buf2, num, (float *)buf1, &num2);  // interpolate
		}

		//fprintf(stderr,"2 rx->size %d num %u num2 %u\n",play->size,num,num2);

		double dmin,dnom,gain;
	
		double amin=1e30;
		double amax=-1e30;
		double average=0;

		gain=rx->gain;
	
		if(gain < 0.0)gain=1.0;
	
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
	
		if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;
		rx->aminGlobal = 0.8*rx->aminGlobal+0.2*amin;
		amin=rx->aminGlobal;
	
		if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;
		rx->amaxGlobal = 0.8*rx->amaxGlobal+0.2*amax;
		amax=rx->amaxGlobal;


		if((amax-amin) > 0){
			dnom=65535.0/(amax-amin);
		}else{
			dnom=65535.0;
		}
	
		dmin=amin;
		//int short *data=filter.data;
		for(size_t k=0;k<num2;++k){
			short int vv;
			unsigned  char *c=(unsigned char *)&vv;
			double v;
			v=buf1[k];
			v=gain*((v-average)*dnom);
			if(rx->pipe){
				buffer[k]=0;
				vv=(short int)v;
				fputc(c[0],stdout);
				fputc(c[1],stdout);
			}else{
				buffer[k]=(short int)v;
			}
		}
   	   rx->ibuff=-2;
   	   //fprintf(stderr,"Buffer OK\n");
	}else{
		fprintf(stderr,"Late for Buffer\n");
		for ( i=0; i<nBufferFrames; i++ ) {
		  buffer[i] = 0;
		}
  	}
  
    
  return 0;
}
int Listen::setFrequency(double frequency)
{
	double pi;

	if(frequency == f)return 0;
	
	f=frequency;
	
	pi=4.0*atan(1.0);
	dt=1.0/(double)samplerate;
	sino=0;
	coso=1;
	w=2.0*pi*(fc - f);
	sindt=sin(w*dt);
	cosdt=cos(w*dt);
	
	return 0;
}
int Listen::setCenterFrequency(double frequency,double sampleRate)
{
	double pi;
	
	// if(frequency == fc && sampleRate == samplerate)return 0;

	fc=frequency;

	samplerate=(int)sampleRate;
	
	pi=4.0*atan(1.0);
	dt=1.0/(double)samplerate;
	sino=0;
	coso=1;
	w=2.0*pi*(fc - f);
	sindt=sin(w*dt);
	cosdt=cos(w*dt);
	
	freeFilters(&filter);
	
	setFilters(this, &filter);

	return 0;
}
SOCKET Listen::waitForService(char *name)
{
	
	serverSocket=startService(name);

	//fprintf(stderr,"startService %lld\n", serverSocket);
	
	addrLen=sizeof(clientSocketAddr);
	
	while(1){
		int count;
		int ret;

	    ret=CheckSocket(serverSocket,&count,3000);


		if(ret <= 0){
		    fprintf(stderr,"ret %d\n",ret);
			continue;
        }
        
		fprintf(stderr,"ret %d\n",ret);
		
		clientSocket=accept(serverSocket,(struct  sockaddr  *)&clientSocketAddr,
	                        &addrLen);
	                        
	    //ListenSocket(clientSocket);
	    
	    
	   // fprintf(stderr,"accept \n");
	    
	    break;

	
	}
	
	
	
	
	return serverSocket;
	
	return -1;
}

SOCKET Listen::startService(char *name)
{

	// if(getPortAndName(name,&hostAddr,&Port))return -1;
	
	serverSocket=createService(&Port);
    if(serverSocket == -1){
          fprintf(stderr,"Create Port %d Failed\n",Port);
	      return -1;
	}

	return serverSocket;
}
int Listen::readString(SOCKET clientSocket,char *buff,long nbyte)
{
	return netRead(clientSocket,buff,nbyte);	
}
int Listen::getLong(SOCKET clientSocket,long *n)
{
	unsigned char c[4];

	if(readString(clientSocket,(char *)c,4L))return 1;

	*n = c[0] + ((long)c[1] << 8) 
	          + ((long)c[2] << 16) + ((long)c[3] << 24);

	return 0;
}

int Listen::readCommand(SOCKET clientSocket,char *buff,long *size)
{
	long n;

	n=4;
	if(readString(clientSocket,buff,n))return 1;
	buff[n]=0;
	if(getLong(clientSocket,size))return 1;
	return 0;
	
}

int Listen::ListenSocket(SOCKET clientSocket)
{
	return 0;
}

int Listen::netRead(SOCKET clientSocket,char *buff,long n)
{
	long k;
	int isleep;

	if(!buff)return 1;
	
	// fprintf(stderr,"clientSocket %d netRead %p n %ld\n",clientSocket,buff,n);

	// Bytes += n;

	isleep=0;

	k=n;
	while(k > 0){
	    n=k;
	    n=recv(clientSocket,buff,n,0);
	    if(n > 0){
	        k -= n;
	        buff += n;
		    isleep=0;
	    }else if(n == 0){
	        if(++isleep > 20){
	            fprintf(stderr,"netRead Time Out Error\n");
	            return 1;
	        }else{
	            Sleep2(10);
	        }
	    }else{
	        fprintf(stderr,"netRead Error Reading Socket\n");
		    return 1;        
	    }
	}
	return 0;
}
int Listen::CheckSocket(SOCKET serverSocket,int *count,int ms)
{
       struct timeval   tv;
       fd_set fds;

        FD_ZERO(&fds);
        FD_SET(serverSocket, &fds);

	tv.tv_sec = ms/1000;
	tv.tv_usec = (ms%1000)*1000;
	
	int ret = select((int)(serverSocket+1), &fds, NULL,  NULL, &tv);
	if (ret < 0) return ret;
	if (ret == 0) return ret;

/*
        if (select(32, &filedes, (fd_set *)NULL, (fd_set *)NULL, &timeout))  {
                ioctl(serverSocket, FIONREAD, count);
                return TRUE;
        }
	*count = 0;
*/
        return ret;
}

SOCKET Listen::createService(unsigned short *Port)
{
	struct sockaddr_in serverSocketAddr;
	SOCKET serverSocket;
	struct sockaddr_in name;
/*
	int buf_size;
	int ret;

	buf_size=32768;
*/
	zerol((char *)&serverSocketAddr,sizeof(serverSocketAddr));
	serverSocketAddr.sin_port=htons((unsigned short)0);
	serverSocketAddr.sin_port=htons(*Port);
	serverSocketAddr.sin_family=AF_INET;
	serverSocketAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverSocket=socket(AF_INET,SOCK_STREAM,0);
/*
	ret=setsockopt( serverSocket, SOL_SOCKET, SO_RCVBUF, 
                  (char *)&buf_size, sizeof(int) );    
        if(ret < 0)fprintf(stderr,"setsockopt failed\n");
 */
	
	int ret = ::bind(serverSocket, (struct  sockaddr  *)&serverSocketAddr, sizeof(serverSocketAddr));
	if (ret < 0) {
		;
	}
	listen(serverSocket,SOMAXCONN);
	namelen=sizeof(struct sockaddr_in);
	getsockname(serverSocket,(struct  sockaddr  *)&name,&namelen);
		
	*Port=ntohs(name.sin_port);
	return serverSocket;
	
}

int Listen::getPortAndName(char *in,unsigned int *hostAddr,unsigned short *Port)
{
	struct sockaddr_in serverSocketAddr;
	struct hostent *serverHostEnt;
	char out[256];
	unsigned int oneNeg;
	char *np;

	if(!in || !Port)return 1;

	zerol((char *)&serverSocketAddr,sizeof(serverSocketAddr));

	strcpy(out,in);
	if((np=strrchr(out,':'))){
	    *np=0;
	    np += 1;
	    *Port=(unsigned short)atol(np);
	}

	oneNeg = -1L;

	*hostAddr=(unsigned int)inet_addr(out);
	if((long)(*hostAddr) != (long)oneNeg){
 	    *hostAddr=htonl(*hostAddr);
	    fprintf(stderr,"Found Address %lx hostAddr %x oneNeg %x diff %x\n",(long)(*hostAddr),*hostAddr,oneNeg,*hostAddr-oneNeg);
	}else{
	    serverHostEnt=gethostbyname(out);
	    if(serverHostEnt == NULL){
	        fprintf(stderr,"Could Not Find Host (%s)\n",out);
	        return 1;
	    }
	    copyl((char *)serverHostEnt->h_addr,(char *)&serverSocketAddr.sin_addr,serverHostEnt->h_length);
	    *hostAddr=serverSocketAddr.sin_addr.s_addr;
 	    *hostAddr=htonl(*hostAddr);
	    fprintf(stderr,"Found Address %lx\n",(long)*hostAddr);
	}

	return 0;
}
static int zerol(char *p,long n)
{
	if(!p)return 1;
	while(n-- > 0)*p++ = 0;
	return 0;
}
SOCKET Listen::connectToServer(char *serverName,unsigned short *Port)
{
	struct sockaddr_in serverSocketAddr;
	struct hostent *serverHostEnt;
	SOCKET toServerSocket;
	int ret;
	unsigned int hostAddr;
	unsigned int oneNeg;
	short result,Try;
	char *np;
	int buf_size;

	/* oneNeg=0xffffffff; */
	oneNeg = -1L;

	long netsize=200000;

	buf_size=(int)(netsize+30);

	result= -1;

    memset(&serverSocketAddr, 0, sizeof(serverSocketAddr));

	if(!(np=strrchr(serverName,':'))){
	    fprintf(stderr,"Bad Address (%s)",serverName);
	    return result;
	}else{
	    *np=0;
	    np += 1;
	    *Port=(unsigned short)atol(np);
	}

//	hostAddr=(unsigned int)inet_addr(serverName);
	inet_pton(AF_INET, serverName, &hostAddr);
	if((long)hostAddr != (long)oneNeg){
	    serverSocketAddr.sin_addr.s_addr=hostAddr;
	    fprintf(stderr,"Found Address %lx hostAddr %x oneNeg %x diff %x\n",(long)hostAddr,hostAddr,oneNeg,hostAddr-oneNeg);
	}else{
	    serverHostEnt=gethostbyname(serverName);
	    if(serverHostEnt == NULL){
	        fprintf(stderr,"Could Not Find Host (%s)\n",serverName);
	        return result;
	    }
	    copyl((char *)serverHostEnt->h_addr,(char *)&serverSocketAddr.sin_addr,serverHostEnt->h_length);
	}
	serverSocketAddr.sin_family=AF_INET;
	serverSocketAddr.sin_port=htons(*Port);
	Try=0;
	while(Try++ < 10){
	    if((toServerSocket=socket(AF_INET,SOCK_STREAM,0)) < 0){
            fprintf(stderr,"socket Error  (%ld)\n",(long)SOCKET_ERRNO);
	        return toServerSocket;
	    }

	    ret=setsockopt( toServerSocket, SOL_SOCKET, SO_SNDBUF,
                  (char *)&buf_size, sizeof(int) );
        if(ret < 0)fprintf(stderr,"setsockopt SO_SNDBUF failed\n");

	    ret=connect(toServerSocket,(struct sockaddr *)&serverSocketAddr,sizeof(serverSocketAddr));
	    if(ret == -1){
                if (SOCKET_ERRNO == SOCKET_ECONNREFUSED)  {
                    fprintf(stderr,"Connection Refused  Try(%d)\n",Try);
                    closesocket(toServerSocket);
					Sleep2(20);
                    continue;
                }else{
                    fprintf(stderr,"Connection Error  (%ld)\n",(long)SOCKET_ERRNO);
                    return ret;
                }
	    }
	    return toServerSocket;
	}

       return ret;
}
static int copyl(char *p1,char *p2,long n)
{
	if(!p1 || !p2)return 1;

	while(n-- > 0)*p2++ = *p1++;

	return 0;
}
int findRadio(class Listen *rx)
{
    
    std::string argStr;
        
    std::vector<SoapySDR::Kwargs> results;
    
    results = SoapySDR::Device::enumerate();
    
    std::cout << "results.size " << results.size() << std::endl;
    
    if(results.size() < 1)return 1;
    
    rx->device = NULL;
    
    SoapySDR::Kwargs deviceArgs;
    
    
    for(unsigned int k=0;k<results.size();++k){
			std::cout << "SDR device = " << k << "  ";
			deviceArgs = results[k];
			for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
				if (it->first == "label")std::cout << "  " << it->first << " = " << it->second << std::endl;
			}
    }
    
    for(unsigned int k=0;k<results.size();++k){
    
    	if(k == rx->deviceNumber){
        
			deviceArgs = results[k];
		
			std::cout << std::endl;
	
			std::cout << "*****   device = " << k << " selected *****" << std::endl;
	
	
			for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
				std::cout << "  " << it->first << " = " << it->second << std::endl;
				if (it->first == "driver") {
					//dev->setDriver(it->second);
				} else if (it->first == "label" || it->first == "device") {
					//dev->setName(it->second);
				}
			}
		
			// testRadio(rx,deviceArgs);
			
			std::cout << std::endl;

			
			rx->device = SoapySDR::Device::make(deviceArgs);
			
			std::cout << std::endl;
			
			        //query device info
        	std::vector<std::string> names = rx->device->listAntennas(SOAPY_SDR_RX,  rx->channel);
        	std::cout << "Rx antennas: " << std::endl;
        
        	for (std::vector<std::string>::const_iterator ii = names.begin(); ii != names.end(); ++ii){
            
           		 std::cout << (*ii) << std::endl;
            
        	}
			        
        	rx->antennaCount=names.size();
        	rx->antenna=(char **)cMalloc((unsigned long)(rx->antennaCount*sizeof(char *)),8833);
        	for (size_t i=0;i<names.size();++i){
            	rx->antenna[i]=strsave((char *)names[i].c_str(),5555);
        	}

			
			
			std::cout << std::endl;
        	std::cout << "Setting Info: " << std::endl;
			
        	SoapySDR::ArgInfoList args = rx->device->getSettingInfo();
        	if (args.size()) {
            	for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
                	SoapySDR::ArgInfo arg = (*args_i);

                	printf("key %s value %s read %s type %d min %g max %g step %g\n",arg.key.c_str(),arg.value.c_str(),rx->device->readSetting(arg.key).c_str(),
                       	(int)arg.type,arg.range.minimum(),arg.range.maximum(),arg.range.step());

            	}
        	}
			std::cout << std::endl;
			
			
			
		
            if(rx->antennaUse){
            	fprintf(stderr,"Use antenna \"%s\"\n",rx->antennaUse);
            	rx->device->setAntenna(SOAPY_SDR_RX, rx->channel, rx->antennaUse);
            }
            
            
            
            if(rx->setcount){          
             	fprintf(stderr,"setcount %d\n",rx->setcount);
           		for(int k=0;k<rx->setcount;++k){
              		fprintf(stderr,"%s %s\n",rx->set[k].c_str(),rx->value[k].c_str());
               		rx->device->writeSetting(rx->set[k],rx->value[k]);
                }
            }
                
                
                
            
			rx->device->setSampleRate(SOAPY_SDR_RX, rx->channel, rx->samplerate);
			
			rx->device->setFrequency(SOAPY_SDR_RX, rx->channel, rx->fc);
			
        	std::cout << "rx->samplerate " << rx->samplerate << std::endl;
			
			rx->rxStream = rx->device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, (const std::vector<size_t>)0);

			rx->device->activateStream(rx->rxStream, 0, 0, 0); 

			std::cout << "getGainMode: " << rx->device->getGainMode(SOAPY_SDR_RX, rx->channel) << " ";

			std::cout << std::endl;
			
             
		}
    
	}
    
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

