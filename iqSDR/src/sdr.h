#ifndef __SDRH__

#define __SDRH__

#include "firstFile.h"
#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>
#include <SoapySDR/Logger.h>

#include "SocketDefs.h"


#if __has_include(<liquid/liquid.h>)
#include <liquid/liquid.h>
#else
#include <liquid.h>
#endif

#if __has_include(<rtaudio/RtAudio.h>)
#include <rtaudio/RtAudio.h>
#else
#include <RtAudio.h>
#endif

#include <iostream>
#include <csignal>
#include <mutex>
#include <cstdio>

#include <chrono>

#include <thread>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include <time.h>

#include "mThread.h"

#include <stdio.h>

#include "bStack.h"

#include "sound.h"

#include <string>

#include "Clisten.h"

/*
#define MODE_AM   0
#define MODE_NAM  1
#define MODE_FM   2
#define MODE_NBFM 3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6
*/

#define TYPE_FLOAT      		0
#define TYPE_SHORT      		1
#define TYPE_UNSIGNEDSHORT      2
#define TYPE_SIGNED     		3
#define TYPE_UNSIGNED   		4


#define NUM_ABUFF 10

#define NUM_DATA_BUFF 10

#define IN_RADIO 0
#define IN_FILE  1
#define IN_PIPE  2
#define IN_TCPIP 3
#define IN_UDP	 4



class sdrClass;

extern int sendAudio(sdrClass *sdr,int short *data,int length);


typedef int (sdrClass::*A_pmf_t)();

int mstrncpy(char *out,char *in,long n);

extern int doEnumerate(char *string);

extern std::vector<SoapySDR::Kwargs> resultsEnumerate;

extern char *strsave(char *s,int tag);

class soundClass;

class BasicPane2;

class BasicPane;

extern soundClass *s;

struct Filters{
	int np;
    ampmodem demodAM;
	freqdem demod;
	msresamp_crcf iqSamplerd;
	msresamp_rrrf iqSampler2;
	nco_crcf fShift;
	int thread;	
	double amHistory;
	double aminGlobal;
    double amaxGlobal;
};

class sdrClass
{
public:
	sdrClass();
	~sdrClass();
	void setMode(std::string mode);
	int setFrequency(double frequency);
	int setFrequencyFC(double frequency);
	int setDataSave(int length);
	int setup(int argc, char *argv[]);
	int run();
	int readUDP();
	int readTCPIP();
	int readSDR();
	int readPipe();
	int readFile();
	int initPlay();
	int findRadio();
	int rxBuffer();
	int startPlay();
	int playRadio();
	int stopPlay();
	int Process();
	int setFilters(struct Filters *f);
	int doFilter(float *wBuff,float *aBuff,struct Filters *f);
	int doAudio(float *aBuff,float *wBuff,struct Filters *f);
	int startSound();
	int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );
	int printInfo(void);
	
	int waitPointer(std::string name,volatile int *pointer,int flag);
	
	int setSampleWidth(double value);
	
	int setCenterFrequency(double value);
	
//	static int printAudio(void);
	int printDevices(void);
	
    double samplerate;
    double samplewidth;
    double samplescale;
	int Debug;
	double gain;
	double fc;
	double fw;
	double f;
	double dt;
	double sino;
	double coso;
	double sindt;
	double cosdt;
	double w;
	double bw;
	int decodemode;
	int audiodevice;

	SoapySDR::Stream *rxStream;
	SoapySDR::Device *device;
	
	int data_type;

	float *saveBuff;
	int saveLength;
	
    volatile int audioSync;
    
    volatile int audioOut;
    
    volatile int saveFlag;
    
    volatile int saveCall;
    
   	volatile int thread; 
   	
	volatile int doWhat;
	
	volatile int witch;
	
    volatile int iWait;
    
    volatile int frame;
    
    volatile int iqToAudio;
    
	static volatile int soundRun;
	
	volatile int retFlag;
        
    double bandwidth;
    
    unsigned long MTU;
    
    float faudio;
    
    float Ratio;
    
    int size; 
    
    int channel;
    
    unsigned int deviceNumber;
    

 	int al_state;
 	
 	std::string set[20];
 	std::string value[20];
 	int setcount;
 	
 	double timeout;
 	double timestart;
 	FILE *outFile;
 	FILE *inFile;
    int idump;
    
    
    volatile int fillBuffer;
    float *sendBuff;

    
    double aminGlobal;
    double amaxGlobal;
 	
 	double PPM;
 	
 	double rf_gain;
 	
 	int IQSwap;
 	
 	int ncut;
 	
 	int hasGainMode;
 	
 	int inData;
 	int inFilenum;
 	
 	std::vector<double> rate;
 	std::vector<double> band;
 	std::vector<std::string> antennaNames;
 	
 	SoapySDR::Range rxGainRange;
 
 	std::vector<SoapySDR::Range> rxGainRangeList;
 	std::vector<std::string> rxGainNames;
 	
 	cStack *bS;
 	
 	cStack *bS2;
 	
 	std::vector<std::string> streamFormat;
 	
    BasicPane2 *gBasicPane2;

    BasicPane *gBasicPane;
 	 	
 	
 	class Listen *l;
 	
 	char *deviceString;
 	
 	int audioThreads;
 	
 	std::chrono::time_point<std::chrono::high_resolution_clock> timeFileRead;
 	
 	double timeFileWait;
 	
 	std::chrono::time_point<std::chrono::high_resolution_clock> timePipeFile;
 	
 	double timePipeWait;
 	
};


#endif
