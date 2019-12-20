//
//  Radio.h
//  SdrGlut
//
//  Created by Dale on 10/8/16.
//  Copyright © 2016 Dale Ranta. All rights reserved.
//

#ifndef SdrFileRA_h
#define SdrFileRA_h
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include "ulibTypes.h"
#include <GL/glui.h>
#include "smeter.h"
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

#include "fastfir.h"

#include "fir.h"

#include "agc.h"


#define    BLOCK_SIZE 4096

#define NUM_SOURCES 1
#define NUM_BUFFERS 5

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5


struct playData4{
    CSMeter m_SMeter;
    ALCdevice *dev;
    ALCcontext *ctx;
    ALuint buffers[NUM_BUFFERS];
    int bufferState[NUM_BUFFERS];
    ALuint source;
    FILE *infile;
    int channels;
    double samplerate;
    short int data[2*BLOCK_SIZE];
    float input[2*BLOCK_SIZE];
    float output[2*BLOCK_SIZE];
    //volatile int inDialog;
    double real[2*BLOCK_SIZE];
    double imag[2*BLOCK_SIZE];
    int count;
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
    iirfilt_crcf lowpass;
    nco_crcf fShift;
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

};


struct WaterFall4{
    unsigned char *data;
    unsigned char ic[2*4800];
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
    
    GLUI_EditText *edittext1;
    GLUI_EditText *edittext2;
    GLUI_EditText *edittext3;
    GLUI_EditText *edittext4;
    GLUI_EditText *edittext5;
    GLUI_EditText *edittext15;
    GLUI_EditText *edittext6;
    GLUI_EditText *edittext7;
    GLUI_EditText *edittext8;
    char text1z[255];
    GLUI_EditText *edittext1z[20];
    
    int iic;
    int useagc;
    struct Scene *sceneLocal;
    GLUI_Checkbox *check_box;
    
    int sub_window;
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
    char ApplicationDirectory[2048];
    char filename[2048];
    int resetDemod();

    volatile int backGroundEvents;
    
    volatile int inDialog;

    struct WaterFall4 water;
    
    struct playData4 play;
    
    double real[2*4800*sizeof(double)];
    double imag[2*4800*sizeof(double)];
    int count;

    
    CLines *lines;
    struct Scene *scenel;
    CLines *lines2;
    struct Scene *scenel2;
    double *range;
    double *dose;
    double *lreal;
    double *limag;
    double *lreal2;
    double *limag2;
    long length;
    double lineTime;
    double lineDumpInterval;
    double lineAlpha;
     int updateLine(void);
    
    struct paletteDraw pd;

    int setDialogFrequency(double frequency);
    
    int setDialogFc(double frequency);
    
    int setDialogRange(double pmin,double pmax);
    
    int setDialogPower(double power);
                    
    double fcdown;
    
    double fdown;
    
    int fcount;

    uRect box;
    
    double power;
    
    struct DialogSdrData dd;
    
    
    int setBuffers(struct playData4 *play);
    
    int StartIt(struct playData4 *play);
    
    struct Filters2 filter;
    
    int w,h;
    
    int window1;
    
    int window2;
    
    volatile int mute;

};

typedef SdrFile *SdrFilePtr;

extern SdrFilePtr FindSceneSdrFile(int window);

extern SdrFilePtr FindSdrFileWindow(int window);


#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif


#endif /* FMRadio_h */
