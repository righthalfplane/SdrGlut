#include <stdio.h>

#include <complex>

#include <iostream>

#include <liquid/liquid.h>

#include <RtAudio.h>

#include <unistd.h>

using namespace std;

//g++ -O2 -o listen listen.cpp mThread.c -lliquid -lrtaudio

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6


#include <stdlib.h>

#include "SocketDefs.h"
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <vector>
#include <chrono>
#include <thread>

#include "mThread.h"

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
	msresamp_crcf iqSampler;
    iirfilt_crcf lowpass;
    int samplerate;
    double aminGlobal;
    double amaxGlobal;

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
	
	int getLong(int clientSocket,long *n);

	int readString(int clientSocket,char *buff,long nbyte);

	int waitForService(char *name);
	
	int setCenterFrequency(double frequency,double sampleRate);

	int setFilters();

	SOCKET startService(char *name);
	
	int mix(float *buf1,float *buf2);

	unsigned int hostAddr;

	long Bytes;

	unsigned short Port;
	
	SOCKET serverSocket;

	SOCKET clientSocket;

	struct sockaddr_in clientSocketAddr;
	
	unsigned int addrLen;
	
	long buffsize;
	
	int decodemode;


};

static int copyl(char *p1,char *p2,long n);

static int zerol(char *p,long n);

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );

int launchThread(void *data,int (*sageThread)(void *data));

int ListenSocket(void *rxv);

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
    iqSampler = NULL;
    lowpass = NULL;    // +- 5000 HZ
    
    gain=1.0;
    
    Port=3700;	
	
}

Listen::~Listen()
{
	shutdown(clientSocket,2);
	close(clientSocket);

}

int main(int argc,char *argv[])
{

    class Listen *l=new Listen;
    
    
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
	         l->gain=atof(argv[++n]);
        }else if(!strcmp(argv[n],"-f")){
            l->f=atof(argv[++n]);
        }else if(!strcmp(argv[n],"-p")){
            l->Port=atof(argv[++n]);
		}
	}
    
    
    

	RtAudio dac;
	if ( dac.getDeviceCount() < 1 ) {
		std::cout << "\nNo audio devices found!\n";
		exit( 0 );
	}
	RtAudio::StreamParameters parameters;
	parameters.deviceId = dac.getDefaultOutputDevice();
	// parameters.deviceId = 7;
	parameters.nChannels = 2;
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
    

	l->Debug=0;
	
	int ret = l->waitForService(argv[1]);
	if(ret < 0){
		return 1;
	}
	
	//l->ListenSocket(l->clientSocket);
	

	l->ibuff=-2;
	launchThread((void *)l,ListenSocket);
	
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
int Listen::setFilters()
{

    float As = 60.0f;
            
    Ratio = fOut / samplerate;
    
    //rx->Ratio = 40960.0 / rx->samplerate;
        
        
    
    if(iqSampler)msresamp_crcf_destroy(iqSampler);
    iqSampler = msresamp_crcf_create(Ratio, As);
    
	//msresamp_crcf_print(iqSampler);
	
    if(lowpass)iirfilt_crcf_destroy(lowpass);
    lowpass = iirfilt_crcf_create_lowpass(6,0.104);    // +- 5000 HZ
        
	return 0;
}

