//
//  Transmit.cpp
//  SdrGlut
//
//  Created by Dale on 2/7/20.
//  Copyright © 2020 Dale Ranta. All rights reserved.
//
#include "firstFile.h"
#include "Transmit.h"
#include <cstdio>
#include <cstdlib>

#include "Radio.h"

#include "DialogRangeData.h"
#include "LoadFiles.h"
#include "Utilities.h"
#include "SceneList.h"

#include "CLines.h"

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>


#define Apply_Button   8

#define Start_Button 150
#define Stop_Button 160
#define Key_Button 170



static GLUI_Checkbox *check_box;
static void control_cb(int control);

int ReDrawScene(struct Scene *scene);

RadioPtr FindTransmit(int window);

extern "C" int doFFT2(double *x,double *y,long length,int direction);

// static int doWindow(double *x,double *y,long length,int type);

static int TransmitThread(void *rxv);

int Radio::setBandWidth(double bandwidth)
{
    char value[256];
    
    if(!tt.edittext15 || !inTransmit)return 0;
    
    msprintf(value,sizeof(value),"%g",bandwidth);
    
    tt.edittext15->set_text(value);
    
    return 0;
}

int Radio::setSampleRate(double samplerate)
{
    char value[256];
    
    if(!tt.edittext5 || !inTransmit)return 0;
    
    msprintf(value,sizeof(value),"%g",samplerate);
    
    tt.edittext5->set_text(value);
    
    return 0;
}

int Radio::setRange(double pmin,double pmax)
{
    char value[256];
    
    // fprintf(stderr," use %ld pmin %g pmax %g\n",(long)pd.UsePlotScales,pmin,pmax);
    
    if(pd.UsePlotScales)return 0;
    
    pd.sPmin=pmin;
    
    pd.sPmax=pmax;
    
    
    if(!tt.edittext7 || !tt.edittext8 || !inTransmit)return 0;
    
    msprintf(value,sizeof(value),"%g",pmin);
    
    tt.edittext7->set_text(value);
    
    msprintf(value,sizeof(value),"%g",pmax);
    
    tt.edittext8->set_text(value);
    
    return 0;
}

