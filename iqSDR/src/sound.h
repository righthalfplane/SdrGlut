#ifndef __SOUNDH__

#define __SOUNDH__

#include "firstFile.h"

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

#include <sndfile.h>

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

#ifdef __APPLE__
# include <OpenAL/al.h>
# include <OpenAL/alc.h>
#else
# include <AL/al.h>
# include <AL/alc.h>
//# include <AL/alext.h>
#endif


#define NO_MORE_SPACE 99999

struct audioInfo
{
    ALCdevice *dev;
    ALCcontext *ctx;
    
	ALuint *source;
	int *sourceFlag;
	int numsource;
	
	ALuint *buff;
	int *buffFlag;
	int numbuff;
	
};

ALvoid DisplayALError(unsigned char *szText, ALint errorCode);

ALuint getsourceAudio(struct audioInfo *audio);

ALuint getbuffAudio(struct audioInfo *audio);

ALuint freebuffAudio(struct audioInfo *audio,ALuint n);

int freesourceAudio(struct audioInfo *audio,ALuint n);

int startAudio(struct audioInfo *audio,int numsource,int numbuff);
int stopAudio(struct audioInfo *audio);
int updateAudio(struct audioInfo *audio);


struct playData{
    ALCdevice *dev;
    ALCcontext *ctx;
    ALuint source;
    int channels;
};



struct names{
	std::string name;
	std::vector<int> sampleRate;
	int deviceID;
};


class soundClass
{
public:
	soundClass();
	~soundClass();
	int setFrequency(double frequency);
	int setDataSave(int length);
	int setup(int argc, char *argv[]);
	int readSDR();
	int readFile();
	int initPlay();
	int findRadio();
	int rxBuffer();
	int startPlay();
	int playRadio();
	void playSound();
	int stopPlay();
	int Process();
	int doAudio(float *aBuff,float *wBuff);
	int startSound();
	int startRecord(char *name);
	int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );
	int printInfo(void);
	int printAudio(void);
	static int printDevices(void);
	int setBuffers(struct playData *play);

    double samplerate;
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
	int audiodevice;
	
	std::vector<names> inputNames;
	std::vector<names> outputNames;

	float *saveBuff;
	int saveLength;
	
	int inputID;
	int outputID;

    volatile int audioSync;
    
    volatile int audioOut;
    
    volatile int saveFlag;
    
    volatile int saveCall;
    
   	volatile int thread; 
   	
	volatile int doWhat;
	
	volatile int witch;
	
    volatile int iWait;
    
    volatile int frame;
    
	volatile int soundRun;
        
    
    unsigned long MTU;
    
    float faudio;
    
    float Ratio;
    
    int size; 
    
    int antennaCount;
    char **antenna;
    char *antennaUse;
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
    
    double aminGlobal;
    double amaxGlobal;
 	
 	double PPM;
 	
 	double rf_gain;
 	
 	int ncut;
 	
 	SNDFILE *outfile;
 	
 	std::vector<double> rate;
 	
 	cStack *bS;
 	
 	struct audioInfo *audio;
 	
 	struct playData play;

 	 	
};

#endif
