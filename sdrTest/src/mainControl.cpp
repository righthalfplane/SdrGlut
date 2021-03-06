#include "firstFile.h"
#include "Clisten.h"
#include "mThread.h"
#include <stdio.h>


/*
g++ -O2 -std=c++11 -Wno-deprecated  -o mainControl mainControl.cpp mThread.cpp send.cpp -lliquid -lrtaudio -lpthread -Wno-return-type-c-linkage

g++ -O2 -std=c++11 -Wno-deprecated -o listen main.cpp Clisten.cpp mThread.cpp -lrtaudio -lSoapySDR -lliquid
*/




struct rxdata{
   SOCKET send;
   unsigned short Port;
   char name[256];
   int decodemode;
   double f;
   double fc;
}rxs;


struct rxdata *rx=&rxs;


int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );
         
SOCKET connectToServer(char *serverName,unsigned short *Port);
int doEnd(SOCKET tcp);
int netWrite(SOCKET toServerSocket,char *buffout,long HdfSize);
int writeLab(SOCKET toServerSocket,char *Type,long size);

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6

int main(int argc,char *argv[])
{

	char *buf=(char *)malloc(256);
	char **in=&buf;
	size_t len;
	
	rx->Port=0;
	rx->f=101.5e6;
	rx->fc=101.1e6;
	rx->send=0;
	rx->decodemode=MODE_FM;

	strncpy(rx->name,"192.168.0.3:3700",sizeof(rx->name));
	
	double buff[2];
	long size;

	while(getline(in,&len,stdin)){
	    long n=strlen(buf);
	    buf[n-1]=0;
	    //printf("buf %s len %ld str %ld\n",buf,(long)len,(long)strlen(buf));
	    if(buf[0] == '#'){
	    	continue;
	    }else if(buf[0] == 'a'){
	    	sscanf(buf,"a %s",rx->name);
	      	rx->send=(SOCKET)connectToServer((char *)rx->name,&rx->Port);
    		if(rx->send == -1){
        		fprintf(stderr,"connect failed\n");
        		continue;
    		}
		}else if(buf[0] == 'f'){
	       if(buf[1] == 'c'){
	       	   	sscanf(buf,"fc %lg",&rx->fc);
	        	buff[0]=rx->fc;
    			size=(long)sizeof(double);
    			if(writeLab(rx->send,(char *)"FC  ",size))continue;
    			if(netWrite(rx->send,(char *)buff,size))continue;
	       }else{
	       	   	sscanf(buf,"f %lg",&rx->f);
	       		buff[0]=rx->f;
    			size=(long)sizeof(double);
    			if(writeLab(rx->send,(char *)"F   ",size))continue;
    			if(netWrite(rx->send,(char *)buff,size))continue;
	       }
	    }else if(buf[0] == 'm'){
	         char buf2[256];
	         sscanf(buf,"m %s",buf2);
	         if(!strcmp(buf2,"fm")){
	            rx->decodemode=MODE_FM;
	         }else if(!strcmp(buf2,"nbfm")){
	            rx->decodemode=MODE_NBFM;
	         }else if(!strcmp(buf2,"am")){
	            rx->decodemode=MODE_AM;
	         }else if(!strcmp(buf2,"nam")){
	            rx->decodemode=MODE_NAM;
	         }else if(!strcmp(buf2,"usb")){
	            rx->decodemode=MODE_USB;
	         }else if(!strcmp(buf2,"lsb")){
	            rx->decodemode=MODE_LSB;
	         }else if(!strcmp(buf2,"cw")){
	            rx->decodemode=MODE_CW;
	         }
	         
    		buff[0]=rx->decodemode;
    		size=(long)sizeof(double);
    		if(writeLab(rx->send,(char *)"DECO",size))continue;
    		if(netWrite(rx->send,(char *)buff,size))continue;
	    }else if(buf[0] == 'q'){
	    	break;
	    }
	}
	
    if(rx->send >= 0)doEnd(rx->send);

	exit(0);

    doEnd(rx->send);
}
int main2(int argc,char *argv[])
{

   // class Listen *l=new Listen;
    
    int device=-2;
    
	for(int n=1;n<argc;++n){
	    if(!strcmp(argv[n],"-debug")){
		  // l->Debug = 1;
	    }else if(!strcmp(argv[n],"-am")){
		  // l->decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"-fm")){
		  //l->decodemode = MODE_FM;
        }else if(!strcmp(argv[n],"-nbfm")){
           // l->decodemode = MODE_NBFM;
        }else if(!strcmp(argv[n],"-usb")){
           // l->decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"-lsb")){
           // l->decodemode = MODE_LSB;
	    }else if(!strcmp(argv[n],"-gain")){
	       //  l->gain=(float)atof(argv[++n]);
        }else if(!strcmp(argv[n],"-f")){
           // l->f=atof(argv[++n]);
        }else if(!strcmp(argv[n],"-p")){
           // l->Port=(unsigned int)atof(argv[++n]);
        }else if(!strcmp(argv[n],"-device")){
            device=(int)atof(argv[++n]);
        }else if(!strcmp(argv[n],"-pipe")){
          //  l->pipe=1;
        }else if(!strcmp(argv[n],"-binary")){
           // l->binary=1;
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
						sampleRate, &bufferFrames, &sound, (void *)NULL);
		dac.startStream();
	}
	catch ( RtAudioError& e ) {
		e.printMessage();
		exit( 0 );
	}
    

	// l->Debug=0;
	
//	SOCKET ret = l->waitForService(argv[1]);
//	if(ret < 0){
//		return 1;
//	}
		


	try {
    	// Stop the stream
    	dac.stopStream();
  	}
  	catch (RtAudioError& e) {
    	e.printMessage();
  	}
  	
  	if ( dac.isStreamOpen() ) dac.closeStream();

    //delete l;

	return 0;
}

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i;
  
  short int *buffer = (short int *) outputBuffer;
/*
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
	}else{
		for ( i=0; i<nBufferFrames; i++ ) {
		  buffer[i] = 0;
		}
  	}
  */
    
  return 0;
}