int Radio::setPower(double power)
{
    char value[256];
    
    if(!tt.edittext1 || !inTransmit)return 0;
    
    msprintf(value,sizeof(value),"%.0f",power);
    
    tt.edittext6->set_text(value);
    
    return 0;
}
int Radio::setFrequency(double frequency)
{
    char value[256];
    
    if(!tt.edittext1 || !inTransmit)return 0;
    
    msprintf(value,sizeof(value),"%g",frequency);
    
    tt.edittext1->set_text(value);
    
    return 0;
}
int Radio::setFc(double frequency)
{
    char value[256];
    
    if(!tt.edittext2 || !inTransmit)return 0;
    
    msprintf(value,sizeof(value),"%g",frequency);
    
    tt.edittext2->set_text(value);
    
    return 0;
}
int Radio::Transmit(struct Scene *scene)
{
    if(!scene)return 1;
    
    if(tt.glui){
        tt.glui->close();
    }
    
    tt.doTransmit=0;
    
    tt.antenna=SoapySDRDevice_listAntennas(rx->device, SOAPY_SDR_TX, 0, &tt.antennaCount);
    
    for (size_t j = 0; j < tt.antennaCount; j++)
    {
        printf("Antenna %lu %s\n",j, tt.antenna[j]);
    }

    tt.gains=SoapySDRDevice_listGains(rx->device, SOAPY_SDR_TX, 0, &tt.gainsCount);
    
    tt.gainsMinimum=(double *)cMalloc((unsigned long)(tt.gainsCount*sizeof(double)),8891);
    
    tt.gainsMaximum=(double *)cMalloc((unsigned long)(tt.gainsCount*sizeof(double)),8892);
    
    for (size_t j = 0; j < tt.gainsCount; j++)
    {
         printf("Gains %lu %s ",j, tt.gains[j]);
        
        SoapySDRRange range3=SoapySDRDevice_getGainElementRange(rx->device, SOAPY_SDR_TX, 0, tt.gains[j]);
        //double el=SoapySDRDevice_getGainElement(rx->device, SOAPY_SDR_TX, 0, rx->gains[j]);
        printf("range max %g min %g\n",range3.maximum,range3.minimum);
        tt.gainsMinimum[j]=range3.minimum;
        tt.gainsMaximum[j]=range3.maximum;
    }
    
    SoapySDRRange range=SoapySDRDevice_getGainRange(rx->device, SOAPY_SDR_TX, 0);
    
    printf("range TX max %g min %g\n",range.maximum,range.minimum);
    
    tt.gainsMin=range.minimum;
    tt.gainsMax=range.maximum;

    tt.hasGainMode=SoapySDRDevice_hasGainMode(rx->device, SOAPY_SDR_TX, 0);
    
    printf("hasGainMode %d\n",rx->hasGainMode);
    
    if(tt.hasGainMode){
        tt.gainMode=SoapySDRDevice_getGainMode(rx->device, SOAPY_SDR_TX, 0);
        printf("GainMode %d\n",tt.gainMode);
    }
    
    SoapySDRRange *rlist=SoapySDRDevice_getFrequencyRange(rx->device, SOAPY_SDR_TX, 0, &tt.frequencyCount);
    tt.frequencyMinimum=(double *)cMalloc((unsigned long)(tt.frequencyCount*sizeof(double)),8894);
    tt.frequencyMaximum=(double *)cMalloc((unsigned long)(tt.frequencyCount*sizeof(double)),8895);
    
    for (size_t j = 0; j < tt.frequencyCount; j++)
    {
        printf("FrequencyRange max %g min %g\n",rlist[j].maximum,rlist[j].minimum);
        tt.frequencyMinimum[j]=rlist[j].minimum;
        tt.frequencyMaximum[j]=rlist[j].maximum;
    }

    
    tt.hasDCOffset=SoapySDRDevice_hasDCOffset(rx->device, SOAPY_SDR_TX, 0);
    
    printf("hasDCOffset %d\n",tt.hasDCOffset);
    
    if(tt.hasDCOffset){
        tt.DCOffset=SoapySDRDevice_getDCOffsetMode(rx->device, SOAPY_SDR_TX, 0);
        printf("DCOffset %d\n",tt.DCOffset);
    }
    
    double *band=SoapySDRDevice_listBandwidths(rx->device, SOAPY_SDR_TX, 0, &tt.bandwidthsCount);
    tt.bandwidths=(double *)cMalloc((unsigned long)(tt.bandwidthsCount*sizeof(double)),8896);
    for (size_t j = 0; j < tt.bandwidthsCount; j++)
    {
        printf("bandwidth %lu max %g\n",j,band[j]);
        tt.bandwidths[j]=band[j];
    }

    char **list=SoapySDRDevice_getStreamFormats(rx->device, SOAPY_SDR_TX, 0, &tt.streamFormatCount);
    tt.streamFormat=(char **)cMalloc((unsigned long)(tt.streamFormatCount*sizeof(double)),8898);
    for (size_t j = 0; j < tt.streamFormatCount; j++)
    {
        printf("StreamFormats %lu %s\n",j, list[j]);
        tt.streamFormat[j]=list[j];
    }

    double *rate=SoapySDRDevice_listSampleRates(rx->device, SOAPY_SDR_TX, 0, &tt.sampleRatesCount);
    tt.sampleRates=(double *)cMalloc((unsigned long)(tt.sampleRatesCount*sizeof(double)),8890);
    for (size_t j = 0; j < rx->sampleRatesCount; j++)
    {
        printf("SampleRates %lu max %g\n",j,rate[j]);
        tt.sampleRates[j]=rate[j];
    }

    size_t length;
    rlist=SoapySDRDevice_getSampleRateRange(rx->device, SOAPY_SDR_TX, 0, &length);
    for (size_t j = 0; j < length; j++)
    {
        printf("SampleRateRange max %g min %g\n",rlist[j].maximum,rlist[j].minimum);
    }

    tt.sceneLocal=scene;
    
    inTransmit=1;
    
    msprintf(tt.text1,sizeof(tt.text1),"%g",rx->f);
    msprintf(tt.text2,sizeof(tt.text2),"%g",rx->fc);
    msprintf(tt.text3,sizeof(tt.text3),"%g",rx->gain);
    msprintf(tt.text4,sizeof(tt.text4),"%g",lineAlpha);
    msprintf(tt.text5,sizeof(tt.text5),"%g",rx->samplerate);
    msprintf(tt.text15,sizeof(tt.text15),"%g",rx->bandwidth);
    msprintf(tt.text16,sizeof(tt.text16),"%g",rx->scaleFactor);
    msprintf(tt.text17,sizeof(tt.text17),"%d",rx->audioThreads);
    msprintf(tt.text6,sizeof(tt.text6),"%.0f",power);
    msprintf(tt.text7,sizeof(tt.text7),"%g",pd.sPmin);
    msprintf(tt.text8,sizeof(tt.text8),"%g",pd.sPmax);
    
    tt.glui = GLUI_Master.create_glui(rx->driveName);
    
    GLUI_Panel *obj_panel =  tt.glui->add_panel( "Mode" );
    
    tt.group2 =
    tt.glui->add_radiogroup_to_panel(obj_panel,&tt.scaletype,5,control_cb);
    
    tt.glui->add_radiobutton_to_group( tt.group2, "AM" );
    tt.glui->add_radiobutton_to_group( tt.group2, "NBFM" );
    tt.glui->add_radiobutton_to_group( tt.group2, "USB" );
    tt.glui->add_radiobutton_to_group( tt.group2, "LSB" );
    tt.glui->add_radiobutton_to_group( tt.group2, "CW" );

    tt.glui->add_column(true);

    tt.edittext2 =
    tt.glui->add_edittext( "Center Frequency:", GLUI_EDITTEXT_TEXT, tt.text2 );
    tt.edittext2->w=200;

    
    tt.edittext1 =
    tt.glui->add_edittext( "Offset Frequency:", GLUI_EDITTEXT_TEXT, tt.text1 );
    tt.edittext1->w=200;
    

    new GLUI_Button(tt.glui, "Start", Start_Button, control_cb);
    
    new GLUI_Button(tt.glui, "Stop", Stop_Button, control_cb);
    
    new GLUI_Button(tt.glui, "Key", Key_Button, control_cb);
    
    new GLUI_Button(tt.glui, "Close", 2, control_cb);
    
    tt.glui->add_column(true);

    obj_panel =  tt.glui->add_panel( "Audio Source" );
    
    tt.audio = 0;
    
    // Default RtAudio constructor
    try {
        tt.audio = new RtAudio();
    }
    catch (RtAudioError &error) {
        error.printMessage();
        exit(1);
    }
    
    if(tt.audio == NULL){
        fprintf(stderr,"audio %p\n",audio);
        return 0;
    }
    
    // Determine the number of devices available
    int devices = tt.audio->getDeviceCount();
    std::cout << "\nFound " << devices << " device(s) ...\n";
    
    // Scan through devices for various capabilities
    RtAudio::DeviceInfo info;
    for (int i=0; i<devices; i++) {
        
        try {
            info = tt.audio->getDeviceInfo(i);
            if(info.inputChannels > 0){
            // Print, for example, the maximum number of output channels for each device
            std::cout << "device = " << i;
            std::cout << ": maximum input  channels = " << info.inputChannels;
            std::cout << " Device Name = " << info.name << '\n';
                new GLUI_Button(obj_panel, info.name.c_str(), 200+i, control_cb);
            }

        }
        catch (RtAudioError &error) {
            error.printMessage();
            break;
        }
        
    }
    
    tt.Params.deviceId = tt.audio->getDefaultInputDevice();
    tt.Params.nChannels = 1;
    tt.Params.firstChannel = 0;

    
    tt.glui->add_column(true);

    
    // Clean up

    
    tt.edittext3 =
    tt.glui->add_edittext( "Gain:", GLUI_EDITTEXT_TEXT, tt.text3);
    tt.edittext3->w=200;
    
    tt.edittext4 =
    tt.glui->add_edittext( "Alpha:", GLUI_EDITTEXT_TEXT, tt.text4);
    tt.edittext4->w=200;
    
    tt.edittext5 =
    tt.glui->add_edittext( "Sample Rate:", GLUI_EDITTEXT_TEXT, tt.text5);
    tt.edittext5->w=200;
    
    tt.edittext6 =
    tt.glui->add_edittext( "Power:", GLUI_EDITTEXT_TEXT, tt.text6);
    tt.edittext6->w=200;
    
    new GLUI_Checkbox( tt.glui, "Set Scale Power", &pd.UsePlotScales, 3, control_cb );
    
    tt.edittext7 =
    tt.glui->add_edittext( "Power Min", GLUI_EDITTEXT_TEXT, tt.text7);
    tt.edittext7->w=200;
    
    tt.edittext8 =
    tt.glui->add_edittext( "Power Max:", GLUI_EDITTEXT_TEXT, tt.text8);
    tt.edittext8->w=200;
    
    new GLUI_Button(tt.glui, "Set Frequency", 5, control_cb);
    
    new GLUI_Button(tt.glui, "Set Gain", 4, control_cb);
    
    new GLUI_Button(tt.glui, "Apply", Apply_Button, control_cb);
    
    new GLUI_Button(tt.glui, "Close", 2, control_cb);
    
    tt.glui->add_column(true);
    
    
    tt.edittext15 =
    tt.glui->add_edittext( "Bandwidth:", GLUI_EDITTEXT_TEXT, tt.text15);
    tt.edittext15->w=200;
    
    tt.edittext16 =
    tt.glui->add_edittext( "Scale Factor:", GLUI_EDITTEXT_TEXT, tt.text16);
    tt.edittext16->w=200;
    
    tt.edittext17 =
    tt.glui->add_edittext( "Auto Threads:", GLUI_EDITTEXT_TEXT, tt.text17);
    tt.edittext17->w=200;
    
    
    
    tt.useagc=rx->gainMode;
    
    check_box = new GLUI_Checkbox( tt.glui, "Use Automatic Agc", &tt.useagc, 7, control_cb );
    
    if(!rx->hasGainMode)check_box->disable();
    
    tt.iic=0;
    
    for(size_t i=0;i<tt.gainsCount;++i){
        GLUI_Panel *panel3 = new GLUI_Panel(tt.glui, tt.gains[tt.iic]);
        double el = SoapySDRDevice_getGainElement(rx->device, SOAPY_SDR_TX, 0, tt.gains[tt.iic]);
        msprintf(tt.text1z,sizeof(tt.text1z),"%.0f",el);
        tt.edittext1z[tt.iic] =
        tt.glui->add_edittext_to_panel(panel3, "", GLUI_EDITTEXT_TEXT, tt.text1z );
        tt.edittext1z[tt.iic]->w=160;
        
        //fprintf(stderr,"el %g\n",el);
        
        tt.line_Index[tt.iic]=el;
        
        tt.line_scroll[tt.iic] =
        new GLUI_Scrollbar( panel3, rx->gains[tt.iic], GLUI_SCROLL_HORIZONTAL,
                           &tt.line_Index[tt.iic], 23+tt.iic,
                           control_cb );
        
        tt.line_scroll[tt.iic]->set_float_limits( rx->gainsMinimum[tt.iic], rx->gainsMaximum[tt.iic] );
        
        //printf("%s min %g max %g\n", rx->gains[tt.iic],rx->gainsMinimum[tt.iic],rx->gainsMaximum[tt.iic]);
        
        //new GLUI_Button(panel3, "Apply", 30, control_cb);
        ++tt.iic;
        
    }
    
    tt.gain_Index=tt.iic;
    
    GLUI_Panel *panel3 = new GLUI_Panel(tt.glui, "gain");
    double el = SoapySDRDevice_getGain(rx->device, SOAPY_SDR_TX, 0);
    msprintf(tt.text1z,sizeof(tt.text1z),"%.0f",el);
    tt.edittext1z[tt.iic] =
    tt.glui->add_edittext_to_panel(panel3, "", GLUI_EDITTEXT_TEXT, tt.text1z );
    tt.edittext1z[tt.iic]->w=160;
    
    tt.line_Index[tt.iic]=el;
    
    //fprintf(stderr,"el %g\n",el);
    
    tt.line_scroll[tt.iic] =
    new GLUI_Scrollbar( panel3, "gain", GLUI_SCROLL_HORIZONTAL,
                       &tt.line_Index[tt.iic], 50,
                       control_cb );
    
    tt.line_scroll[tt.iic]->set_float_limits( rx->gainsMin, rx->gainsMax );
    
    ++tt.iic;
    
    //printf("%s min %g max %g\n", "gain",rx->gainsMin,rx->gainsMax);
    
    tt.sub_window=glutGetWindow();
    
    tt.glui->set_main_gfx_window(tt.sub_window);
    
    return 0;
}
static void control_cb(int control)
{
    std::string file_name;
    double f,fc,gain,lineAlpha,scaleFactor;
    double dmin,dmax;
    double sameleRate;
    double bandwidth;
    int audioThreads;
    
    RadioPtr s=(RadioPtr)FindTransmit(glutGetWindow());
    if(!s)return;
    
    sscanf(s->tt.edittext1->get_text(),"%lg", &f);
    sscanf(s->tt.edittext2->get_text(),"%lg", &fc);
    sscanf(s->tt.edittext3->get_text(),"%lg", &gain);
    sscanf(s->tt.edittext4->get_text(),"%lg", &lineAlpha);
    sscanf(s->tt.edittext5->get_text(),"%lg", &sameleRate);
    sscanf(s->tt.edittext15->get_text(),"%lg", &bandwidth);
    sscanf(s->tt.edittext16->get_text(),"%lg", &scaleFactor);
    sscanf(s->tt.edittext17->get_text(),"%d", &audioThreads);
    sscanf(s->tt.edittext7->get_text(),"%lg", &dmin);
    sscanf(s->tt.edittext8->get_text(),"%lg", &dmax);
    
    if(control == 4){
        s->rx->gain=gain;
        s->rx->scaleFactor=scaleFactor;
        if(s->pd.UsePlotScales)
        {
            s->pd.sPmin=dmin;
            s->pd.sPmax=dmax;
        }
    }
    else if(control == 2)
    {
        s->inTransmit=0;
        s->tt.glui->close();
        s->tt.glui=NULL;
        delete s->tt.audio;

    }
    else if(control == Start_Button)
    {
        fprintf(stderr,"Start_Button %d\n",control);
        if(s->tt.doTransmit == 0){
            s->rx->pstopPlay(s->rx);
            s->tt.doTransmit=1;
            launchThread((void *)s,TransmitThread);
        }
    }
    else if(control == Stop_Button)
    {
        fprintf(stderr,"Stop_Button %d\n",control);
        if(s->tt.doTransmit == 1){
            s->tt.doTransmit=0;
            s->tt.info.loop=0;
            Sleep2(100);
            fprintf(stderr,"Stop_Button Sleep\n");
            s->rx->pstartPlay(s->rx);
            s->rx->pplayRadio(s->rx);
        }
   }
    else if(control == Key_Button)
    {
        fprintf(stderr,"Key_Button %d\n",control);
    }
    else if(control >= 200 && control < 220)
    {
        fprintf(stderr,"Audio Source %d\n",control-200);
        s->tt.Params.deviceId = s->tt.audio->getDefaultInputDevice();
        s->tt.Params.deviceId = control-200;
        s->tt.Params.nChannels = 1;
        s->tt.Params.firstChannel = 0;
    }
    else if(control == 5)
    {
        s->rx->fc=fc;
        s->rx->f=f;
        s->setFrequency(s->rx);
    }
    else if(control == 7)
    {
        if(s->tt.hasGainMode){
            bool automatic=s->tt.useagc;
            s->rx->gainMode=s->tt.useagc;
            int ret=SoapySDRDevice_setGainMode(s->rx->device, SOAPY_SDR_TX, 0, automatic);
            if(ret)printf("useagc %d ret %d\n",s->tt.useagc,ret);
        }
    }
    else if(control == 23  || control == 24  || control == 25  || control == 26)
    {
        char value[256];
        int ind=control-23;
        msprintf(value,sizeof(value),"%0.f",s->tt.line_Index[ind]);
        s->tt.edittext1z[ind]->set_text(value);
        if(s->tt.line_Index_old[ind] != (int)s->tt.line_Index[ind]){
            int ret=SoapySDRDevice_setGainElement(s->rx->device, SOAPY_SDR_TX, 0, s->tt.gains[ind], s->tt.line_Index[ind]);
            if(ret)printf("SoapySDRDevice_setGainElement ret %d\n",ret);
            s->tt.line_Index[ind]=(int)s->tt.line_Index_old[ind];
        }
    }
    else if(control == 50)
    {
        char value[256];
        int ind=s->tt.gain_Index;
        msprintf(value,sizeof(value),"%0.f",s->tt.line_Index[ind]);
        s->tt.edittext1z[ind]->set_text(value);
        if(s->tt.line_Index_old[ind] != (int)s->tt.line_Index[ind]){
            int ret=SoapySDRDevice_setGain(s->rx->device, SOAPY_SDR_TX, 0, s->tt.line_Index[ind]);
            if(ret)printf("SoapySDRDevice_setGain ret %d\n",ret);
            s->tt.line_Index_old[ind]=(int)s->tt.line_Index[ind];
            // double el = SoapySDRDevice_getGain(s->rx->device, SOAPY_SDR_TX, 0);
            // printf("value %s value %g el %g\n",value,s->tt.line_Index[ind],el);
            
        }
    }
    else if(control == Apply_Button)
    {
        s->rx->pstopPlay(s->rx);
        
        s->rx->scaleFactor=scaleFactor;
        s->rx->gain=gain;
        s->rx->fc=fc;
        s->rx->f=f;
        s->lineAlpha=lineAlpha;
        s->rx->samplerate=sameleRate;
        s->rx->bandwidth=bandwidth;
        s->rx->audioThreads=audioThreads;
        
        if(s->pd.UsePlotScales)
        {
            s->pd.sPmin=dmin;
            s->pd.sPmax=dmax;
        }
        
        for(int n=0;n<s->FFTlength;++n){
            s->lreal[n]=0;
            s->limag[n]=0;
        }
        
        for(int y=0;y<s->water.ysize*2;++y){
            int ns=3*s->water.xsize*y;
            for(int n=0;n<s->water.xsize;++n){
                s->water.data[ns+3*n]=255;
                s->water.data[ns+3*n+1]=255;
                s->water.data[ns+3*n+2]=255;
            }
        }
        
        s->rx->pstartPlay(s->rx);
        
        s->rx->pplayRadio(s->rx);
        
        s->water.nline=0;
        
    }
    
    glutPostRedisplay();
}

