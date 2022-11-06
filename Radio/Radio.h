//
//  Radio.h
//  SdrGlut
//
//  Created by Dale on 10/8/16.
//  Copyright © 2016 Dale Ranta. All rights reserved.
//

#ifndef RadioRA_h
#define RadioRA_h
#include "RaceFastsdr.h"
#include "ulibTypes.h"
#include <GL/glui.h>

#include "Poly.h"

#include "audiolib.h"

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <vector>

using namespace std;

//#include <getopt.h>

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sndfile.h>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif

#ifndef __has_include
#error "Compiler missing __has_include macro!"
#endif

#if __has_include(<rtaudio/RtAudio.h>)
#include <rtaudio/RtAudio.h>
#else
#include <RtAudio.h>
#endif

#include <time.h>

#include "Utilities.h"

#include "CLines.h"
#include "GridPlot.h"
#include "paletteDraw.h"
#include "DialogSave.h"


class GLUIAPI GLUI_TextBox3 : public GLUI_TextBox
{
public:
    virtual int mouse_down_handler( int local_x, int local_y );
    int  key_handler( unsigned char key,int modifiers );
    virtual int special_handler( int key, int modifiers);
    GLUI_TextBox3( GLUI_Node *parent, bool scroll = false,
                  int id=-1, GLUI_CB cb=GLUI_CB() );
    
};

struct playData4{
    int samplerate;
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

    int deviceNumber;

    int decodemode;
    
    int size;
    
    int debug;
};


struct WaterFall4{
    unsigned char *data;
    unsigned char ic[2*131072];
    double amin,amax;
    uRect SRect;
    uRect DRect;
    int nline;
    int xsize;
    int ysize;
};

struct DialogSendData{
    GLUI *glui;
    char text1[255];
    GLUI_EditText *edittext1;
    char text2[255];
    GLUI_EditText *edittext2;
    char text3[255];
    GLUI_EditText *edittext3;
    struct Scene *sceneLocal;
    int sub_window;
    GLUI_RadioGroup *group2;
    int datatype;
    int sendmode;
    int demodulationFlag;
    int frequencyFlag;
};

struct DialogSendVoice{
    GLUI *glui;
    char text1[255];
    GLUI_EditText *edittext1;
    char text2[255];
    GLUI_EditText *edittext2;
    char text3[255];
    GLUI_EditText *edittext3;
    struct Scene *sceneLocal;
    int sub_window;
    GLUI_RadioGroup *group2;
    int datatype;
    int sendmode;
    int demodulationFlag;
    int frequencyFlag;
};

struct DialogSendIQ{
    GLUI *glui;
    char text1[255];
    GLUI_EditText *edittext1;
    char text2[255];
    GLUI_EditText *edittext2;
    char text3[255];
    GLUI_EditText *edittext3;
    struct Scene *sceneLocal;
    int sub_window;
    GLUI_RadioGroup *group2;
    int datatype;
    int sendmode;
    int demodulationFlag;
    int frequencyFlag;
    FILE *pipe;
};

struct DialogRadioData{
    GLUI *glui;
    char text1[255];
    char text2[255];
    char text3[255];
    char text4[255];
    char text5[255];
    char text15[255];
    char text16[255];
    char text17[255];
    char text6[255];
    char text7[255];
    char text8[255];
    char text20[255];
    char text21[255];
    char text22[255];

    GLUI_EditText *edittext1;
    GLUI_EditText *edittext2;
    GLUI_EditText *edittext3;
    GLUI_EditText *edittext4;
    GLUI_EditText *edittext5;
    GLUI_EditText *edittext15;
    GLUI_EditText *edittext16;
    GLUI_EditText *edittext17;
    GLUI_EditText *edittext6;
    GLUI_EditText *edittext7;
    GLUI_EditText *edittext8;
    GLUI_EditText *edittext20;
    GLUI_EditText *edittext21;
    GLUI_EditText *edittext22;

    GLUI_Button *search;
    GLUI_Button *scan;

    char text1z[255];
    GLUI_EditText *edittext1z[20];
    GLUI_Scrollbar *line_scroll[20];
    GLUI_EditText *editlowpass;
    GLUI_Scrollbar *lowpass;
    float lowpassvalue;
    float line_Index[20];
    float line_Index_old[20];
    int gain_Index;
    int iic;
    int useagc;
    struct Scene *sceneLocal;
    int sub_window;
};

struct Info{
    int modetype;
    msresamp_rrrf iqSampler;
    class AMmod *am;
    msresamp_crcf iqSampler2;
    SoapySDR::Device *device;
    SoapySDR::Stream *txStream;
    ampmodem demodAM;
    freqmod mod;
    
    volatile int Tone;

    double dt;
    double sino;
    double coso;
    double sindt;
    double cosdt;
    
    int extraBytes;

};




struct TransmitData{
    GLUI *glui;
    char text1[255];
    char text2[255];
    char text3[255];
    char text4[255];
    char text5[255];
    char text15[255];
    char text16[255];
    char text17[255];
    char text6[255];
    char text7[255];
    char text8[255];
    
    GLUI_EditText *edittext1;
    GLUI_EditText *edittext2;
    GLUI_EditText *edittext3;
    GLUI_EditText *edittext4;
    GLUI_EditText *edittext5;
    GLUI_EditText *edittext15;
    GLUI_EditText *edittext16;
    GLUI_EditText *edittext17;
    GLUI_EditText *edittext6;
    GLUI_EditText *edittext7;
    GLUI_EditText *edittext8;

    
    GLUI_Button *talk;

    struct Info info;

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

    GLUI_RadioGroup *group2;
    int modetype;
    
    RtAudio::StreamParameters Params;
    RtAudio *audio;
    
