#ifndef __SDRRECEIVEH__

#define __SDRRECEIVEH__

#include "firstFile.h"
#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>


#include <liquid/liquid.h>

#if __has_include(<rtaudio/RtAudio.h>)
#include <rtaudio/RtAudio.h>
#else
#include <RtAudio.h>
#endif
#include <liquid/liquid.h>

#if __has_include(<hdf/df.h>)
#include <hdf/df.h>
#else
#include <df.h>
#endif


#include <iostream>
#include <csignal>
#include <mutex>
#include <cstdio>
#include <thread>



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include<sys/stat.h>
#include <sys/timeb.h>

#include <time.h>

/*
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif
*/

#include "mThread.h"

void *cMalloc(unsigned long r, int tag);

char *strsave(char *s,int tag);

int cFree(char *p);

int zerol(unsigned char *s,unsigned long n);


#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6

enum{
	Wait,
	Exit,
	Work,
	GoToSleep,
};


#define FILTER_RECTANGULAR     0
#define FILTER_HANN            1
#define FILTER_HAMMING         2
#define FILTER_FLATTOP         3
#define FILTER_BLACKMANHARRIS  4
#define FILTER_BLACKMANHARRIS7 5

/*
#define LIQUID_VERSION_4 1
*/


/*

this does not decode anything - dsdccx -i - -fa -o - | play -q -t s16 -r 8k -c 1 -

g++ -O2 -std=c++11 -Wno-deprecated -o sdrReceive.x sdrReceive.x.cpp mThread.cpp cMalloc.cpp Clisten.cpp -lrtaudio -lSoapySDR -lliquid -framework OpenAL -Wno-return-type-c-linkage

g++ -O2 -std=c++11 -Wno-deprecated -o sdrReceive.x sdrReceive.x.cpp mThread.cpp cMalloc.cpp  Clisten.cpp -lrtaudio -lSoapySDR -lliquid -lopenal -pthread

only a few work - sdrReceive.x -fc 853e6 -f 851.4125e6 -f 851.4375e6 -f 851.6375e6 -f 851.8875e6 -f 851.9125e6 -f 852.0625e6 -f 852.1375e6 -f 852.3875e6 -f 852.4215e6 -f 852.6125e6 -f 852.6375e6 -f 852.8875e6 -f 852.9125e6 -f 853.0750e6 -f 853.4125e6 -device 3 -nbfm -samplerate 10e6 -print 2 -cutoff -80 -pipe -mute | dsd -i - -o pa:1 > junk.out

only a few work - sdrReceive.x -fc 853e6 -f 851.8875e6 -f 852.4375e6 -f 852.6375e6 -f 852.8875e6 -f 853.1000e6 -f 854.2625e6 -f 854.3125e6 -f 854.8125e6 -f 854.8875e6 -f 855.3125e6 -f 855.6125e6 -f 855.7375e6 -f 855.8375e6 -f 856.3875e6 -f 856.6875e6 -device 3 -nbfm -samplerate 10e6 -print 2 -cutoff -80 -pipe -mute | dsd -i - -o pa:1 > junk.out

Many work but time of day - sdrReceive.x -fc 770e6 -f 769.31875e6 -f 769.50625e6 -f 769.55625e6 -f 769.75625e6 -f 769.81875e6 -f 770.01875e6 -f 770.25625e6 -f 770.26875e6 -f 770.51875e6 -f 770.75625e6 -f 770.76875e6 -f 771.05625e6 -f 771.06875e6 -f 771.26875e6 -device 3 -nbfm -samplerate 10e6 -print 2 -cutoff -80 -pipe -mute | dsd -i - -o pa:1 > junk.out

./sdrReceive.x -fc 770e6 -f 769.31875e6 -f 769.50625e6 -f 769.55625e6 -f 769.75625e6 -f 769.81875e6 -f 770.01875e6 -f 770.25625e6 -f 770.26875e6 -f 770.51875e6 -f 770.75625e6 -f 770.76875e6 -f 771.05625e6 -f 771.06875e6 -f 771.26875e6 -device 3 -nbfm -samplerate 10e6 -print 2 -cutoff -80 -pipe > out.raw

./sdrReceive.x -fc 101.1e6 -f 101.5e6 -fm -gain 1 -audiodevice 2

./sdrReceive.x -fc 1e6 -f 0.6e6 -am -gain 1  -timeout 5

./sdrReceive.x -fc 1e6 -f 0.76e6 -am -gain 1 -device 3
./sdrReceive.x -fc 1e6 -f 0.76e6 -am -gain 1 -device 2 -audiodevice 1

./sdrReceive.x -fc 1e6 -f 1.17e6 -am -gain 1

./sdrReceive.x -fc 10.1e6 -f 10.0e6 -am -gain 1

./sdrReceive.x -fc 27.1e6 -f 27.185e6 -gain 1

./sdrReceive.x -fc 101.1e6 -f 101.5e6 -fm -gain 1 -device 3
./sdrReceive.x -fc 101.1e6 -f 101.5e6 -fm -gain 1 -device 2 -audiodevice 1

./sdrReceive.x -fc 103.0e6 -f 103.7e6 -fm -gain 1

./sdrReceive.x -fc 102.0e6 -f 102.1e6 -fm -gain 1

./sdrReceive.x -fc 465e6 -f 467.6125e6 -f 462.6750e6 -f 462.6250e6 -device 3 -nbfm -samplerate 10e6 -print 2 -cutoff -80 -pipe > out.raw

./sdrReceive.x -fc 854.0e6 -f 854.3600e6 -f 854.6360e6 -f 854.6608e6 -f 854.7360e6 -f 854.9620e6 -f 855.0620e6 -f 855.0860e6 -f 855.2620e6 -f 855.5850e6 -f 855.9120e6 -f 856.8360e6 -f 856.8380e6 -f 856.8860e6 -f 857.0860e6 -nbfm -gain 1 -device 3 -samplerate 10e6

./sdrReceive.x -fc 465e6 -f 462.5500e6 -f 462.5625e6 -f 462.5750e6 -f 462.5875e6 -f 462.6000e6 -f 462.6125e6 -f 462.6250e6 -f 462.6375e6 -f 462.6500e6 -f 462.6625e6, -f 462.6750e6 -f 462.6875e6 -f 462.7000e6 -f 462.7125e6 -f 462.7250e6 -f 467.5500e6 -f 467.5675e6 -f 467.5750e6 -f 467.6125e6 -f 467.6000e6 -f 467.6625e6 -f 467.6250e6 -f 467.7125e6 -f 467.6500e6 -f 467.5875e6 -f 467.6750e6 -f 467.6375e6 -f 467.7000e6 -f 467.6875e6 -f 467.7250e6 -device 3 -nbfm -samplerate 10e6 -print 2 -pipe > out.raw

./sdrReceive.x -fc 162.0e6 -f 162.4e6 -nbfm -gain 1 -device 3
./sdrReceive.x -fc 162.0e6 -f 162.4e6 -nbfm -gain 1 -device 2 -audiodevice 1

./sdrReceive.x -fc 102.0e6 -f 102.1e6 -fm -gain 0.9 -faudio 12000 -file test.raw -timeout 10

./sdrReceive.x -fc 101.1e6 -f 101.5e6 -fm -gain 0.9 -faudio 12000 -file test.raw -timeout 10

./sdrReceive.x -fc 102.0e6 -f 102.1e6 -fm -gain 1 -timeout 15 -faudio 12000 -file test.raw

./sdrReceive.x -fc 102.0e6 -f 102.1e6 -fm -gain 1 -timeout 601 -dumpbyminute -faudio 12000

./sdrReceive.x -fc 102.0e6 -f 102.1e6 -fm -gain 1 -dumpbyminute -faudio 12000

./sdrReceive.x -fc 602e6 -f 602.308400e6 -usb -timeout 10 -file test01.raw

./sdrReceive.x -fc 602e6 -f 602.308400e6 -usb -timeout 10 -file test01.raw -device 1

/opt/local/bin/sox -t raw -r 48000 -b 16 -c 1 -L -e signed-integer test01.raw -n stat

./sdrReceive.x -fc 602e6 -f 602.308400e6 -usb -timeout 10 -file test01.raw -device 1 -PPM -0.380
./sdrReceive.x -fc 602e6 -f 602.308400e6 -usb -timeout 5 -file test01.raw -device 0
/opt/local/bin/sox -t raw -r 48000 -b 16 -c 1 -L -e signed-integer test01.raw -n stat

*/