class AMmod{
public:
    AMmod(float mui){
        sample_rate_save=0;
        mu=mui;
        alpha=0.1f;
        amin=0;
        amax=0;
    };
    virtual ~AMmod(){
        ;
    };
    int modulate(float *in,int sample_rate,float *out){
        float dmin = 1e33;
        float dmax =-1e33;
        for(int i=0;i<sample_rate;++i){
            float v=in[i];
            if(v < dmin)dmin=v;
            if(v > dmax)dmax=v;
        }
        
        if(amin == 0.0)amin=dmin;
        amin = (1-alpha)*amin+alpha*dmin;
        
        if(amax == 0.0)amax=dmax;
        amax = (1-alpha)*amax+alpha*dmax;
        
        //    fprintf(stderr,"dmin %g dmax %g amin %g amax %g \n",dmin,dmax,amin,amax);
        
        
        if(sample_rate != sample_rate_save){
            double pi;
            pi=4.0*atan(1.0);
            dt=1.0/(double)sample_rate;
            sino=0;
            coso=1;
            double w=2.0*pi*(sample_rate);
            sindt=sin(w*dt);
            cosdt=cos(w*dt);
            sample_rate_save=sample_rate;
        }
        
        double sint,cost;
        
        dmin=amin;
        float average=dmax=amax;
        if(fabs(dmin) > average)average=fabs(dmin);
        
        float over=0.1/average;
        
        for(int i=0;i<sample_rate;++i){
            float v=over*(2.0f*average+mu*in[i]);
            
            sint=sino*cosdt+coso*sindt;
            cost=coso*cosdt-sino*sindt;
            coso=cost;
            sino=sint;
            out[2*i]=v*coso;
            out[2*i+1]=v*sino;
        }
        
        double r=sqrt(coso*coso+sino*sino);
        coso /= r;
        sino /= r;
        
        return 0;
    };
    float sample_rate_save;
    float alpha;
    float amin;
    float amax;
    float mu;
    
