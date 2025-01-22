/*
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif
*/

#include "sdr.h"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <chrono>
#include <thread>
#include <vector>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


/*
g++ -O2 -std=c++11 -Wno-deprecated -o sdrTest sdrTest.cpp mThread.cpp cMalloc.cpp Clisten.cpp -lrtaudio -lSoapySDR -lliquid -framework OpenAL -Wno-return-type-c-linkage

g++ -O2 -std=c++11 -Wno-deprecated -o sdrTest sdrTest.cpp mThread.cpp cMalloc.cpp  Clisten.cpp -lrtaudio -lSoapySDR -lliquid -lopenal -pthread

./sdrTest.x -fc 101.1e6 -f 101.5e6 -fm -gain 1 -device 1 -samplerate 2000000

./sdrTest.x -fc 162.0e6 -f 162.4e6 -nbfm -gain 1

*/

void *cMalloc(unsigned long r, int tag);

char *strsave(char *s,int tag);

int cFree(char *p);

int zerol(unsigned char *s,unsigned long n);


static int GetTime(long *Seconds,long *milliseconds);

static double rtime(void);

std::vector<SoapySDR::Kwargs> resultsEnumerate;
         
int audiodevice;

//static void list_audio_devices(const ALCchar *devices);
//static void list_audio(void);

void checkall(void);

#ifdef MAIN_IN
int main (int argc, char *argv[])
{	
	class sdrClass *sdr;
	
	char *argv2[]={(char *)"sdrTest.x", (char *)"-fc", (char *)"1e6", (char *)"-f", (char *)"0.76e6", (char *)"-am", 
	               (char *)"-gain", (char *)"1", (char *)"-samplerate", (char *)"912000", (char *)"-device", (char *)"0"};
	               
	int argc2=sizeof(argv2)/sizeof(char *);
/*
	char *argv3[]={(char *)"sdrTest.x", (char *)"-fc", (char *)"101.1e6", (char *)"-f", (char *)"101.5e6", (char *)"-fm", 
	               (char *)"-gain", (char *)"1", (char *)"-samplerate", (char *)"2000000", (char *)"-device", (char *)"1"};
	               
	int argc3=sizeof(argv3)/sizeof(char *);
*/
		
	sdr=new sdrClass();
	
	sdr->setup(argc2,argv2);
	
	if(sdr)delete sdr;
	
	return 0;
}
#endif