int Listen::mix(float *buf1,float *buf2)
{

    double sint,cost;
    
    for (int k = 0 ; k < size ; k++){
        float r = buf1[k * channels];
        float i = buf1[k * channels + 1];
        //r = 0.001*(rand() % 100);
        //i = 0.001*(rand() % 100);
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
    
	unsigned long start,total;
	time_t ship;
	char buff[256];
	float *buffdata;
	long size;

	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 620 - COPYRIGHT 2020. Start **\n");
	fprintf(stderr,"******************************************************\n");

	start=time(&ship);
	
	l->Bytes=0;
	
    l->ncommand=0;

	while(1){
	    if(l->readCommand(l->clientSocket,buff,&size))return 1;
		if(l->Debug)printf("buff %s size %ld ncommand %ld\n",buff,size,l->ncommand);
		l->ncommand++;
	    if(!strcmp(buff,"ENDT")){
	        if(l->Debug){
				printf("ENDT\n");
				fflush(stdout);
		    }
	        break;
	    }else if(!strcmp(buff,"STAT")){
	        if(l->Debug){
				printf("STAT\n");
				fflush(stdout);
		    }
		    long n=2*sizeof(double);
		    double buff[2];
		    l->netRead(l->clientSocket,(char *)buff,n);
		    l->setCenterFrequency(buff[0],buff[1]);
		    if(l->Debug)printf("fc %g samplerate %d\n",l->fc,l->samplerate);
	    }else if(!strcmp(buff,"FLOA")){
	        if(l->Debug){
				printf("FLOA\n");
				fflush(stdout);
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
		    l->size=size/(2*sizeof(float));
            l->mix((float *)l->buff1,(float *)l->output);
            l->ibuff=1;
            while(l->ibuff==1)Sleep2(10);
         }else if(!strcmp(buff,"SHOR")){
            if(l->Debug){
                printf("SHOR\n");
                fflush(stdout);
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
                printf("SIGN\n");
                fflush(stdout);
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
	    }else{
	        printf("Unknown Command (%s) %d %d %d %d Droping Connection\n",
	                buff,buff[0],buff[1],buff[2],buff[3]);
	        l->ibuff= -1;
	        return 1;
	    }
	}
	

	l->ibuff= -1;

    total=time(&ship)-start;
	if(!total)total=1;
    printf("%ld Seconds To Receive %ld Bytes (%ld Bytes/s)\n",
                 total,l->Bytes,(l->Bytes/total));
	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 620 - COPYRIGHT 2020. Done  **\n");
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
  unsigned int i, j;
  
  short int *buffer = (short int *) outputBuffer;
    
  class Listen *rx=(class Listen *)userData;
  
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;
    
    //int nskip=rx->size/nBufferFrames;
  
	if (rx->ibuff >= 0){
	
	    //fprintf(stderr,"sound rx->size %d \n",rx->size);
	    unsigned int num;
        msresamp_crcf_execute(rx->iqSampler, (liquid_float_complex *)rx->output, rx->size, (liquid_float_complex *)rx->buff1, &num);
    /*
		for(unsigned int k=0;k<num;++k){ 
			liquid_float_complex v;
			v=rx->buff[k];
			iirfilt_crcf_execute(rx->lowpass,v, (liquid_float_complex *)&rx->output[k]);
		}
	*/
        //fprintf(stderr,"sound rx->size %d num %d\n",rx->size,num);
        
 	    double amin =  1e33;
	    double amax = -1e33;
	    
 	    double vmin =  1e33;
	    double vmax = -1e33;
	    
	    int kk=0;
	    
	    for(int n=0;n<num;++n){
	    
	        double value = abs(rx->buff1[n]);
	        
	        if(value < amin)amin=value;
	        if(value > amax)amax=value;
	        
			++kk;
			if(kk < nBufferFrames){
				//double vv=(32000.0*rx->buff1[n].real()/0.1);
				double vv=(rx->buff1[n].real());
				if(vv < vmin)vmin=vv;
	        	if(vv > vmax)vmax=vv;
				if(vv >  32000)vv =  32000;
				if(vv < -32000)vv = -32000;
				short int v=(short int)(vv);
				//short int v=(short int)(2.0*32000.0*value/.002);
				buffer[2*kk] = v;
				buffer[2*kk+1] = v;
			}
	    
	    }
	    
	    if(rx->Debug)printf("sound rx-size %d num %d amin %g amain %g nBufferFrames %d kk %d vmin %g vmax %g\n",rx->size,num,amin,amax,nBufferFrames,kk,vmin,vmax);
    
    	rx->ibuff=-2;

	}else{
  
	for ( i=0; i<nBufferFrames; i++ ) {
		for ( j=0; j<2; j++ ) {
		  *buffer++ = 0;
		}
	}
	 // printf("sound nBufferFrames %d\n",nBufferFrames);

  }
  
    
  return 0;
}
int Listen::setCenterFrequency(double frequency,double sampleRate)
{
	float pi;
	
	if(frequency == fc && sampleRate == samplerate)return 0;

	fc=frequency;
	samplerate=sampleRate;
	
	pi=4.0*atan(1.0);
	dt=1.0/(double)samplerate;
	sino=0;
	coso=1;
	w=2.0*pi*(fc - f);
	sindt=sin(w*dt);
	cosdt=cos(w*dt);
	
	setFilters();

	return 0;
}
int Listen::waitForService(char *name)
{
	
	serverSocket=startService(name);
	
	addrLen=sizeof(clientSocketAddr);
	
	while(1){
		int count;
		int ret;

	    ret=CheckSocket(serverSocket,&count,3000);


		if(ret <= 0){
		    printf("ret %d\n",ret);
			continue;
        }
        
		printf("ret %d\n",ret);
		
		clientSocket=accept(serverSocket,(struct  sockaddr  *)&clientSocketAddr,
	                        &addrLen);
	                        
	    //ListenSocket(clientSocket);
	    
	    
	   // printf("accept \n");
	    
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
          printf("Create Port %d Failed\n",Port);
	      return -1;
	}

	return serverSocket;
}
int Listen::readString(int clientSocket,char *buff,long nbyte)
{
	return netRead(clientSocket,buff,nbyte);	
}
int Listen::getLong(int clientSocket,long *n)
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
	unsigned long start,total;
	time_t ship;
	char buff[256];
	float *buffdata;
	long size;

	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 01 - COPYRIGHT 2020. Start **\n");
	fprintf(stderr,"******************************************************\n");

	start=time(&ship);
	
	Bytes=0;
	
	buffdata=NULL;

    ncommand=0;

	while(1){
	    if(readCommand(clientSocket,buff,&size))return 1;
		if(Debug)printf("buff %s size %ld ncommand %ld\n",buff,size,ncommand);
		ncommand++;
	    if(!strcmp(buff,"ENDT")){
	        if(Debug){
				printf("ENDT\n");
				fflush(stdout);
		    }
	        break;
	    }else if(!strcmp(buff,"STAT")){
	        if(Debug){
				printf("STAT\n");
				fflush(stdout);
		    }
		    long n=2*sizeof(double);
		    double buff[2];
		    netRead(clientSocket,(char *)buff,n);
		    setCenterFrequency(buff[0],buff[1]);
		    if(Debug)printf("fc %g samplerate %d\n",fc,samplerate);
	    }else if(!strcmp(buff,"FLOA")){
	        if(Debug){
				printf("FLOA\n");
				fflush(stdout);
		    }
		    if(size > buffsize){
		       if(output)free(output);
		       output=(complex<float> *)malloc(size);
		       if(buff1)free(buff1);
		       buff1=(complex<float> *)malloc(size);
		       buffsize=size;
		    }
		    Bytes += size;
		    netRead(clientSocket,(char *)buff1,size);
		    this->size=size/(2*sizeof(float));
		    mix((float *)buff1,(float *)output);
		    ibuff=1;
   			while(ibuff==1)Sleep2(10);
	    }else{
	        printf("Unknown Command (%s) %d %d %d %d Droping Connection\n",
	                buff,buff[0],buff[1],buff[2],buff[3]);
	        return 1;
	    }
	}
	
//	writeHDF();

    total=time(&ship)-start;
	if(!total)total=1;
    printf("%ld Seconds To Receive %ld Bytes (%ld Bytes/s)\n",
                 total,Bytes,(Bytes/total));
	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 01 - COPYRIGHT 2020. Done  **\n");
	fprintf(stderr,"******************************************************\n");

	return 0;
}

int Listen::netRead(SOCKET clientSocket,char *buff,long n)
{
	long k;
	int isleep;

	if(!buff)return 1;
	
	// printf("clientSocket %d netRead %p n %ld\n",clientSocket,buff,n);

	// Bytes += n;

	isleep=0;

	k=n;
	while(k > 0){
	    n=k;
	    n=read(clientSocket,buff,n);
	    if(n > 0){
	        k -= n;
	        buff += n;
		    isleep=0;
	    }else if(n == 0){
	        if(++isleep > 20){
	            fprintf(stderr,"netRead Time Out Error\n");
	            return 1;
	        }else{
	            sleep(1L);
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
	
	int ret = ::select((int)(serverSocket+1), &fds, NULL,  NULL, &tv);
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
	unsigned int namelen;
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
        if(ret < 0)printf("setsockopt failed\n");
 */
	
	bind(serverSocket,(struct  sockaddr  *)&serverSocketAddr,sizeof(serverSocketAddr));
	listen(serverSocket,5);
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
	    printf("Found Address %lx hostAddr %x oneNeg %x diff %x\n",(long)(*hostAddr),*hostAddr,oneNeg,*hostAddr-oneNeg);
	}else{
	    serverHostEnt=gethostbyname(out);
	    if(serverHostEnt == NULL){
	        printf("Could Not Find Host (%s)\n",out);
	        return 1;
	    }
	    copyl((char *)serverHostEnt->h_addr,(char *)&serverSocketAddr.sin_addr,serverHostEnt->h_length);
	    *hostAddr=serverSocketAddr.sin_addr.s_addr;
 	    *hostAddr=htonl(*hostAddr);
	    printf("Found Address %lx\n",(long)*hostAddr);
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
	    printf("Bad Address (%s)",serverName);
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
	    printf("Found Address %lx hostAddr %x oneNeg %x diff %x\n",(long)hostAddr,hostAddr,oneNeg,hostAddr-oneNeg);
	}else{
	    serverHostEnt=gethostbyname(serverName);
	    if(serverHostEnt == NULL){
	        printf("Could Not Find Host (%s)\n",serverName);
	        return result;
	    }
	    copyl((char *)serverHostEnt->h_addr,(char *)&serverSocketAddr.sin_addr,serverHostEnt->h_length);
	}
	serverSocketAddr.sin_family=AF_INET;
	serverSocketAddr.sin_port=htons(*Port);
	Try=0;
	while(Try++ < 10){
	    if((toServerSocket=socket(AF_INET,SOCK_STREAM,0)) < 0){
            printf("socket Error  (%ld)\n",(long)SOCKET_ERRNO);
	        return toServerSocket;
	    }

	    ret=setsockopt( toServerSocket, SOL_SOCKET, SO_SNDBUF,
                  (char *)&buf_size, sizeof(int) );
        if(ret < 0)printf("setsockopt SO_SNDBUF failed\n");

	    ret=connect(toServerSocket,(struct sockaddr *)&serverSocketAddr,sizeof(serverSocketAddr));
	    if(ret == -1){
                if (SOCKET_ERRNO == SOCKET_ECONNREFUSED)  {
                    printf("Connection Refused  Try(%d)\n",Try);
                    closesocket(toServerSocket);
                    std::this_thread::sleep_for(std::chrono::seconds(4));
                    continue;
                }else{
                    printf("Connection Error  (%ld)\n",(long)SOCKET_ERRNO);
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
