#ifndef RadioFMCRA_h
#define RadioFMCRA_h

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include <liquid/liquid.h>

#include "audiolib.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <mutex>

#include <sys/timeb.h>


#include <sndfile.h>


#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif

#include <time.h>

#include "mThread.h"

#include "SocketDefs.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6


#define FILTER_RECTANGULAR     0
#define FILTER_HANN            1
#define FILTER_HAMMING         2
#define FILTER_FLATTOP         3
#define FILTER_BLACKMANHARRIS  4
#define FILTER_BLACKMANHARRIS7 5


#define START_AUDiO  107
#define STOP_AUDiO   108
#define START_IQ     109
#define STOP_IQ      110

#define FFT_1024 1024
#define FFT_2048 2048
#define FFT_4096 4096
#define FFT_8192 8192
#define FFT_16384 16384
#define FFT_32768 32768
#define FFT_65536 65536
#define FFT_131072 131072

#define INPUT1 NULL
//#define INPUT1 "Built-in Audio Digital Stereo (IEC958)"
//#define INPUT1 "GF108 High Definition Audio Controller Digital Stereo (HDMI)"
//#define INPUT1 "USB Headset Analog Stereo"

#define    BLOCK_SIZE5 4800

#define NUM_SOURCES5 1

#define NUM_BUFFERS5 8

#define NUM_ABUFF5 10

#define NUM_DATA_BUFF5 10

class saveData{
public:
    int decodemode;
    double samplerate;
    double fc;
    double f;
 
    saveData()
    {
        f=-1;
        fc=-1;
        samplerate=-1;
        decodemode=-1;
    }
    
};

struct Info1{
    int Tone;
    double f;
    double dt;
    double sino;
    double coso;
    double sindt;
    double cosdt;
};

class Radio;

struct playData{
    unsigned char start;
    ALCdevice *dev;
    ALCcontext *ctx;
    double real[2*131072*2];
    double imag[2*131072*2];
    double reals[2*131072*2];
    double imags[2*131072*2];
    int FFTcount;
    int FFTfilter;
    int wShift;
    int size;

    ALuint source;
    double samplerate;
    double bandwidth;
    double meterMax;
    int Debug;
    double gain;
    int nreceive;
    int ntransmit;
    int channel;
    double foffset;
    double fc;
    double fv;
    double f;
    double dt;
    double sino;
    double coso;
    double sindt;
    double cosdt;
    double w;
    double bw;
    
    double samplerate_save;
    double fc_save;

    
    int decodemode;

    SoapySDR::Stream *rxStream;
    SoapySDR::Device *device;
    
    int fOut;
    
    float Ratio;
    
    float *buff[NUM_DATA_BUFF5];
    int buffStack[NUM_DATA_BUFF5];
    volatile int bufftop;
    
    short int *buffa[NUM_ABUFF5];
    int buffStacka[NUM_ABUFF5];
    volatile int bufftopa;
    
    
    unsigned int deviceNumber;
    
    volatile int witchRFBuffer;
    volatile int witchAudioBuffer;
    volatile int threadNumber;
    volatile int controlRF;
    volatile int controlAudio;
    volatile int controlProcess;
    volatile int mute;
    volatile int muteScan;
    volatile int output;

    volatile int controlSend;
    volatile int fillBuffer;
    unsigned short Port;
    float *sendBuff1;
    float *sendBuff2;
    SOCKET send;
    
    char *name;
    
    int al_state;
    
    int dataType;
    int sendMode;
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
    
    int directSampleMode;

    int getRadioAttributes;
    
    SNDFILE *audioOutput;
    volatile FILE *iqOutput;

    char driveName[256];
    
    int (*pstopPlay)(struct playData *rx);
    int (*pstartPlay)(struct playData *rx);
    int (*pplayRadio)(struct playData *rx);
    int (*psdrDone)(struct playData *rx);
    int (*psdrSetMode)(struct playData *rx);
    int (*pSetAudio)(struct playData *rx,char *name,int type);
    int (*pStartSend)(struct playData *rx,char *name,int type,int mode);

    double aminGlobal;
    double amaxGlobal;
    
    double aminGlobal2;
    double amaxGlobal2;
    
    double averageGlobal;
    
    int drops;
    
    double aminGlobal3;
    double amaxGlobal3;
    double shiftGlobal;

    
    double scaleFactor;
    
    double cutOFF;

    volatile int cutOFFSearch;
    
    int audioThreads;
    
    class saveData *save;
    
    int demodulationFlag;
    
    int frequencyFlag;
    
    unsigned char end;
    
    size_t MTU;
    
    struct Info1 info1;
    
    double viewWindow;
    
    double rmin;
    double rmax;
    
    class Radio *mRadio;

    
    int junk22;
    
    SoapySDR::Kwargs deviceToOpen;
    
    std::mutex mutex;
    
    std::mutex mutexa;
    
    std::mutex mutexo;
    
    std::string biasMode;
    
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
SNDFILE *sfopen(char *filename);
#endif
