#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif


#ifndef  _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS  1
#endif

#include <stdlib.h>

#include "SocketDefs.h"

#include <cstdio>
#include <chrono>

#include <complex>

#include <iostream>

#include <string.h>

#include <stdio.h>

#include <stdlib.h>

#include <liquid/liquid.h>

#include <rtaudio/RtAudio.h>

using namespace std;


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
	
	//int ListenSocket(SOCKET clientSocket);
	
	int netRead(SOCKET clientSocket,char *buff,long n);
	
	int readCommand(SOCKET clientSocket,char *buff,long *size);
	
	int getLong(SOCKET clientSocket,long *n);

	int readString(SOCKET clientSocket,char *buff,long nbyte);

	SOCKET waitForService(char *name);
	
	int setCenterFrequency(double frequency,double sampleRate);
	
	int setFrequency(double frequency);
	
	int freeFilters(struct Filters2 *f);
	
	int setFilters(class Listen *rx,struct Filters2 *f);

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

};

int ListenSocket(void *rxv);

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6
#define MODE_NAM2 7