int udpRead(class Listen *l,int n)
{
	socklen_t slen=sizeof(sockaddr);
	struct sockaddr_in si_other;
	
	long k;
	int isleep;
	
	char *buff;
	
	buff=(char *)l->buff1;

	if(!buff)return 1;
	
	isleep=0;

	k=n;
	while(k > 0){
	    n=k;
	    n=recvfrom(l->s, buff, n, 0, (sockaddr *)&si_other, &slen);
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

int ListenSocket2(class Listen *l)
{
	
	time_t start,total;
	time_t ship;
	char buff[256];
	long size;
//	int ret;
		
	start=time(&ship);
	
	l->Bytes=0;
	
    l->ncommand=0;
    
    int ncut=20;
    
    size=0;
    
	while(1){
		if(l->exitListen)break;
		l->ncommand++;
	    if(l->data_type == TYPE_FLOAT){
		    size=(int)(2*sizeof(float)*l->samplerate/ncut);
	        if(l->Debug){
				fprintf(stderr,"FLOAT size %ld\n",size);
		    }
		    if(size > l->buffsize){
		       if(l->output)free(l->output);
		       l->output=(complex<float> *)malloc(size);
		       if(l->buff1)free(l->buff1);
		       l->buff1=(complex<float> *)malloc(size);
		       l->buffsize=size;
		    }
		    l->Bytes += size;
			if(l->listenType == LISTEN_TCP){
		        if(l->netRead(l->clientSocket,(char *)l->buff1,size))break;
		    }else{
		    	udpRead(l,size);
		    	//fprintf(stderr,"UDP Read ret %d\n",ret);
		    }
		    if(l->binary)fwrite((char *)l->buff1,size,1,stdout);
		    l->size=size/(2*sizeof(float));
          //  l->mix((float *)l->buff1,(float *)l->output);
            l->ibuff=1;
            while(l->ibuff==1)Sleep2(10);
         }else if(l->data_type == TYPE_SHORT){
 		    size=(int)(2*sizeof(short int)*l->samplerate/ncut);
           	if(l->Debug){
 				fprintf(stderr,"SHORT size %ld\n",size);
           	}
            if(size > l->buffsize){
                if(l->output)free(l->output);
                l->output=(complex<float> *)malloc(size*2);
                if(l->buff1)free(l->buff1);
                l->buff1=(complex<float> *)malloc(size*2);
                l->buffsize=size;
            }
            l->Bytes += size;
			if(l->listenType == LISTEN_TCP){
		        if(l->netRead(l->clientSocket,(char *)l->buff1,size))break;
		    }else{
		    	udpRead(l,size);
		        //fprintf(stderr,"UDP Read ret %d\n",ret);
		    }
		    if(l->binary)fwrite((char *)l->buff1,size,1,stdout);
            l->size=size/(2*sizeof(short int));
            short int *in=(short int *)l->buff1;
            float *out=(float *)l->buff1;
            for(int n=0;n<l->size*2;++n){
                int kk=l->size*2-1-n;
                out[kk]=in[kk];
            }
           // l->mix((float *)l->buff1,(float *)l->output);
            l->ibuff=1;
            while(l->ibuff==1)Sleep2(10);
       }else if(l->data_type == TYPE_SIGNED){
 		    size=(int)(2*sizeof(signed char)*l->samplerate/ncut);
            if(l->Debug){
  				fprintf(stderr,"SIGNED size %ld\n",size);
          	}
            if(size > l->buffsize){
                if(l->output)free(l->output);
                l->output=(complex<float> *)malloc(size*8);
                if(l->buff1)free(l->buff1);
                l->buff1=(complex<float> *)malloc(size*8);
                l->buffsize=size;
            }
            l->Bytes += size;
			if(l->listenType == LISTEN_TCP){
		        if(l->netRead(l->clientSocket,(char *)l->buff1,size))break;
		    }else{
		    	udpRead(l,size);
		    	//fprintf(stderr,"UDP Read ret %d\n",ret);
		    }
 		    if(l->binary)fwrite((char *)l->buff1,size,1,stdout);
            l->size=size/(2*sizeof(signed char));
            signed char *in=(signed char *)l->buff1;
            float *out=(float *)l->buff1;
            for(int n=0;n<l->size*2;++n){
                int kk=l->size*2-1-n;
                out[kk]=(float)(in[kk]*256.0+0.5);
            }
            //l->mix((float *)l->buff1,(float *)l->output);
            l->ibuff=1;
            while(l->ibuff==1)Sleep2(10);            
       }else if(l->data_type == TYPE_UNSIGNED){
 		    size=(int)(2*sizeof(unsigned char)*l->samplerate/ncut);
            if(l->Debug){
   				fprintf(stderr,"UNSIGNED size %ld\n",size);
          	}
            if(size > l->buffsize){
                if(l->output)free(l->output);
                l->output=(complex<float> *)malloc(size*8);
                if(l->buff1)free(l->buff1);
                l->buff1=(complex<float> *)malloc(size*8);
                l->buffsize=size;
            }
            l->Bytes += size;
			if(l->listenType == LISTEN_TCP){
		        if(l->netRead(l->clientSocket,(char *)l->buff1,size))break;
		    }else{
		    	udpRead(l,size);
		    	//fprintf(stderr,"UDP Read ret %d\n",ret);
		    }
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
            //l->mix((float *)l->buff1,(float *)l->output);
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
	
	l->ibuff= -1;

    total=time(&ship)-start;
	if(!total)total=1;
    if(l->Debug)fprintf(stderr,"%ld Seconds To Receive %ld Bytes (%ld Bytes/s)\n",
                 (long)total,l->Bytes,(long)(l->Bytes/total));

	
	return 1;
}
int ListenSocket3(class Listen *l)
{


	int size=10000000*2;
	char *buff1=(char *)malloc(size*2);
	char *buff=buff1;
	if(!buff)return 1;


	long n;
	
	fprintf(stderr,"Start Piping to stdout\n");
		
	while(1){
		n=recv(l->clientSocket,buff,size,0);
		if(n <= 0){
			break;
		}
	//	fprintf(stderr,"ListenSocket3 n %ld\n",n);
		fwrite(buff,n,1,stdout);
	}
	
	if(buff1)free(buff1);

	fprintf(stderr,"Stop Piping to stdout\n");
	
	return 0;
}
int ListenSocket(void *rxv)
{
    
    class Listen *l=(class Listen *)rxv;
    
	time_t start,total;
	time_t ship;
	char buff[256];
	long size;
	
	if(l->data_type >= 0){
		return ListenSocket2(l);
	}else if(l->cat > 0){
		return ListenSocket3(l);	
	}
	
	//FILE *in=NULL;
	
	//if(!in)in=fopen("junk.raw","wb");

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
		    //setFrequency(buff[0],&rx);
		   // if(l->Debug)fprintf(stderr,"f %g \n",rx.f);
	    }else if(!strcmp(buff,"FC  ")){
	        if(l->Debug){
				fprintf(stderr,"FC  \n");
		    }
		    double buff[2];
		    l->netRead(l->clientSocket,(char *)buff,size);
		  //  if(l->Debug)fprintf(stderr,"fc %g \n",rx.fc);
	    }else if(!strcmp(buff,"DECO")){
	        if(l->Debug){
				fprintf(stderr,"DECO\n");
		    }
		    double buff[2];
		    l->netRead(l->clientSocket,(char *)buff,size);
		  //  setDecodeMode(buff[0],&rx);
		  //  if(l->Debug)fprintf(stderr,"decodemode %d \n",rx.decodemode);
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
          //  l->mix((float *)l->buff1,(float *)l->output);
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
           // l->mix((float *)l->buff1,(float *)l->output);
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
            //l->mix((float *)l->buff1,(float *)l->output);
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
            //l->mix((float *)l->buff1,(float *)l->output);
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
    if(l->Debug)fprintf(stderr,"%ld Seconds To Receive %ld Bytes (%ld Bytes/s)\n",
                 (long)total,l->Bytes,(long)(l->Bytes/total));

    return 1;
}

int doEnumerate(char *deviceString)
{

	fprintf(stderr,"deviceString %p\n",deviceString);
	
	resultsEnumerate=SoapySDR::Device::enumerate(deviceString);
	
	int length=resultsEnumerate.size();
    
	if(length == 0){
		fprintf(stderr,"Error: enumerate Found No Devices - Try Again !\n");
		return 1;
	}

	return 0;
}
int sdrClass::waitPointer(std::string name,volatile int *pointer,int flag)
{
	int nc=0;
/*
	fprintf(stderr,"name %s pointer %p *pointer %d thread %llx\n",name.c_str(),pointer,
	                *pointer,(long long)std::this_thread::get_id);
*/
	if(*pointer < 0){
		if(flag > 0)fprintf(stderr,"%s Immediate return %d\n",name.c_str(),nc);
		return 0;
	}
	*pointer = -1;
	while(*pointer == -1){
		Sleep2(20);
		if(nc++ > 200){
			fprintf(stderr,"%s Timed out %d\n",name.c_str(),nc);
			return 1;
		}else if(flag > 0){
			fprintf(stderr,"%s Waiting %d\n",name.c_str(),nc);		
		}
	}
	return 0;
}

void sdrClass::setMode(std::string mode)
{

	//fprintf(stderr,"setMode Start\n");
	
	waitPointer("iqToAudio(1)",&iqToAudio,0);
				
	if(mode == "AM"){
	   decodemode = MODE_AM;
	}else if(mode == "NAM"){
	   decodemode = MODE_NAM;
	}else if(mode == "FM"){
	   decodemode = MODE_FM;
	}else if(mode == "NBFM"){
	   decodemode = MODE_NBFM;
	}else if(mode == "USB"){
		decodemode = MODE_USB;
	}else if(mode == "LSB"){
		decodemode = MODE_LSB;
	}else if(mode == "CW"){
		decodemode = MODE_CW;
	}else{
		decodemode = MODE_AM;
	}

	iqToAudio=0;

	std::thread(&sdrClass::Process,this).detach();
	for(int n=0;n<audioThreads;++n){
		std::thread(&sdrClass::Process,this).detach();
    }

  	
	//fprintf(stderr,"setMode End\n");
  	
  		
}

sdrClass::sdrClass()
{
	//fprintf(stderr,"sdrClass::sdrClass\n");
	
	audiodevice=0;
	samplerate=2000000;
	samplewidth=samplerate;
	samplescale=1.0;
	deviceNumber=0;
	gain=1.0;
	fc=101.5e6;
	fw=fc;
	f=101.5e6;
    channel=0;
    setcount=0;
    faudio=48000;
    outFile=NULL;
    inFile=NULL;
    inData=IN_RADIO;
    timeout=0;
    timestart=0;
    idump=0;
    PPM=0;
    aminGlobal=0;
    amaxGlobal=0;
    decodemode = MODE_FM;
    Debug = 0;
    ncut = 20;
    
    rf_gain=0;
    
    rxStream=NULL;
    
    device=NULL;
    
	
	setcount=0;
	     
	     
	saveFlag=0;
    saveCall=0;
    saveLength=0;
    saveBuff=NULL;
    
    bandwidth=0;
    
    thread=0;
        
    iWait=0;
    
    bS=NULL;
    
    bS2=NULL;
    

    iqToAudio=0;
    
    mprintFlag=0;
    
    hasGainMode=0;
    
    inFilenum=-1;
    
    IQSwap=0;
    
    sendBuff=NULL;
    
    deviceString=(char *)"";
    
    audioThreads=0;
      	
}
sdrClass::~sdrClass()
{	

	Sleep2(10);
	
	waitPointer("iqToAudio(2)",&iqToAudio,0);
	
	waitPointer("frame(2)",&frame,0);
	
	waitPointer("doWhat(2)",&doWhat,0);
	
	if(sendBuff)cFree((char *)sendBuff);
	sendBuff=NULL;
	
	if(bS)delete bS;
	bS=NULL;
	if(bS2)delete bS2;
	bS2=NULL;
	
	//fprintf(stderr,"exit sdrClass %p thread %llx\n",this,(long long)std::this_thread::get_id);
}

int sdrClass::setDataSave(int length)
{
	saveCall=0;
	Sleep2(10);
	if(saveBuff)cFree((char *)saveBuff);
	saveBuff=(float *)cMalloc(length*2*sizeof(float),9999);
	saveLength=length;
	saveCall=1;
	return 0;
}

int sdrClass::setup(int argc, char *argv[])
{	
	for(int n=1;n<argc;++n){
	    //fprintf(stderr,"argv %d %s\n",n,argv[n]);
	    if(!strcmp(argv[n],"-debug")){
		   Debug = 1;
	    }else if(!strcmp(argv[n],"-am")){
		   decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"AM")){
		   decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"-nam")){
		   decodemode = MODE_NAM;
	    }else if(!strcmp(argv[n],"NAM")){
		   decodemode = MODE_NAM;
	    }else if(!strcmp(argv[n],"-fm")){
		   decodemode = MODE_FM;
	    }else if(!strcmp(argv[n],"FM")){
		   decodemode = MODE_FM;
	    }else if(!strcmp(argv[n],"-nbfm")){
		   decodemode = MODE_NBFM;
	    }else if(!strcmp(argv[n],"NBFM")){
		   decodemode = MODE_NBFM;
        }else if(!strcmp(argv[n],"-usb")){
            decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"USB")){
            decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"-lsb")){
            decodemode = MODE_LSB;
        }else if(!strcmp(argv[n],"LSB")){
            decodemode = MODE_LSB;
	    }else if(!strcmp(argv[n],"-gain")){
	         gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-PPM")){
	         PPM=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-rf_gain")){
	         rf_gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-fc")){
	         fc=atof(argv[++n]);
	         fw=fc;
	    }else if(!strcmp(argv[n],"-f")){
	         f=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-channel")){
	         channel=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-outFile")){
	         outFile=fopen(argv[++n],"wb");
	         if(outFile == NULL){
	             mprint("Could Not Open %s to Write\n",argv[n]);
	         }
	    }else if(!strcmp(argv[n],"-inFile")){
	         inData=IN_RADIO;
	         inFile=fopen(argv[++n],"rb");
	         if(inFile == NULL){
	             mprint("Could Not Open %s to Read\n",argv[n]);
	         }else{
	         	inData=IN_FILE;
	         }
	    }else if(!strcmp(argv[n],"-faudio")){
	         faudio=(float)atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-device")){
	         deviceNumber=atoi(argv[++n]);
	    }else if(!strcmp(argv[n],"-audiodevice")){
	         audiodevice=atoi(argv[++n]);
	    }else if(!strcmp(argv[n],"-samplerate")){
            samplerate=(int)atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-timeout")){
            timeout=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-antenna")){
           ;
	    }else if(!strcmp(argv[n],"-set")){
	     set[setcount]=argv[++n];
	     value[setcount++]=argv[++n];
	    }else{
	    	mprint("Unknown Command = \"%s\"\n",argv[n]);
			// infilename = argv [n] ;
		}
	}
	
	if(startPlay())return 0;
		
	return run();	
}
int sdrClass::run()
{
	
	bS=new cStack;
	
	bS2=new cStack;

	playRadio();
	
	if(bS)delete bS;
	bS=NULL;

	if(bS2)delete bS2;
	bS2=NULL;
	
	stopPlay();
	
//	fprintf(stderr,"return stopPlay\n");

	if(outFile)fclose(outFile);
	
	if(inFile)fclose(inFile);
		
	checkall();
	
	//fprintf(stderr,"End of Setup\n");
	
	frame=-2;

	return 0;
}
int sdrClass::startPlay()
{
	//fprintf(stderr,"startPlay\n");

	if(inData == IN_RADIO){
		if(findRadio() || device == NULL){
			fprintf(stderr,"Error Opening SDR\n");
			return 1;
		}
	}
	
	initPlay();

	return 0;	

}
int sdrClass::playRadio()
{
	class sdrClass *rx=this;

	double rate=samplerate;
			  
	int size=(int)rate/rx->ncut;

	rx->size=size;

	mprint("rate %f rx->size %d\n",rate,rx->size);

	size += 1024;  // bug in rfspace NetSDR and icr8600

	bS->setBuff(size,faudio);
	
	bS2->setBuff(size,faudio);
	
	fillBuffer=0;
	if(sendBuff)cFree((char *)sendBuff);
	sendBuff=(float *)cMalloc(size*2*sizeof(float),76868);
	if(!sendBuff){
		fprintf(stderr,"Error playRadio cMalloc Failed\n");
		return 1;	
	}
	
	rx->doWhat=0;

	rx->witch=0;

	rx->frame=0;

	rx->iqToAudio=0;
			
	rx->audioOut=0;		
	
	if(rx->bandwidth > 0){
		rx->device->setBandwidth( SOAPY_SDR_RX, rx->channel, rx->bandwidth);
	}

	std::thread(&sdrClass::rxBuffer,this).detach();
	
	rx->doWhat=2;
	
	aminGlobal=0;
	    
    amaxGlobal=0;
    
    std::thread(&sdrClass::Process,this).detach();
	for(int n=0;n<audioThreads;++n){
		std::thread(&sdrClass::Process,this).detach();
    }

    s->bS=bS;

	rx->timestart=rtime();
	
  	while(rx->frame >= 0){
  		Sleep2(50);
  		
		if(rx->timeout > 0 && rtime() > rx->timeout+rx->timestart){
			break;
		}
		
   	}  
   	      
	waitPointer("doWhat(4)",&doWhat,0);
   
    return 0;
  
}

