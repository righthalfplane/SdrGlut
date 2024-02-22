#include "Clisten.h"
#include "cStack.h"

#include <assert.h>

/*
c++ -O2 -o listen main.cpp Clisten.cpp mThread.cpp -lliquid -lrtaudio -lpthread

c++ -O2 -std=c++11 -Wno-deprecated -o listen main.cpp Clisten.cpp -lrtaudio -lSoapySDR -lliquid
*/

int Sleep2(int ms);

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );


int doRemoteSpeakers(class Listen *l,unsigned int bufferFrames);

int remoteSpeakers=0;

int launchThread(void *data,int (*sageThread)(void *data));


int zerol(char *p,unsigned long n);

volatile int threadexit; 

void signalHandler( int signum ) {
	//mprint("signum %d\n",signum);
	threadexit=1;
	//exit(0);
}

int Usage()
{
	fprintf(stderr,"Usage:\n");
	fprintf(stderr," listen.x [options]\n");
	fprintf(stderr," listen.x -h\n");
	fprintf(stderr," control-c to stop program\n\n");
	fprintf(stderr,"Mode:\n");
	fprintf(stderr,"  -am               Select AM mode\n");
	fprintf(stderr,"  -fm               Select FM mode\n");
	fprintf(stderr,"  -nbfm             Select narrow band FM mode\n");
	fprintf(stderr,"  -usb              Select upper side band\n");
	fprintf(stderr,"  -lsb              Select lower side band\n");
	fprintf(stderr,"  -cw               Select CW mode\n");
	fprintf(stderr,"  -remotespeakers   Select remotespeakers mode\n");
	fprintf(stderr,"\nAdjustments:\n");
	fprintf(stderr,"  -debug                   Turn on debug output\n");
	fprintf(stderr,"  -f  162.4                Set radio frequency to  162.4 MHZ\n");
	fprintf(stderr,"  -pipe                    Pipe the audio output to sdtout\n");
	fprintf(stderr,"  -audiodevice 1           Use audio output device one\n");
	fprintf(stderr,"  -binary                  Write I/Q data to stdout\n\n");
	fprintf(stderr,"  -p  3500                 listen on port 3500\n\n");
	fprintf(stderr,"\nSoapy Path:\n");
	fprintf(stderr,"SOAPY_SDR_ROOT\n");

	return 0;
}


int main(int argc,char *argv[])
{

	signal(SIGINT, signalHandler);		
	
	threadexit=0;								

    class Listen *l=new Listen;
    
    int device=-2;
    
    
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
            device=(int)atof(argv[++n]);
        }else if(!strcmp(argv[n],"-remotespeakers")){
            remoteSpeakers=1;
            l->Port=5000;
        }else if(!strcmp(argv[n],"-h")){
            Usage();
            exit(0);
        }else if(!strcmp(argv[n],"-pipe")){
            l->pipe=1;
        }else if(!strcmp(argv[n],"-binary")){
            l->binary=1;
        }else{
            fprintf(stderr,"\nUnknown Command \'%s\'\n\n",argv[n]);
        	Usage();
        	exit(0);
		}
	}
       
	RtAudio dac;
	
	int deviceCount=dac.getDeviceCount();
		
	if (deviceCount  < 1 ) {
		fprintf(stderr,"\nNo audio devices found!\n");
		return 1;
	}
	
	fprintf(stderr,"deviceCount %d default output device %d listen port %u\n",deviceCount,dac.getDefaultOutputDevice(),l->Port);
	
    RtAudio::DeviceInfo info;
    
#ifndef RTAUDIO_OLD
	std::vector<unsigned int> id=dac.getDeviceIds();
#else
	std::vector<unsigned int> id;
	for(unsigned int n=0;n<deviceCount;++n){
		id.push_back(n);
	}
#endif

    
    
    for (int i=0; i<deviceCount; i++) {
        
        try {
            info=dac.getDeviceInfo(id[i]);
            if(info.outputChannels > 0){
            // Print, for example, the maximum number of output channels for each device
                fprintf(stderr,"device = %d : maximum output  channels = %d Device Name = %s\n",id[i],info.outputChannels,info.name.c_str());
             }
             
            if(info.inputChannels > 0){
            // Print, for example, the maximum number of output channels for each device
                fprintf(stderr,"device = %d : maximum output  channels = %d Device Name = %s\n",id[i],info.inputChannels,info.name.c_str());
            }

        }
        catch (...) {
            fprintf(stderr,"Audio Error\n");
            break;
        }
        
    }

	RtAudio::StreamParameters parameters;
	
	if(device == -2){
	    parameters.deviceId = dac.getDefaultOutputDevice();
	}else{
	    parameters.deviceId = device;
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
	catch (...) {
		fprintf(stderr,"Audio Error\n");
		exit(1);
	}
	
	
	if(remoteSpeakers){
	   doRemoteSpeakers(l,bufferFrames);
	}else{
    
		// l->Debug=0;
	
		SOCKET ret = l->waitForService(argv[1]);
		if(ret < 0){
			return 1;
		}
		
	}

	try {
    	// Stop the stream
    	dac.stopStream();
  	}
	catch (...) {
		fprintf(stderr,"Audio Error\n");
	}
  	
  	if ( dac.isStreamOpen() ) dac.closeStream();
  	
    delete l;

	return 0;
}