    volatile int doTransmit;
    
    
    volatile double fc;
    double foffset;
    double transmitSampleRate;
    double gain;
    char text1z[255];
    GLUI_EditText *edittext1z[20];
    GLUI_Scrollbar *line_scroll[20];
    float line_Index[20];
    float line_Index_old[20];
    int gain_Index;
    int iic;
    int useagc;
    struct Scene *sceneLocal;
    int sub_window;
};

struct RecordSoundStruct{
    time_t start[5];
    time_t stop[5];
    char FilePath[5][2048];
    int on[5];
    int state[5];
    double frequency[5];
    char mode[5][16];;
};

struct ResetInfo{
    double frequency;
    int decodemode;
    int reset;
};

class CLines;

class Radio;

class Radio: public CWindow{
public:
    Radio(struct Scene *scene,SoapySDR::Kwargs deviceArgs);
    virtual ~Radio();
    
    Radio *findMate(struct playData *rx);
    int playRadio(struct playData *rx);
    int stopPlay(struct playData *rx);
    int startPlay(struct playData *rx);
    int closeScenes();
    int controlScan(struct playData *rx);
    virtual int LoadFile (struct Scene *scene,char *name, int fileType);
    virtual int BackGroundEvents(struct Scene *scene);
    virtual int UpdateTemperatureColors(struct Scene *scene);
    virtual int sendMessage(char *m1,char *m2,int type);
    virtual int SetFrequency(struct Scene *scene,double f,double bw,int messageType);
    int SetWindow(struct Scene *scene);
    int mMenuSelectl(struct Scene *scene,int item);
    int KillScene (struct Scene *scene);
    int dialogRadio (struct Scene *scene);
    //int dialogSend(struct Scene *scene);
    int dialogSendIQ(struct Scene *scene);
    int doVoiceControl(struct Scene *scene);
    int doVoice();
    int doUP();
    int doDown();
    int setFrequency2(struct playData *play);
    int setFrequency3(struct playData *play);
    int setFrequencyDuo(struct playData *play);
    int processScan(struct playData *play);
    int setFrequencyCoefficients(struct playData *rx);
    int stopPlay(struct playData4 *play);
    void playRadio (struct playData4 *play);
    int startRadio (struct playData4 *play);
    int initPlay(struct playData4 *play);
    virtual int FindPoint(struct Scene *scene,int x,int y);
    virtual int OpenWindows(struct Scene *scene);
    void getMouse(int button, int state, int x, int y);
    void adjustView(int button);
    int Display(struct Scene *scene);
    int resetDemod();
    
    int setBandWidth(double bandwidth);
    
    int setSampleRate(double samplerate);
    
    int setRange(double pmin,double pmax);
    
    int setPower(double power);
    
    int setFrequency(double frequency);
    
    int fftIndex(double frequency);
    
    int setFc(double frequency);
    
    int Transmit(struct Scene *scene);
    
    int WriteToWindow(char *message);
    
    int doFrequencyFile(char *path);
    
    int doSoundRecord();
    
    int dialogTime();
    
    int setDialogFrequency(double frequency);
    
    int setDialogFc(double frequency);
    
    int setDialogRange(double pmin,double pmax);
    
    int setDialogPower(double power);
    
    int setDialogBandWidth(double power);
    
    int setDialogSampleRate(double power);
    
    int updateLine(void);
    
    int welch(double *real,double *imag,int *length);
        
    unsigned char start;

    char ApplicationDirectory[2048];
    
    char filename[2048];
    
    volatile int inTransmit;
    
    volatile int inDialog;

    volatile int backGroundEvents;

    struct WaterFall4 water;
    
    struct playData *rx;
    
    
    struct DialogRadioData dd;
    
    struct DialogSendData bb;
    
    struct DialogSendVoice vv;
    
    struct DialogSendIQ qq;

    struct TransmitData tt;
    
    struct ResetInfo reset;

    double real[2*131072*sizeof(double)];
    double imag[2*131072*sizeof(double)];
    long FFTlength;

    CLines *lines;
    struct Scene *scenel;
    CLines *lines2;
    struct Scene *scenel2;
    double *range;
    double *range3;
    double *magnitude;
    double *magnitude2;
    double *magnitude3;
    double *frequencies;
    double *ampitude;
    double lineTime;
    double lineDumpInterval;
    double lineAlpha;
    
    struct paletteDraw pd;
    
    double fcdown;
    
    double fdown;
    
    int fcount;

    uRect box;
    
    double power;
    
    int w,h;
    
    int window1;

    int window2;
    
    int window3;

    int *flagsmenu;
    int flagsflag;
    
    volatile int inuseflag;
    
    
    int scanFound[200];
    double pauseTime;
    double pauseTimeDelta;
    int pauseChannel;
    int scanRun;
    int scanWait;
    
    GLUI_TextBox3 *moo;

    int gluiID;
    
    int mooWindow;
    
    int insert;

    struct RecordSoundStruct rs;
    
    unsigned char end;
    
    volatile int voicecontrol;
    
    volatile int voiceSpectrum;
    
    
    int inAxis;

    int junk11;
    
    vector<double> scanFrequencies;
    
    SoapySDR::ArgInfoList flags;
    
    struct playData rxs;
    
    class Poly *plowpass;


};

typedef Radio *RadioPtr;

extern RadioPtr FindSceneRadio(int window);

extern RadioPtr FindSdrRadioWindow(int window);

extern RadioPtr FindSdrRadioWindow2(int window);

extern RadioPtr FindSdrRadioWindow(struct playData *rx);

/*
extern int testRadio(struct playData3 *rx,SoapySDR::Kwargs deviceArgs);
*/

#endif /* FMRadio_h */