int sdrClass::stopPlay()
{
	sdrClass *rx=this;
	
	   
    s->bS=NULL;
    
    if(bS)delete bS;
    bS=NULL;

    if(bS2)delete bS2;
    bS2=NULL;


    if(rx->device){
        rx->device->deactivateStream(rx->rxStream, 0, 0);
   
        rx->device->closeStream(rx->rxStream);
     
     	Sleep2(20);
    
        SoapySDR::Device::unmake(rx->device);
       
        rx->device=NULL;
    }
    
    
    
	return 0;
}

int sdrClass::findRadio()
{
    
    std::string argStr;
    
    device = NULL;
        
 //   fprintf(stderr,"Call enumerate\n");
    
    if(resultsEnumerate.size() < 1){
    	if(doEnumerate(deviceString))return 1;
    }
        
  // fprintf(stderr,"1 findRadio samplerate %g\n",samplerate);
    
    mprintFlag=Debug;
    
    mprintFlag=1;
    
    mprint("Number of Devices Found: %ld\n",(long)resultsEnumerate.size());
    
    SoapySDR::Kwargs deviceArgs;
    
    for(unsigned int k=0;k<resultsEnumerate.size();++k){
    		mprint("SDR device =  %ld ",(long)k);
			deviceArgs = resultsEnumerate[k];
			for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
				if (it->first == "label")mprint(" %s = %s\n ",it->first.c_str(), it->second.c_str());
			}
    }
    
    mprint("\n");
    
    if(deviceNumber < 0 || deviceNumber >= resultsEnumerate.size()){
    	deviceNumber=0;
    }

    for(unsigned int k=0;k<resultsEnumerate.size();++k){
		deviceArgs = resultsEnumerate[k];
		
		SoapySDR::Device *devicer= SoapySDR::Device::make(deviceArgs);;
		
	//	fprintf(stderr,"2 findRadio samplerate %g devicer %p\n",samplerate,devicer);


    	if(k == deviceNumber){
        
			device=devicer;
		
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

			
			mprint("driver = %s\n",device->getDriverKey().c_str());
			
			mprint("hardware = %s\n",device->getHardwareKey().c_str());
        
			SoapySDR::Kwargs it=device->getHardwareInfo();
		
			for( SoapySDR::Kwargs::iterator  ii=it.begin(); ii!=it.end(); ++ii)
			{
					mprint("%s = %s ",ii->first.c_str(), ii->second.c_str());
			}

			
    		mprint("\n\n");
    		
    		//fprintf(stderr,"2 findRadio samplerate %g device %p\n",samplerate,device);

			
			        //query device info
			antennaNames.clear();
        	antennaNames = device->listAntennas(SOAPY_SDR_RX,  channel);
        	mprint("Rx antennas: \n",(long)k);

        	for (std::vector<std::string>::const_iterator ii = antennaNames.begin(); ii != antennaNames.end(); ++ii){
       			mprint("%s\n",(*ii).c_str());
        	}
			        
        	mprint("\n");
        	
        	mprint("Setting Info: \n");
						
        	SoapySDR::ArgInfoList args = device->getSettingInfo();
        	if (args.size()) {
            	for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
                	SoapySDR::ArgInfo arg = (*args_i);

                	mprint("key %s value %s read %s type %d min %g max %g step %g\n",arg.key.c_str(),arg.value.c_str(),device->readSetting(arg.key).c_str(),
                       	(int)arg.type,arg.range.minimum(),arg.range.maximum(),arg.range.step());

            	}
        	}
        	mprint("\n");
        	
			
	        if(setcount){          
             	mprint("setcount %d\n",setcount);
           		for(int k=0;k<setcount;++k){
              		mprint("%s %s\n",set[k].c_str(),value[k].c_str());
               		device->writeSetting(set[k],value[k]);
                }
        		mprint("\n");
            }
            
            
            streamFormat.clear();
        	streamFormat=device->getStreamFormats(SOAPY_SDR_RX, channel);
        	
			for (size_t j = 0; j < streamFormat.size(); j++)
			{
				mprint("RX StreamFormats %lu %s\n",j, streamFormat[j].c_str());
				if(j == streamFormat.size()-1)mprint("\n");
			}
			          
        	rxGainRange=device->getGainRange(SOAPY_SDR_RX, channel);
        
            mprint("RF Gain range RX min %g max %g \n",rxGainRange.minimum(),rxGainRange.maximum());
            
            rxGainRangeList.clear();
            rxGainNames.clear();
            
            rxGainRangeList.push_back(rxGainRange);
            rxGainNames.push_back("RX Gain");
           
   
            SoapySDR::RangeList rlist=device->getFrequencyRange(SOAPY_SDR_RX, channel);
            
        	for (size_t j = 0; j < rlist.size(); j++)
        	{
         	    mprint("FrequencyRange min %g max %g \n",rlist[j].minimum(),rlist[j].maximum());
        	}

        	band=device->listBandwidths(SOAPY_SDR_RX, channel);
        	if(band.size()){
                mprint("\nBandwidth MHZ ");  		
				for (size_t j = 0; j <band.size(); j++)
				{
				   mprint(" %.2f ",band[j]/1.0e6);
				}
				mprint("\n\n");
            }
            

			rate=device->listSampleRates(SOAPY_SDR_RX, channel);
        	if(rate.size()){
                 mprint("SampleRates MHZ ");
      		}
			for (size_t j = 0; j < rate.size(); j++)
        	{
           		mprint(" %.6f ",rate[j]/1.0e6);
         	}
            mprint("\n\n");

            mprint("Gains: \n");  		
			std::vector<std::string> names=device->listGains( SOAPY_SDR_RX, channel);
			for (size_t j = 0; j < names.size(); j++)
			{
				mprint("%lu %s ",j, names[j].c_str());
			
				SoapySDR::Range range3=device->getGainRange(SOAPY_SDR_RX, channel, names[j].c_str());
				mprint("range max %g min %g\n",range3.maximum(),range3.minimum());
				
				rxGainRangeList.push_back(range3);
            	rxGainNames.push_back(names[j]);

			}

           mprint("\n");
           
           // fprintf(stderr,"3 findRadio samplerate %g device %p channel %d\n",samplerate,device,channel);


			device->setSampleRate(SOAPY_SDR_RX, channel, samplerate);
			
			
	       // fprintf(stderr,"4 findRadio samplerate %g device %p\n",samplerate,device);
		
			
			samplewidth=samplerate;
			
			device->setFrequency(SOAPY_SDR_RX, channel, fc);
			
  	       // fprintf(stderr,"5 findRadio samplerate %g device %p fc %g\n",samplerate,device,fc);
  	        
      		mprint("samplerate %g\n",samplerate);
			
			//const std::vector<size_t> channels = {(size_t)0,(size_t)1};
			
			const std::vector<size_t> channels = {(size_t)0};
						
			rxStream = device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, channels);

			device->activateStream(rxStream, 0, 0, 0); 
			
			hasGainMode=device->hasGainMode(SOAPY_SDR_RX, channel);

       		mprint("hasGainMode: %d\n",hasGainMode);

            if(hasGainMode){
            	bool automatic=true;
            	device->setGainMode(SOAPY_SDR_RX,channel, automatic);
        		mprint("getGainMode: %d\n",device->getGainMode(SOAPY_SDR_RX, channel));
        	}
			
			int hasFrequencyCorrection= device->hasFrequencyCorrection(SOAPY_SDR_RX, channel);
			
        	mprint("hasFrequencyCorrection: %d\n",hasFrequencyCorrection);
			
			if(hasFrequencyCorrection && PPM){
			    device->setFrequencyCorrection(SOAPY_SDR_RX, channel,PPM);
			}
			
			if(rf_gain > 0){
              	mprint("RF Gain set to %g \n",rf_gain);
              	device->setGain(SOAPY_SDR_RX, channel,rf_gain);
            }
            
    		MTU=device->getStreamMTU(rxStream);
    
    		mprint("MTU %ld\n",(long)MTU);

		}else{
			 if(devicer)SoapySDR::Device::unmake(devicer);
		}
    }
    return 0;
}
int sdrClass::setSampleWidth(double value)
{
	if(value <= 0)value=1;
	
	value /= 200;
	
	samplescale=value;
	
	samplewidth=samplerate*samplescale;
	
	fw=f;
	
	if((fw - samplewidth*0.5) < 0.0){
		fw=samplewidth*0.5;
	}else if((fw - samplewidth*0.5) < fc-0.5*samplerate){
		fw=fc-0.5*samplerate+samplewidth*0.5;	
	}else if((fw + samplewidth*0.5) > fc+0.5*samplerate){
		fw=fc+0.5*samplerate-samplewidth*0.5;
	}
		
	return 0;
}
int sdrClass::setFrequencyFC(double frequency)
{
	if(inData != IN_RADIO)return 1;
	
	if(frequency <= 0)frequency = bw*0.5;
	if(frequency >=900000000000ll)frequency=900000000000ll;
	try{
		device->setFrequency(SOAPY_SDR_RX, channel, frequency);
	} catch(const std::exception &e) {
		std::string streamExceptionStr = e.what();
		fprintf(stderr,"setCenterFrequency Error: %s\n",streamExceptionStr.c_str());
		device->setFrequency(SOAPY_SDR_RX, channel, fc);
		return 1;
	}
	fc=frequency;	
	f=frequency;
	fw=fc;
	//fprintf(stderr,"setFrequencyFC %g\n",fc);
	initPlay();
	return 1;
}
int sdrClass::setCenterFrequency(double frequency)
{

	if(inData != IN_RADIO){
		if(frequency <= 0.0)frequency = bw*0.5;
		if(frequency >=900000000000ll)frequency=900000000000ll;
		if(frequency < fw-samplewidth*0.5){
			fw=frequency;
			if(fw - 0.5*samplewidth  < fc-0.5*samplerate)fw=fc-0.5*samplerate+0.5*samplewidth;
			if(frequency < fw - 0.5*samplewidth)frequency=fw - 0.5*samplewidth+bw/2;		
		}else if(frequency > fw+samplewidth*0.5){
			fw=frequency;
			if(fw + 0.5*samplewidth > fc+0.5*samplerate)fw=fc+0.5*samplerate-0.5*samplewidth;
			if(frequency > fw + 0.5*samplewidth)frequency=fw + 0.5*samplewidth-bw/2;
		}
		f=frequency;
		initPlay();
	}else{
		if(frequency <= 0)frequency = bw*0.5;
		if(frequency >=900000000000ll)frequency=900000000000ll;
		if(frequency < fw-samplewidth*0.5){
			fw=frequency;
			if(fw - 0.5*samplewidth < 0.0)fw=0.5*samplewidth;
			if(fw - 0.5*samplewidth < fc-0.5*samplerate){
				fc = fw+0.5*samplewidth-0.5*samplerate;
				if(fc < 0.5*samplerate)fc=0.5*samplerate;
			}else{
				f=frequency;
				initPlay();
				return 1;
			}
		}else if(frequency > fw+samplewidth*0.5){
			fw=frequency;
			if(fw + 0.5*samplewidth > fc+0.5*samplerate){
				fc=fw-0.5*samplewidth+0.5*samplerate;
			}else{
				f=frequency;
				initPlay();
				return 1;			
			}
			if(frequency > fw + 0.5*samplewidth)frequency=fw + 0.5*samplewidth-bw/2;
		}else{
			f=frequency;
			initPlay();
			return 1;
		}
		
		try{
			device->setFrequency(SOAPY_SDR_RX, channel, fc);
		} catch(const std::exception &e) {
			std::string streamExceptionStr = e.what();
			fprintf(stderr,"setCenterFrequency Error: %s\n",streamExceptionStr.c_str());
			device->setFrequency(SOAPY_SDR_RX, channel, fc);
			return 1;
		}
		f=frequency;
		initPlay();
	}
	return 1;
}
int sdrClass::setFrequency(double frequency)
{
	if(frequency < 0)frequency = -frequency;
	if(frequency == 0.0)frequency=samplerate;
	if(frequency >=900000000000ll)frequency=900000000000ll;
		
	if((frequency-fc) > 0.5*samplerate){
		if(inData != IN_RADIO){
		    frequency=fc+0.5*samplerate;
		}else{
			try{
				device->setFrequency(SOAPY_SDR_RX, channel, frequency);
				fc=frequency;
				fw=fc;
				f=frequency;
			} catch(const std::exception &e) {
				std::string streamExceptionStr = e.what();
				fprintf(stderr,"setFrequency Error: %s\n",streamExceptionStr.c_str());
				device->setFrequency(SOAPY_SDR_RX, channel, fc);
				return 1;
			}
		}
	}else if((frequency-fc) < -0.5*samplerate){
		if(inData != IN_RADIO){
		    frequency=fc-0.5*samplerate;
		}else{
			try{
				device->setFrequency(SOAPY_SDR_RX, channel, frequency);
				fc=frequency;
				fw=fc;
				f=frequency;
			} catch(const std::exception &e) {
				std::string streamExceptionStr = e.what();
				fprintf(stderr,"setFrequency Error: %s\n",streamExceptionStr.c_str());
				device->setFrequency(SOAPY_SDR_RX, channel, fc);
				return 1;
			}
		}
	}

	f=frequency;
	initPlay();
	return 0;
}
int sdrClass::readFile(){

	extern soundClass *s;

	while(s->audioSync == 1 && doWhat == 2)Sleep2(5);
	
	float *buff=bS->buff[witch % NUM_DATA_BUFF];
	
 	int ret=fread(buff, sizeof(float)*2, size, inFile);
 
 	retFlag=ret;
 	
 	if(ret <= 0){
 		rewind(inFile);
 	}
 	 	
 	s->audioSync=1;
 
	return ret;
}
int sdrClass::readSDR(){

   
	float *buff=bS->buff[witch % NUM_DATA_BUFF];
	 
	void *buffs[] = {buff};
	
	int toRead=size;
	
	int ret=0;
	
	int count=0;
		 
	while(doWhat == 2){
		long long timeNs=0;	
		int flags=0;
		
		buffs[0]=buff+2*count;
		
		unsigned int iread;
		
		iread=toRead;
		if(iread > MTU)iread=MTU;
		
		 ret = device->readStream(rxStream, buffs, iread, flags, timeNs, 100000L);
		 
		 retFlag=ret;
	 
		 timeNs++;
				   
		if(ret <= 0){
		   printf("readSDR readStream ret %d iread %d MTU %ld\n",ret,iread,(long)MTU);
		 //  if(Debug > 0)fprintf(stderr,"readStream ret %d \n",ret);
		   break;
		}else if(ret < toRead){
			count += ret;
			toRead=toRead-ret;
		}else{
			break;
		}
	}

	return 0;
}

