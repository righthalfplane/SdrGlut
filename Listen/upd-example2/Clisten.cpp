#include "Clisten.h"
#include <cstdio>
#include <chrono>
#include <thread>

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

static int copyl(char *p1,char *p2,long n);

int Sleep2(int ms)

{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return 0;
}

int launchThread(void *data,int (*sageThread)(void *data))
{
    std::thread(sageThread,data).detach();
    return 0;
}


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
	
}

Listen::~Listen()
{
	closesocket(clientSocket);
}
int Listen::freeFilters(struct Filters2 *f)
{
    
    return 0;

}
int Listen::setFilters(class Listen *rx,struct Filters2 *f)
{


    return 0;
    
}

int Listen::mix(float *buf1,float *buf2)
{
    
	return 0;
}

int ListenSocket(void *rxv)
{
    
    class Listen *l=(class Listen *)rxv;
    
	time_t start,total;
	time_t ship;
	long size;
	
	size=1024;
	
	l->output=(complex<float> *)malloc(size*sizeof(complex<float>));
	
	FILE *in=NULL;
	
	if(!in)in=fopen("junk.raw","wb");

	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 825 - COPYRIGHT 2020-2021. Start **\n");
	fprintf(stderr,"******************************************************\n");

	start=time(&ship);
	
	l->Bytes=0;
	
    l->ncommand=0;

	while(1){
	    int ret=l->netRead(l->clientSocket,(char *)l->output,size);
	    if(ret > 0)break;
	    fwrite((char *)l->output,size,1,in);
	    fprintf(stderr,"ret %d size %ld\n",ret,size);

	}
	
	if(in)fclose(in);
	
	if(l->output)free(l->output);
	l->output=NULL;

	l->ibuff= -1;

    total=time(&ship)-start;
	if(!total)total=1;
    fprintf(stderr,"%ld Seconds To Receive %ld Bytes (%ld Bytes/s)\n",
                 (long)total,l->Bytes,(long)(l->Bytes/total));
	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 825 - COPYRIGHT 2020-2021. Done  **\n");
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


int Listen::setFrequency(double frequency)
{
	
	return 0;
}
int Listen::setCenterFrequency(double frequency,double sampleRate)
{
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
	                        
		this->ibuff=-2;
		launchThread((void *)this,ListenSocket);
	
		while(this->ibuff != -1){
			Sleep2(10);
		}
	                                             
 	     shutdown(clientSocket,2);
	     close(clientSocket);
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
	memset((char *)&serverSocketAddr,0,sizeof(serverSocketAddr));
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

	memset((char *)&serverSocketAddr,0,sizeof(serverSocketAddr));

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
