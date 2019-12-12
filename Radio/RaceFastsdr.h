#ifndef RadioFMCRA_h
#define RadioFMCRA_h

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include <pthread.h>

#include <liquid/liquid.h>

#include "smeter.h"

#include "audiolib.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <sys/timeb.h>


//#include <sndfile.h>


#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif

#include <time.h>

#include "mThread.h"

//#include "agc.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define START_AUDiO  6
#define STOP_AUDiO   7
#define START_IQ  8
#define STOP_IQ   9

#define INPUT1 NULL
//#define INPUT1 "Built-in Audio Digital Stereo (IEC958)"
//#define INPUT1 "GF108 High Definition Audio Controller Digital Stereo (HDMI)"
//#define INPUT1 "USB Headset Analog Stereo"

#define    BLOCK_SIZE5 4800

#define NUM_SOURCES5 1

#define NUM_BUFFERS5 8

#define NUM_ABUFF5 10

#define NUM_DATA_BUFF5 10

struct playData{
    ALCdevice *dev;
    ALCcontext *ctx;
    double real[2*4800*2];
    double imag[2*4800*2];
    int count;
    ALuint source;
    int channels;
    double samplerate;
    double bandwidth;
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
    
    SoapySDRStream *rxStream;
    SoapySDRDevice *device;
    
    CSMeter m_SMeter;
/*
    int AgcSlope;
    int AgcThresh;
    int AgcManualGain;
    bool AgcOn;
    bool AgcHangOn;
    int AgcDecay;
*/

    unsigned long MTU;
    
    int fOut;
    
    float Ratio;
    
    float *buff[NUM_DATA_BUFF5];
    int buffStack[NUM_DATA_BUFF5];
    int bufftop;
    
    short int *buffa[NUM_ABUFF5];
    int buffStacka[NUM_ABUFF5];
    int bufftopa;
    
    int size;
        
    unsigned int deviceNumber;
    
    volatile int witchRFBuffer;
    volatile int witchAudioBuffer;
    volatile int threadNumber;
    volatile int controlRF;
    volatile int controlAudio;
    volatile int controlProcess;
    volatile int mute;

    pthread_mutex_t mutex;
    
    pthread_mutex_t mutexa;
    
    pthread_mutex_t mutexo;
    
    
    int al_state;
    
    agc_rrrf agc;
    
    char **antenna;
    size_t antennaCount;
    char **gains;
    size_t gainsCount;
    double *gainsMinimum;
    double *gainsMaximum;
    double gainsMin;
    double gainsMax;

    bool hasGainMode;
    int gainMode;
    
    double *frequencyMinimum;
    double *frequencyMaximum;
    size_t frequencyCount;
    
    bool hasDCOffset;
    int DCOffset;
    
    double *bandwidths;
    size_t bandwidthsCount;

    char **streamFormat;
    size_t streamFormatCount;

    double *sampleRates;
    size_t sampleRatesCount;

    int getRadioAttributes;
    
    FILE *audioOutput;
    volatile FILE *iqOutput;

    char driveName[256];
    
    int (*pstopPlay)(struct playData *rx);
    int (*pstartPlay)(struct playData *rx);
    int (*pplayRadio)(struct playData *rx);
    int (*psdrDone)(struct playData *rx);
    int (*psdrSetMode)(struct playData *rx);
    int (*pSetAudio)(struct playData *rx,char *name,int type);
    
    double aminGlobal;
    double amaxGlobal;
    double averageGlobal;
    
    double scaleFactor;
    
    int audioThreads;

};

struct Filters{
    int np;
    ampmodem demodAM;
    freqdem demod;
    msresamp_crcf iqSampler;
    msresamp_rrrf iqSampler2;
    iirfilt_crcf dcFilter;
    iirfilt_crcf lowpass;
    nco_crcf fShift;
    double amHistory;
    int thread;
};


extern "C" int RadioStart(int argc, char *argv [],struct playData *rx);
extern "C" struct playData *RacePointer();

#endif