int sdrClass::readPipe(){
   
   //static long int countl=0;
   
	float *buff=bS->buff[witch % NUM_DATA_BUFF];
	 
	int rec=size;
	
	int ret=0;
	
	int count=0;
	
	ssize_t iread=rec;
	
	s->audioSync = 1;
	
	while(doWhat == 2){
	
		while(s->audioSync == 1 && doWhat == 2)Sleep2(5);
			
		float *buffs=buff+2*count;
		
		iread=rec;
					
		if(data_type == TYPE_SHORT){
		//fprintf(stderr,"readPipe iread %ld size %d toRead %d data_type %d\n",iread,size,rec,data_type);
			ret = fread(buffs, sizeof(short)*2,iread,stdin);
			if(ret <= 0)break;
			short int *in=(short int *)buffs;
			int ne=ret-1;
			for(int n=0;n<ret;++n){
				buffs[2*ne]=in[2*ne]/32000.0;
				buffs[2*ne+1]=in[2*ne+1]/32000.0;
				--ne;
			}
		}else if(data_type == TYPE_UNSIGNEDSHORT){
		//fprintf(stderr,"readPipe iread %ld size %d toRead %d data_type %d\n",iread,size,rec,data_type);
			ret = fread(buffs, sizeof(unsigned short)*2,iread,stdin);
			if(ret <= 0)break;
			unsigned short int *in=(unsigned short int *)buffs;
			int ne=ret-1;
			for(int n=0;n<ret;++n){
				buffs[2*ne]=((in[2*ne]/32767.0)-1.0);
				buffs[2*ne+1]=((in[2*ne+1]/32767.0)-1.0);
				--ne;
			}
		}else if(data_type == TYPE_UNSIGNED){
		//fprintf(stderr,"readPipe iread %ld size %d toRead %d data_type %d\n",iread,size,rec,data_type);
			ret = fread(buffs, sizeof(unsigned char)*2,iread,stdin);
			if(ret <= 0)break;
			unsigned char *in=(unsigned char *)buffs;
			int ne=ret-1;
			for(int n=0;n<ret;++n){
				buffs[2*ne]=((in[2*ne]/128.0)-1.0);
				buffs[2*ne+1]=((in[2*ne+1]/128.0)-1.0);
				--ne;
			}
		 }else{
			ret = fread(buffs, sizeof(float)*2,iread,stdin);		 
		 }
		 retFlag=ret;
	 				   
		if(ret <= 0){
		 //  if(Debug > 0)fprintf(stderr,"readStream ret %d \n",ret);
		   break;
		}else if(ret < rec){
			count += ret;
			rec -= ret;
		}else{
			break;
		}

		//fprintf(stderr,"%p %p \n",s->bS,bS);
		
		s->audioSync=1;
		
	}
	
	//fprintf(stderr,"readPipe countl %ld size %d ret %d inFilenum %d\n",countl++,size,ret,inFilenum);

	return 0;


}

