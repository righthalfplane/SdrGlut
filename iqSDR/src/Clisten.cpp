#include "firstFile.h"

#include "Clisten.h"

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

#include <stdlib.h>

#include "SocketDefs.h"

#include "mThread.h"

int copyl(char *p1,char *p2,long n);

//static int zerol(char *p,long n);

Listen::Listen(int listenTypei)
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
	
	exitListen=0;
	
	listenType=listenTypei;
	
	cat=0;
	
}

Listen::~Listen()
{
	closesocket(clientSocket);
	fprintf(stderr,"exit Listen %p\n",this);


}
int Listen::freeFilters(struct Filters2 *f)
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
int Listen::setFilters(class Listen *rx,struct Filters2 *f)
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
    
    int size=(int)(0.5+(rate/10.0));
    
    rx->size=size;

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



/*
int launchThread(void *data,int (*sageThread)(void *data))
{
    
    std::thread(sageThread,data).detach();
    
    return 0;
}
*/


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
	
	if(frequency == fc && sampleRate == samplerate)return 0;

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
int Listen::WaitFor()
{
	if(listenType == LISTEN_TCP){
		return WaitForTCP();
	}else{
		return WaitForUDP();
	}
	return 0;
}
int Listen::WaitForUDP()
{

	struct sockaddr_in si_me;

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
	
	ListenSocket(this);

	return 0;
}
int Listen::WaitForTCP()
{
	class Listen *l=(class Listen *)this;
	
	l->serverSocket=l->startService(l->name);

	//fprintf(stderr,"startService %lld\n", serverSocket);
	
	l->addrLen=sizeof(l->clientSocketAddr);
	
	while(1){
		int count;
		int ret;
		
		if(!l)break;

	    ret=l->CheckSocket(l->serverSocket,&count,3000);

		if(ret <= 0){
		    if(l->Debug)fprintf(stderr,"ret %d exitListen %d\n",ret,exitListen);
	     	if(exitListen)break;
			continue;
        }
        
		if(l->Debug)fprintf(stderr,"ret %d\n",ret);
		
		l->clientSocket=accept(l->serverSocket,(struct  sockaddr  *)&l->clientSocketAddr,
	                        &l->addrLen);
	                        
		l->ibuff=-2;
		
	    ListenSocket(l);
	                                             
 	     shutdown(l->clientSocket,2);
 	     
	     closesocket(l->clientSocket);
	     
	     if(exitListen)break;
	}
	
	fprintf(stderr,"Out of WaitFor\n");
	     
	return l->serverSocket;
	
}

SOCKET Listen::waitForService(char *name)
{
	
	copyl(name,this->name,(long)strlen(name)+1);
	
//	std::thread(&Listen::WaitFor,this).detach();
	
	return 0;
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

//int Listen::ListenSocket(SOCKET clientSocket)
//{
//	return 0;
//}

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
//static int zerol(char *p,long n)
//{
//	if(!p)return 1;
//	while(n-- > 0)*p++ = 0;
//	return 0;
//}
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
int copyl(char *p1,char *p2,long n)
{
	if(!p1 || !p2)return 1;

	while(n-- > 0)*p2++ = *p1++;

	return 0;
}
