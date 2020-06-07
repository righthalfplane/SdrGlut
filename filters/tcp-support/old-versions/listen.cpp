#include <stdio.h>

#include <complex>

#include <iostream>

#include <liquid/liquid.h>

#include <RtAudio.h>

#include <unistd.h>

using namespace std;


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


int getPortAndName(char *in,unsigned int *hostAddr,unsigned short *Port);

static SOCKET connectToServer(char *serverName,unsigned short *Port);

static int copyl(char *p1,char *p2,long n);

static int zerol(char *p,long n);

int CheckSocket(SOCKET serverSocket,int *count,int ms);

SOCKET createService(unsigned short *Port);

int ListenSocket(SOCKET clientSocket);

SOCKET tcp;

SOCKET serverSocket;

SOCKET clientSocket;

int Debug;

long Bytes;

unsigned short Port;

int netRead(SOCKET clientSocket,char *buff,long n);

struct playData{
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
	complex<float> *buff;
	float Ratio;
	double fOut;
	msresamp_crcf iqSampler;
    iirfilt_crcf lowpass;
    int samplerate;
    double aminGlobal;
    double amaxGlobal;
};

struct playData play;

int main(int argc,char *argv[])
{
	struct sockaddr_in clientSocketAddr;
	unsigned int hostAddr;
	unsigned int addrLen;
	unsigned short Port;







	Debug=1;

	if(getPortAndName(argv[1],&hostAddr,&Port))exit(1);

	serverSocket=createService(&Port);
    if(serverSocket == -1){
          printf("Create Port %d Failed\n",Port);
	      exit(1);
	}

	addrLen=sizeof(clientSocketAddr);
	
	
	while(1){
		int count;
		int ret;

	    ret=CheckSocket(serverSocket,&count,300);


		if(ret <= 0){
		    printf("ret %d\n",ret);
			continue;
        }
        
		clientSocket=accept(serverSocket,(struct  sockaddr  *)&clientSocketAddr,
	                        &addrLen);
	                        
	    ListenSocket(clientSocket);
	    
	    shutdown(clientSocket,2);
	    close(clientSocket);
	    
	    printf("accept \n");
	    
	    break;

	
	}

	return 0;
}

int readString(int clientSocket,char *buff,long nbyte)
{
	return netRead(clientSocket,buff,nbyte);	
}
static int getLong(int clientSocket,long *n)
{
	unsigned char c[4];

	if(readString(clientSocket,(char *)c,4L))return 1;

	*n = c[0] + ((long)c[1] << 8) 
	          + ((long)c[2] << 16) + ((long)c[3] << 24);

	return 0;
}

int readCommand(SOCKET clientSocket,char *buff,long *size)
{
	long n;

	n=4;
	if(readString(clientSocket,buff,n))return 1;
	buff[n]=0;
	if(getLong(clientSocket,size))return 1;
	return 0;
	
}

int ListenSocket(SOCKET clientSocket)
{
	unsigned long start,total;
	time_t ship;
	char buff[256];
	float *buffdata;
	long buffsize;
	long size;

	fprintf(stderr,"******************************************************\n");
	fprintf(stderr,"**  listen 01 - COPYRIGHT 2020. Start **\n");
	fprintf(stderr,"******************************************************\n");

	start=time(&ship);
	
	Bytes=0;
	
	buffdata=NULL;

//	countHDF=0;

	while(1){
	    if(readCommand(clientSocket,buff,&size))return 1;
		printf("buff %s size %ld\n",buff,size);
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
		    printf("fc %g samplerate %g\n",buff[0],buff[1]);
	    }else if(!strcmp(buff,"FLOA")){
	        if(Debug){
				printf("FLOA\n");
				fflush(stdout);
		    }
		    if(size > buffsize){
		       if(buffdata)free(buffdata);
		       buffdata=(float *)malloc(size);
		       buffsize=size;
		    }
		    netRead(clientSocket,(char *)buffdata,size);
		    printf("size %ld\n",size);
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

int netRead(SOCKET clientSocket,char *buff,long n)
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
int CheckSocket(SOCKET serverSocket,int *count,int ms)
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

SOCKET createService(unsigned short *Port)
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

int getPortAndName(char *in,unsigned int *hostAddr,unsigned short *Port)
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
static SOCKET connectToServer(char *serverName,unsigned short *Port)
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