int sdrClass::readUDP(){

	if(doWhat != 2)return 1;
   
	while(l && (l->ibuff == 0)){
		if(doWhat != 2)return 1;
		Sleep2(5);
	}
	
	if(!l)return 1;

	float *buff=bS->buff[witch % NUM_DATA_BUFF];
	
	float *data=(float *)l->buff1;
	
	if(!data)return 1;
	
	//fprintf(stderr,"l->size %d size %d\n",l->size,size);

	for(int n=0;n<l->size*2;++n){
		buff[n]=data[n];
	}
	
	l->ibuff=0;
	
	return 0;
}

int sdrClass::readTCPIP(){

	if(doWhat != 2)return 1;
   
	while(l && (l->ibuff == 0)){
		if(doWhat != 2)return 1;
		Sleep2(5);
	}
	
	if(!l)return 1;

	float *buff=bS->buff[witch % NUM_DATA_BUFF];
	
	float *data=(float *)l->buff1;
	
	if(!data)return 1;
	
	//fprintf(stderr,"l->size %d size %d\n",l->size,size);

	for(int n=0;n<l->size*2;++n){
		buff[n]=data[n];
	}
	
	l->ibuff=0;
	
	return 0;
}

int sdrClass::rxBuffer()
{
	class sdrClass *rx=this;
	

	while(1)
	{
		if(rx->doWhat < 0){
			rx->doWhat=-2;
			//fprintf(stderr,"rxBuffer Exit\n");
			return 0;
		}
	     switch(rx->doWhat){
	     case 0:
	     	Sleep2(50);
			//fprintf(stderr,"rxBuffer Sleep\n");
	        break;
	     case 1:
	        return 0;
		 case 2:
			//auto t1 = std::chrono::high_resolution_clock::now();
			
			if(inData == IN_FILE){
				readFile();
			}else if(inData == IN_PIPE){
				readPipe();
			}else if(inData == IN_RADIO){
				readSDR();
			}else if(inData == IN_UDP){
				readUDP();
			}else if(inData == IN_TCPIP){
				readTCPIP();
			}
	        if(rx->doWhat == 2){
	        	float *buff=bS->buff[rx->witch % NUM_DATA_BUFF];
/*     	
 	static FILE *out11;
 	if(!out11)out11=fopen("test1_IQ_101500000_10000000_fc.raw","wb");
 	if(out11)fwrite(buff,8,retFlag,out11);
*/       
 	
	            if(rx->IQSwap){
                    for(int n=0;n<rx->size;++n){
                        float save;
                        save=buff[2*n];
                        buff[2*n]=buff[2*n+1];
                        buff[2*n+1]=save;
                    }
                }
                
                int idc=0;
                if(idc){
                	double average=0;
                    for(int n=0;n<rx->size*2;++n){
                    	average += buff[n];
                    }
                    average /= (rx->size*2);
                    
                    for(int n=0;n<rx->size*2;++n){
                    	buff[n] -= average;
                    }
                    
                    fprintf(stderr,"average %g\n",average);
          
                }

	        	if(!iWait)bS->pushBuff(rx->witch);

	        	float *buff2=bS2->buff[rx->witch % NUM_DATA_BUFF];

				for(int n=0;n<rx->size*2;++n){
					buff2[n]=buff[n];
				}
	        	
	        	if(!iWait)bS2->pushBuff(rx->witch);
	        	
	        	
             	++rx->witch;

             	if(saveCall){ 	
             	//	fprintf(stderr,"rx->witch %ld saveLength %ld\n",(long)rx->witch,(long)saveLength);
					for(int k=0;k<saveLength;++k){
						if(k < rx->size){
							saveBuff[2*k]=buff[2*k];
							saveBuff[2*k+1]=buff[2*k+1];
						}else{
							saveBuff[2*k]=0;
							saveBuff[2*k+1]=0;
						}
                    }
             	}
				saveFlag=1;
			//	fprintf(stderr,"rxBuff saveCall %d witch %d\n",saveCall,rx->witch);
				
	        }
	      //  auto t2 = std::chrono::high_resolution_clock::now();
		   // std::chrono::duration<double> difference = t2 - t1;
		   // printf("Time %g rx->witch %d\n",difference.count(),rx->witch);
	//		fprintf(stderr,"rxBuffer doWhat %p %d\n",&rx->doWhat,rx->doWhat);
	        break;
	     }
	     
	}
	return 0;
}

