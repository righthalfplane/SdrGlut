//
//  Radio.h
//  SdrGlut
//
//  Created by Dale on 10/8/16.
//  Copyright © 2016 Dale Ranta. All rights reserved.
//

#ifndef SdrFileRA_h
#define SdrFileRA_h
#include "ulibTypes.h"
#include <GL/glui.h>
#include <cstdlib>
#include <cstddef>
#include <iostream>
//#include <getopt.h>

#include <liquid/liquid.h>

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sndfile.h>

#include "audiolib.h"

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif

#include <time.h>

#include "Utilities.h"

#include "CLines.h"
#include "GridPlot.h"
#include "paletteDraw.h"
#include "DialogSave.h"

#define BLOCK_SIZE 32768

#define NUM_SOURCES 1
#define NUM_BUFFERS 5

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

#define FFT_1024 1024
#define FFT_2048 2048
#define FFT_4096 4096
#define FFT_8192 8192
#define FFT_16384 16384
#define FFT_32768 32768


struct playData4{
    ALCdevice *dev;
    ALCcontext *ctx;
    ALuint buffers[NUM_BUFFERS];
    int bufferState[NUM_BUFFERS];
    ALuint source;
    FILE *infile;
    double samplerate;
    double meterMax;
    short int data[2*BLOCK_SIZE];
    float input[2*BLOCK_SIZE];
    float output[2*BLOCK_SIZE];
    //volatile int inDialog;
    double real[2*BLOCK_SIZE];
    double imag[2*BLOCK_SIZE];
    double reals[2*BLOCK_SIZE];
    double imags[2*BLOCK_SIZE];
    int FFTcount;
    int FFTfilter;
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
    volatile int frame;
    volatile long setFrameNumber;
    int gainMode;
    int hasGainMode;
    int decodemode;
    double bw;

    int al_state;
    
    volatile int controlProcess;

    int fOut;
    
    float Ratio;
    
    int size;
    
    double aminGlobal;
    double amaxGlobal;
    double averageGlobal;
    
    double aminGlobal3;
    double amaxGlobal3;

    double scaleFactor;
};


struct WaterFall4{
    unsigned char *data;
    unsigned char ic[2*32768];
    double amin,amax;
    uRect SRect;
    uRect DRect;
    int nline;
    int xsize;
    int ysize;
};

struct Filters2{
    int np;
    ampmodem demodAM;
    freqdem demod;
    msresamp_crcf iqSampler;
    msresamp_rrrf iqSampler2;
    iirfilt_crcf dcFilter;
    iirfilt_crcf lowpass;
    int thread;
    float *buf1;
    float *buf2;
    double amHistory;
    int short *data;
};

struct DialogSdrData{
    GLUI *glui;
    char text1[255];
    char text2[255];
    char text3[255];
    char text4[255];
    char text5[255];
    char text15[255];
    char text6[255];
    char text7[255];
    char text8[255];
    char text16[255];

    GLUI_EditText *edittext1;
    GLUI_EditText *edittext2;
    GLUI_EditText *edittext3;
    GLUI_EditText *edittext4;
    GLUI_EditText *edittext5;
    GLUI_EditText *edittext15;
    GLUI_EditText *edittext6;
    GLUI_EditText *edittext7;
    GLUI_EditText *edittext8;
    GLUI_EditText *edittext16;
    char text1z[255];
    GLUI_EditText *edittext1z[20];
    
    int iic;
    int useagc;
    struct Scene *sceneLocal;
    GLUI_Checkbox *check_box;
    
    int sub_window;
};

struct Info{
    int Tone;
    double f;
    double dt;
    double sino;
    double coso;
    double sindt;
    double cosdt;
};


class CLines;

class SdrFile;

class SdrFile: public CWindow{
public:
    SdrFile(struct Scene *scene);
    virtual ~SdrFile();
    virtual int LoadFile (struct Scene *scene,char *name, int fileType);
    virtual int BackGroundEvents(struct Scene *scene);
    virtual int UpdateTemperatureColors(struct Scene *scene);
    virtual int SetFrequency(struct Scene *scene,double f,double bw,int messageType);
    int SetWindow(struct Scene *scene);
    int mMenuSelectl(struct Scene *scene,int item);
    int KillScene (struct Scene *scene);
    int dialogSdrFile (struct Scene *scene);
    int setFrequency(struct playData4 *play);
    int stopPlay(struct playData4 *play);
    void playFile (struct playData4 *play);
    int startRadio (struct playData4 *play);
    int initPlay(struct playData4 *play);
    virtual int FindPoint(struct Scene *scene,int x,int y);
    virtual int OpenWindows(struct Scene *scene);
    void getMouse(int button, int state, int x, int y);
    int Display(struct Scene *scene);
    
    int setDialogFrequency(double frequency);
    
    int setDialogFc(double frequency);
    
    int setDialogRange(double pmin,double pmax);
    
    int setDialogPower(double power);
    
    int resetDemod();
    
    int updateLine(void);
    
    int fftIndex(double frequency);

    int setBuffers(struct playData4 *play);
    
    int StartIt(struct playData4 *play);
    
    int setInfo();

    unsigned char start;
    
    char ApplicationDirectory[2048];
    char filename[2048];

    volatile int backGroundEvents;
    
    volatile int inDialog;

    struct WaterFall4 water;
    
    struct playData4 play;
    
    struct paletteDraw pd;

    double real[2*32768*sizeof(double)];
    double imag[2*32768*sizeof(double)];
    CLines *lines;
    struct Scene *scenel;
    CLines *lines2;
    struct Scene *scenel2;
    double *range;
    double *dose;
    double *lreal;
    double *limag;
    long FFTlength;
    double lineTime;
    double lineDumpInterval;
    double lineAlpha;
    

    double fcdown;
    
    double fdown;
    
    int fcount;

    uRect box;
    
    double power;
    
    struct DialogSdrData dd;
    
    struct Filters2 filter;
    
    struct Info info;
    
    int w,h;
    
    int window1;
    
    int window2;
    
    volatile int mute;
    
    unsigned char end;
    
    int junk11;

};

typedef SdrFile *SdrFilePtr;

extern SdrFilePtr FindSceneSdrFile(int window);

extern SdrFilePtr FindSdrFileWindow(int window);


#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif


#endif /* FMRadio_h */
