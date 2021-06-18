#include "SocketDefs.h"
#include "Utilities.h"
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <vector>
#include <chrono>
#include <thread>
#include <complex>
#include <iostream>
#include <cstring>

using namespace std;

static int copyl(char *p1,char *p2,long n);

SOCKET connectToServer(char *serverName,unsigned short *Port);

int writeLab(SOCKET toServerSocket,char *Type,long size);

int doEnd(SOCKET tcp);

SOCKET tcp;

unsigned short Port;

long Bytes;

double fc=1e6;
double sampleRate=2000000;

int netWrite(SOCKET toServerSocket,char *buffout,long HdfSize);

int writeStat(SOCKET toServerSocket);
/*
int main(int argc,char *argv[])
{

	fprintf(stdout,"sizeof(complex<double>) %ld\n",(long)sizeof(complex<double>));

	fprintf(stdout,"sizeof(complex<float>) %ld\n",(long)sizeof(complex<float>));

 	fprintf(stdout,"sizeof(complex<short int>) %ld\n",(long)sizeof(complex<short int>));

	fprintf(stdout,"sizeof(complex<signed char>) %ld\n",(long)sizeof(complex<signed char>));
	
	complex<signed char> r,r2;
	
	r.real(127);
	r2.real(-128);
	
	fprintf(stdout,"r.real() %g %g\n",(double)r.real(),(double)r2.real());
	
	complex<short int> r3,r4;
	
	r3.real(32767);
	r4.real(-32768);
	
	fprintf(stdout,"r.real() %g %g\n",(double)r3.real(),(double)r4.real());
	
	
	tcp=(SOCKET)connectToServer((char *)argv[1],&Port);
    if(tcp == -1){
      fprintf(stderr,"connect failed\n");
      throw std::runtime_error(socket_strerror(SOCKET_ERRNO));
	}
	
	long size=500;

	if(writeStat(tcp))return 1;
	
	if(writeLab(tcp,(char *)"CATN",size))return 1;

	size=600;

	if(writeLab(tcp,(char *)"CATE",size))return 1;

	
	if(tcp >= 0){
	    doEnd(tcp);
	    shutdown(tcp,2);
	    close(tcp);
	}
	
	return 0;
}
*/
int writeStat(SOCKET toServerSocket)
{
	double buff[2];
	
	buff[0]=fc;
	buff[1]=sampleRate;
	
	long size=2;

	if(writeLab(toServerSocket,(char *)"STAT",size))return 1;

	if(netWrite(toServerSocket,(char *)buff,(long)(2*sizeof(double))))return 1;
	
	return 0;
}

int netWrite(SOCKET toServerSocket,char *buffout,long HdfSize)
{
	long n;
	
	Bytes += HdfSize;
	while(HdfSize > 0){
		n=HdfSize;
		if(n > 120000)n=120000;
        n=::send(toServerSocket,buffout,n,0);
		if(n < 0){
			fprintf(stderr,"Write Socket Error (%ld)\n",(long)errno);
			return 1;
		}
		buffout += n;
		HdfSize -= n;
	}
        
	return 0;
}

int writeString(SOCKET toServerSocket,char *buffout,long HdfSize)
{
	return netWrite(toServerSocket,buffout,HdfSize);
}

int writeLong(SOCKET toServerSocket,long i)
{
	unsigned char c[4];
	
	c[0] = (unsigned char)(i & 255);
	c[1] = (unsigned char)((i >> 8) & 255);
	c[2] = (unsigned char)((i >> 16) & 255);
	c[3] = (unsigned char)((i >> 24) & 255);

	if(writeString(toServerSocket,(char *)c,4L))return 1;

	return 0;
}


int writeLab(SOCKET toServerSocket,char *Type,long size)
{
	
	if(writeString(toServerSocket,Type,4L))return 1;
	if(writeLong(toServerSocket,(long)size))return 1;
	return 0;
}


int doEnd(SOCKET tcp)
{
	char *endt;

	endt=(char *)"ENDT";

	if(writeLab(tcp,endt,0L))return 1;
	
	return 0;
}

SOCKET connectToServer(char *serverName,unsigned short *Port)
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

    ret=0;
    
	/* oneNeg=0xffffffff; */
	oneNeg = -1L;

	long netsize=200000;

	buf_size=(int)(netsize+30);

	result= -1;

    zerol((char *)&serverSocketAddr, sizeof(serverSocketAddr));

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
	while(Try++ < 4){
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