int sdrClass::initPlay()
{
//    ALenum error;
    
    
    {
    	double pi;
    	pi=4.0*atan(1.0);
    	dt=1.0/(double)samplerate;
    	sino=0;
    	coso=1;
    	w=2.0*pi*(fc - f);
    	sindt=sin(w*dt);
    	cosdt=cos(w*dt);
    	if(Debug)mprint("fc %f f %f dt %g samplerate %d\n",fc,f,dt,samplerate);
    }
    
	return 0;
}

int sdrClass::Process()
{
	
	sdrClass *rx=this;
	
	struct Filters ff;			
	
	zerol((char *)&ff,sizeof(f));

	ff.thread=rx->thread++;
	
	ff.aminGlobal=0;
	
	ff.amaxGlobal=0;
	
	//fprintf(stderr,"Process %d\n",ff.thread);
	
	setFilters(&ff);
	
	float *buff1=NULL;
	
	float *buff2=NULL;
	
	int size1=2*rx->size*sizeof(float);
	int size2=2*rx->faudio*sizeof(short);
	if(size1 < size2)size1=size2;
	if(size2 < size1)size2=size1;
	
	buff1=(float *)cMalloc(size1,4567);
    if(!buff1){
        mprint("2 cMalloc Errror Requested %ld\n",(long)(size1));
       	goto OutOfHere;
    }
    zerol((char *)buff1,size1);

	
	buff2=(float *)cMalloc((size_t)(size2),9837);
    if(!buff2){
        mprint("3 cMalloc Errror Requested %ld\n",(long)(size2));
        goto OutOfHere;
    }
    zerol((char *)buff2,(unsigned long)(size2));
	
	while(rx->frame >= 0 && iqToAudio >= 0){
		//auto t1 = std::chrono::high_resolution_clock::now();
	    int ret=doFilter(buff1,buff2,&ff);
		if(ret){
			//fprintf(stderr,"Sleep2\n");
			Sleep2(5);
		}else{
			doAudio(buff2,buff1,&ff);
			//auto t2 = std::chrono::high_resolution_clock::now();
			//std::chrono::duration<double> difference = t2 - t1;
			//printf("Time %g thread %d size %d\n",difference.count(),ff.thread,rx->size);
		}
	}
	
OutOfHere:
	
	if(buff1)cFree((char *)buff1);
	
	if(buff2)cFree((char *)buff2);
	
	if (ff.iqSamplerd)msresamp_crcf_destroy(ff.iqSamplerd);
	
	if (ff.iqSampler2)msresamp_rrrf_destroy(ff.iqSampler2);
	
    if(ff.fShift)nco_crcf_destroy(ff.fShift);
    
    if(ff.demod)freqdem_destroy(ff.demod);
    
    if(ff.demodAM)ampmodem_destroy(ff.demodAM);
    
    iqToAudio=-2;
    

	return 0;
}
int sdrClass::setFilters(struct Filters *f)
{
	sdrClass *rx=this;

    // double shift=rx->f-rx->fc;
    
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
        rx->bw=1000.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }
    
    rx->Ratio = (float)(rx->bw/ rx->samplerate);
    
    ratio= (float)(rx->faudio/rx->bw);
    
    f->demod=freqdem_create(0.5);
    