    float sino;
    float coso;
    float sindt;
    float cosdt;
    float dt;
};



using namespace std;

#define    BLOCK_SIZE2 200000

#define    BLOCK_SIZE 2048

float buf [BLOCK_SIZE2],r[2*BLOCK_SIZE2];
float buf2 [BLOCK_SIZE2],r2[2*BLOCK_SIZE2];

int SendData(struct Info *data,unsigned int frames,short *buf);

int input( void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
          double /*streamTime*/, RtAudioStreamStatus /*status*/, void *datain );
static int TransmitThread(void *rxv)
{
    
    RadioPtr s=(RadioPtr)rxv;
    struct playData *rx=s->rx;
    
    fprintf(stderr,"Transmit Thread Start\n");
    
    size_t length;

    SoapySDRKwargs *results = SoapySDRDevice_enumerate(NULL, &length);
    if(length < 1)return 1;
    rx->device = NULL;
    
    SoapySDRKwargs deviceArgs;
    
    size_t driver=0;
    for(unsigned int k=0;k<length;++k){
        
        if(k == rx->deviceNumber){
            deviceArgs = results[k];
            
            printf("Found device #%d:",(int)k);
            for (size_t j = 0; j < deviceArgs.size; j++)
            {
                printf("%s=%s, ", deviceArgs.keys[j], deviceArgs.vals[j]);
                if(!strcmp("driver",deviceArgs.keys[j])){
                    driver=j;
                }
            }
            printf("\n");

            mstrncpy(rx->driveName,(char *)deviceArgs.vals[driver],sizeof(rx->driveName));
            
            rx->device = SoapySDRDevice_make(&deviceArgs);

            break;
        }
        
    }
    
    const double frequency = 27.315e6;  //center frequency to 500 MHz
    const double sample_rate = 2e6;    //sample rate to 5 MHz
    float As = 60.0f;
        
    std::vector<size_t> channels;
    
    channels = {0};
    
    s->tt.info.device=rx->device;
    
    SoapySDRDevice_setSampleRate(rx->device,SOAPY_SDR_TX, 0, sample_rate);
    
    cout << "Sample rate: " << sample_rate/1e6 << " MHz" << endl;
    
    //Set center frequency
    
    SoapySDRDevice_setFrequency(rx->device,SOAPY_SDR_TX, 0, frequency,NULL);
    
    SoapySDRDevice_setGain(rx->device,SOAPY_SDR_TX, 0, 22.0);
    SoapySDRDevice_setGain(rx->device,SOAPY_SDR_TX, 0, 32.0);
    
    //  device->setAntenna(SOAPY_SDR_TX, 0, BAND1);
    
    //Streaming Setup
    
#if SOAPY_SDR_API_VERSION < 0x00080000
    SoapySDRDevice_setupStream(rx->device, &rx->rxStream, SOAPY_SDR_RX, SOAPY_SDR_CF32, NULL, 0, NULL);
#else
    rx->rxStream=SoapySDRDevice_setupStream(rx->device,SOAPY_SDR_RX, SOAPY_SDR_CF32, NULL,0,NULL);
#endif

    //fprintf(stderr,"txStream %p\n",txStream);
    
    s->tt.info.txStream=rx->rxStream;
    
    size_t MTU=SoapySDRDevice_getStreamMTU(rx->device,rx->rxStream);
    cout << "MTU: " << MTU << endl;
    
/*
    SoapySDRDevice_setHardwareTime(rx->device,0); //clear HW time for easy debugging
*/
    
    int ret4=SoapySDRDevice_activateStream(rx->device,rx->rxStream, 0, 0, 0);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    

    freqmod mod = freqmod_create(0.5);
    
    float Ratio1 = (float)(10000/ (float)44100);
    
    float Ratio2 = (float)(sample_rate/(float)10000);
    
    fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
    
    msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
    
    s->tt.info.iqSampler=iqSampler;
    
    msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
    
    s->tt.info.iqSampler2=iqSampler2;
    
    //ampmodem demodAM = ampmodem_create(0.5, 0.0,LIQUID_AMPMODEM_DSB, 0);
    ampmodem demodAM = ampmodem_create(0.5,LIQUID_AMPMODEM_DSB, 0);
    s->tt.info.demodAM=demodAM;
    
    AMmod modulation(0.5);
    
    s->tt.info.am=&modulation;
    
    s->tt.info.loop=1;
    
    std::vector<void *> buffs(2);
    
    unsigned int bufferFrames = 512;
    
    //int fs=44100;
    int fs=48000;

    try {
        s->tt.audio->openStream( NULL, &s->tt.Params, RTAUDIO_SINT16, fs, &bufferFrames, &input, (void *)&s->tt.info );
    }
    catch ( RtAudioError& e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        goto cleanup;
    }
    
    try {
        s->tt.audio->startStream();
    }
    catch ( RtAudioError& e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        goto cleanup;
    }

    while(s->tt.doTransmit == 1){
        while ( s->tt.audio->isStreamRunning() ) {
            Sleep2(100);
        }
    }
cleanup:
    s->tt.info.loop=0;
    Sleep2(100);
    
    if (  s->tt.audio->isStreamOpen() )  s->tt.audio->closeStream();
    
    
    freqmod_destroy(mod);
    
    if(demodAM)ampmodem_destroy(demodAM);

    SoapySDRDevice_deactivateStream(rx->device,rx->rxStream, 0, 100000L);
    
    SoapySDRDevice_closeStream(rx->device,rx->rxStream);
    
    SoapySDRDevice_unmake(rx->device);
    
    rx->device=NULL;
    
    rx->rxStream=NULL;
    
    fprintf(stderr,"Transmit Thread End\n");

    return 0;
}