#define SERVER_PORT 5000

int ListenAudio(void *rxv)
{
  unsigned char *data;
  int audioOut;
  int bytes;
  
  
  socklen_t slen=sizeof(sockaddr);


    
    class Listen *rx=(class Listen *)rxv;
    
    
    
    
 	struct sockaddr_in si_me, si_other;
	int s;
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int port=5000;
	int broadcast=1;
	//struct hostent *host;

	setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast, sizeof broadcast);			
	 //host= (struct hostent *) gethostbyname((char *)"192.168.0.7");

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = INADDR_ANY;
//	si_me.sin_addr.s_addr =  *((struct in_addr *)host->h_addr);


	::bind(s, (sockaddr *)&si_me, sizeof(sockaddr));
	
    if(!rx->cs){
    	fprintf(stderr,"ListenAudio Error\n");
    	return 1;
	}
	
    rx->audioOut=0;
    audioOut=0;
    
	rx->mutexo.lock();
	data=(unsigned char *)rx->buffa[rx->audioOut++ % NUM_ABUFF];
	rx->mutexo.unlock();
	
	
	int nn=0;
    int nc=0;
	while(1)
	{
		char buf[10000];
		int ret;
		ret=recvfrom(s, buf, sizeof(buf)-1, 0, (sockaddr *)&si_other, &slen);

		printf("recv ret %d %d\n",ret,nn++);
		bytes=ret;
		for(int k=0;k<bytes;++k){
			data[nc++]=buf[k];
			if(nc >= (int)rx->samplerate*2){
				nc=0;
				rx->cs->pushBuffa(audioOut,rx);
				
				rx->mutexo.lock();
				audioOut=rx->audioOut;
				//printf("audioOut %d rx->samplerate %d\n",audioOut,rx->samplerate);
				data=(unsigned char *)rx->buffa[rx->audioOut++ % NUM_ABUFF];
				rx->mutexo.unlock();
				
			}
		}
		
		if(threadexit){
			//if(out)fclose(out);
			//fprintf(stderr,"ListenAudio Return\n");
			rx->ibuff=0;
			return 0;
		}
		
		
	}
 
 
    return 0;

}
int doRemoteSpeakers(class Listen *rx,unsigned int bufferFrames)
{
	fprintf(stderr,"doRemoteSpeakers bufferFrames %d\n",bufferFrames);
	
 	printf("Listen activating.\n");
 
 	rx->faudio=48000;
 
 	rx->cs=new cStack(rx);
		
	for(int k=0;k<NUM_BUFFERS;++k){
		rx->bufferState[k]=0;
	}

 
 	rx->buff1=(complex<float> *)malloc(bufferFrames*2);
 	if(!rx->buff1){
 	    fprintf(stderr,"malloc error\n");
 	    exit(1);
 	}
 	
 	for(int k=0;k<NUM_ABUFF;++k){
		if(rx->buffa[k])free((char *)rx->buffa[k]);
		rx->buffa[k]=(short int *)malloc((size_t)(2*rx->faudio*4));
		if(!rx->buffa[k]){
			printf("10 cMalloc Errror %ld\n",(long)(2*rx->faudio*4));
			return 1;
		}
		zerol((char *)rx->buffa[k],(unsigned long)(2*rx->faudio*4));
		rx->buffStacka[k]=-1;
	}
	


	
	rx->samplerate=bufferFrames;

	rx->ibuff=-2;
	launchThread((void *)rx,ListenAudio);

	while(rx->ibuff != -1){
		Sleep2(10);
	}

  
  
  	for(int k=0;k<NUM_ABUFF;++k){
		if(rx->buffa[k])free((char *)rx->buffa[k]);
		rx->buffa[k]=NULL;
	}

	if(rx->cs)delete rx->cs;
	rx->cs=NULL;

	
	return 0;
	
}

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i;
  
  short int *buffer = (short int *) outputBuffer;
    
  class Listen *rx=(class Listen *)userData;
  
  if ( status )fprintf(stderr,"Stream underflow detected!");
    
    //int nskip=rx->size/nBufferFrames;
 
 

     	
	if(threadexit){
		fprintf(stderr,"Exit from sound\n");
		exit(0);
	}

   
    if(remoteSpeakers){
       	    
		int ibuff=-1;
		if(rx->cs){
			ibuff=rx->cs->popBuffa(rx);
		}
		//fprintf(stderr,"ibuff %d nBufferFrames %d\n",ibuff,nBufferFrames);
		if (ibuff > 0){
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
   	    
   	    return 0;
    
    }
  
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

		double dnom,gain;
	
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
	
		//dmin=amin;
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
	}else{
		for ( i=0; i<nBufferFrames; i++ ) {
		  buffer[i] = 0;
		}
  	}
  
    
  return 0;
}

int zerol(char *p,unsigned long n)
{
	if(!p || !n)return 1;
	
	while(n-- > 0)*p++ = 0;
	
	return 0;
}