#ifdef LIQUID_VERSION_4
    f->demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
 #else
    f->demodAM = ampmodem_create(0.5, mode, iflag);
#endif

    f->iqSamplerd  = msresamp_crcf_create(rx->Ratio, As);
    
    f->iqSampler2 = msresamp_rrrf_create(ratio, As);
    
    //msresamp_crcf_print(f->iqSampler);
    
    
    f->fShift = nco_crcf_create(LIQUID_NCO);
    
    f->amHistory=0;
    
    return 0;
    	
}

int sdrClass::doFilter(float *wBuff,float *aBuff,struct Filters *f)
{

	sdrClass *rx=this;

 	int ip=bS->popBuff();
 	if(ip < 0){
 	     return 1;
 	}
 	
 	int witch=ip % NUM_DATA_BUFF;
 	
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


		float *buf=bS->buff[witch];
		
		float *buf2=wBuff;
				
		if(outFile){
			if(data_type == TYPE_FLOAT){
				size_t ret=fwrite(buf, 2*sizeof(float), rx->size,outFile);
				if(ret != (size_t)rx->size){
					fprintf(stderr,"fwrite Error ret %ld bytes %ld\n",(long)ret,(long)rx->size);
				}
			}else if(data_type == TYPE_SHORT){
				double amin=  1e33;
				double amax= -1e33;
				for(int n=0;n<rx->size;++n){
					char out[4];
					float r=buf[2*n];
					float i=buf[2*n+1];
					short rs=32000.0*r;
					if(r > amax)amax=r;
					if(r < amin)amin=r;
					out[0]=rs & 0xff;
					out[1]=(rs >> 8) & 0xff;
					short is=32000.0*i;
					if(i > amax)amax=i;
					if(i < amin)amin=i;
					out[2]=is & 0xff;
					out[3]=(is >> 8) & 0xff;
					size_t ret=fwrite(out, 1, 4,outFile);
					if(ret != 4){
						fprintf(stderr,"fwrite Error ret %ld bytes %ld\n",(long)ret,(long)4);
					}
				}
				//fprintf(stderr,"amin %g amax %g\n",amin,amax);
			}else if(data_type == TYPE_UNSIGNEDSHORT){
				double amin=  1e33;
				double amax= -1e33;
				for(int n=0;n<rx->size;++n){
					char out[4];
					float r=buf[2*n];
					float i=buf[2*n+1];
					unsigned short rs=32767*(1+r);
					if(r > amax)amax=r;
					if(r < amin)amin=r;
					out[0]=rs & 0xff;
					out[1]=(rs >> 8) & 0xff;
					unsigned short is=32767*(1+i);
					if(i > amax)amax=i;
					if(i < amin)amin=i;
					out[2]=is & 0xff;
					out[3]=(is >> 8) & 0xff;
					size_t ret=fwrite(out, 1, 4,outFile);
					if(ret != 4){
						fprintf(stderr,"fwrite Error ret %ld bytes %ld\n",(long)ret,(long)4);
					}
				}
			}else if(data_type == TYPE_UNSIGNED){
				double amin=  1e33;
				double amax= -1e33;
				for(int n=0;n<rx->size;++n){
					char out[4];
					float r=buf[2*n];
					float i=buf[2*n+1];
					unsigned char rs=127*(1.0+r);
					if(r > amax)amax=r;
					if(r < amin)amin=r;
					out[0]=rs;
					unsigned char is=127*(1.0+i);
					if(i > amax)amax=i;
					if(i < amin)amin=i;
					out[1]=is;
					size_t ret=fwrite(out, 1, 2,outFile);
					if(ret != 2){
						fprintf(stderr,"fwrite Error ret %ld bytes %ld\n",(long)ret,(long)4);
					}
				}
				//fprintf(stderr,"amin %g amax %g\n",amin,amax);
			}
		}
		
				
		double sint,cost;

        for (int k = 0 ; k < rx->size ; k++){
            float r = buf[k * 2];
            float i = buf[k * 2 + 1];
            if(rx->dt > 0){
                buf2[k * 2] = (float)(r*rx->coso - i*rx->sino);
                buf2[k * 2+ 1] = (float)(i*rx->coso + r*rx->sino);
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
    
	buf=aBuff;
	
    unsigned int num;
    unsigned int num2;
    
    num=0;
    num2=0;

    msresamp_crcf_execute(f->iqSamplerd, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf, &num);  // decimate
            
    if(rx->decodemode == MODE_FM || rx->decodemode == MODE_NBFM){

		freqdem_demodulate_block(f->demod, (liquid_float_complex *)buf, (int)num, (float *)buf2);

        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate

        //mprint("2 rx->size %d num %u num2 %u\n",rx->size,num,num2);

    }else if(rx->decodemode == MODE_AM || rx->decodemode == MODE_NAM){
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

        
	return 0;
}

int sdrClass::doAudio(float *aBuff,float *wBuff,struct Filters *f)
{
	int short *data;
	int audioOut;
	int length;

	sdrClass *rx=this;
	
	bS->mutexo.lock();
	audioOut=rx->audioOut;
	data=bS->buffa[rx->audioOut++ % NUM_ABUFF];
	bS->mutexo.unlock();
	
	float *buff=aBuff;

	double dnom,gain;
	
	gain=rx->gain;
	
	if(gain <= 0.0)gain=1.0;
	
	double amin=1e30;
	double amax=-1e30;
	double average=0;
	
	length=rx->faudio/rx->ncut;
	
	//fprintf(stderr," sdrClass::doAudio rx->faudio %g\n",rx->faudio);
	
	for (int n=0; n<length; n++ ) {
		double v;
		v=buff[n];
        average += v;
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}
	
	average /= length;
		
	//fprintf(stderr,"rx->faudio %g amin %g amax %g\n",rx->faudio,amin,amax);
	
    amin -= average;

    amax -= average;
	

    if(f->aminGlobal == 0.0)f->aminGlobal=amin;

    f->aminGlobal = 0.9*f->aminGlobal+0.1*amin;

    amin=f->aminGlobal;


    if(f->amaxGlobal == 0.0)f->amaxGlobal=amax;

    f->amaxGlobal = 0.9*f->amaxGlobal+0.1*amax;

    amax=f->amaxGlobal;
	
	
    if((amax-amin) > 0){
        dnom=0.90*65535.0/(amax-amin);
    }else{
        dnom=65535.0;
    }
		
	//fprintf(stderr,"amin %g amax %g average %g dnom %g\n",amin,amax,average,dnom);
	
	for(int k=0;k<length;++k){
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

	if(audioThreads > 0){
		for(int n=0;n<100;++n){
			data[n]=data[n]*(float)n/100.0;
			data[length-1-n]=data[length-1-n]*(float)n/100.0;
		}
	}


	sendAudio(data,length);
	
	bS->pushBuffa(audioOut);

	return 0;
}

int sdrClass::printDevices(void)
{

    if(resultsEnumerate.size() < 1){
    	if(doEnumerate(deviceString))return 1;
    }
 
    std::vector<SoapySDR::Kwargs> results=resultsEnumerate;
       
    mprint("Number of Devices Found: %ld\n",(long)results.size());
    
    if(results.size() < 1)return 1;
        
    SoapySDR::Kwargs deviceArgs;
    
    for(unsigned int k=0;k<results.size();++k){
    		mprint("SDR device =  %ld ",(long)k);
			deviceArgs = results[k];
			for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
				if (it->first == "label")mprint(" %s = %s\n",it->first.c_str(), it->second.c_str());
			}
    }
    
    mprint("\n");
    
    return 0;
}

int sdrClass::printInfo(void)
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
static double rtime(void)
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