int input( void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
          double /*streamTime*/, RtAudioStreamStatus /*status*/, void *datain )
{
    struct Info *info=(struct Info *)datain;
    unsigned int frames = nBufferFrames;
    if(!info->loop)return 2;
    
    SendData(info,frames,(short *)inputBuffer);
    /*
     static int count;
     static FILE *in;
     if(!in)in=fopen("rec.raw","wb");
     fwrite(inputBuffer,frames,sizeof(short),in);
     if(++count > 500){
     fclose(in);
     return 2;
     }
     
     */
    return 0;
}

int SendData(struct Info *info,unsigned int frames,short *bufin)
{
    unsigned int num,num2;
    
    for(unsigned int k=0;k<frames;++k){
        buf[k]=bufin[k];
    }
    
    
    /*
     int readcount;
     if ((readcount = sf_readf_float (info->infile, buf, frames)) <= 0){
     sf_seek(info->infile,(sf_count_t)0,SEEK_SET);
     return 1;
     }
     
     for(unsigned int k=0;k<frames;++k){
     buf[k]=buf[k]+0.00002*bufin[k];
     buf[k]=0.00002*bufin[k];
     }
     
     */
    msresamp_rrrf_execute(info->iqSampler, (float *)buf, frames, (float *)buf2, &num);  // decimate
    
    //ampmodem_modulate_block(info->demodAM,buf2, num, (liquid_float_complex *)r2);
    
    info->am->modulate(buf2,num,r2);
    
    msresamp_crcf_execute(info->iqSampler2, (liquid_float_complex *)r2, num, (liquid_float_complex *)buf2, &num2);  // decimate
    
    fprintf(stderr,"num %d num2 %d\n",num,num2);
    
    std::vector<void *> buffs(2);
    
    int flags(0);

    long long timeNs=0;
    long timeoutUs=10000;

    buffs[0] = buf2;
    
    cout << "samples sent: " << num2 << endl;

    unsigned int ret = SoapySDRDevice_writeStream(info->device,info->txStream,  &buffs[0], num2, &flags,timeNs,timeoutUs);
    if (ret != num2)
        cout << "error: samples sent: " << ret << "/" << num2 << endl;
    return 0;
}
EXTERN CWinPtr Root;
RadioPtr FindTransmit(int window)
{
    RadioPtr r;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSdrRadio){
            r=(RadioPtr)w;
            if(r->tt.sub_window == window)return r;
        }
        w=w->CNext;
    }
    
    return NULL;
    
}