#define NUM_SOURCES 1

#define NUM_BUFFERS 5

#define NUM_ABUFF 10

#define NUM_DATA_BUFF 10

class cStack;
class cDemod;
class cReceive;
struct playData{
    unsigned char start;
    double real[2*32768*2];
    double imag[2*32768*2];
    double reals[2*32768*2];
    double imags[2*32768*2];

  //  ALCdevice *dev;
  //  ALCcontext *ctx;
  //  ALuint buffers[NUM_BUFFERS];
    int bufferState[NUM_BUFFERS];
  //  ALuint source;
    int channels;
    int samplerate;
	int Debug;
	double gain;
	double fc;
	double f;
	double dt;
	double sino;
	double coso;
	double sindt;
	double cosdt;
	double w;
	double bw;
	int decodemode;

	SoapySDR::Stream *rxStream;
	SoapySDR::Device *device;
	
	class cStack *cs;
	class cDemod *d;
	class cReceive *r;
	
	volatile int doWhat;
	volatile int witch;
	volatile int retFlag;
	
    volatile int audioOut;

    unsigned long MTU;
    
    float faudio;
    
    float faudioCount;
    
    float Ratio;

     
    int size; 
    
    int antennaCount;
    char **antenna;
    char *antennaUse;
    int channel;
    
    int deviceNumber;
    
    volatile int frame;
    

   	volatile int thread; 

 	int al_state;
 	
 	std::string set[20];
 	std::string value[20];
 	int setcount;
 	
 	double timeout;
 	double timestart;
 	int dumpbyminute;
    int idump;
    
    double aminGlobal;
    double amaxGlobal;
    double averageGlobal;
 	
 	double PPM;
 	
 	double rf_gain;
 	
 	int ncut;
 	
 	int audiodevice;
 	
 	double aminGlobal3;
 	double shiftGlobal;
 	int FFTfilter;
 	double cutOFFSearch;
 	double cutOFF;

	double meterMax;
	
	int muteScan;
	
	long FFTcount;
	
	int pipe;
	
	int binary;
	
	long startSound;
	
	float *sweepBuff;
	double sweepLower;
	double sweepUpper;
	double sweepLower2;
	double sweepUpper2;
	double sweepSize;
	double crop;
	int sweepFound;
	int sweep;
	FILE *out;
	char *s2dName;


 	unsigned char end;
 	
 	int muteAudio;
 	
 	double junk22;

	std::mutex mutexo;
	std::mutex mutex1;
	std::mutex mutexa;
	
	std::string DeviceString;

};

struct Filters{
	int np;
    ampmodem demodAM;
	freqdem demod;
	msresamp_crcf iqSampler;
	msresamp_rrrf iqSampler2;
	nco_crcf fShift;
	int thread;	
	double amHistory;
};

#include "cStack.h"

#endif


