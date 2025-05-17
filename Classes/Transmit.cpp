//
//  Transmit.cpp
//  SdrGlut
//
//  Created by Dale on 2/7/20.
//  Copyright © 2020 Dale Ranta. All rights reserved.
//
#include "firstFile.h"
#include "Utilities.h"
#include "Transmit.h"
#include <cstdio>
#include <cstdlib>

#include "Radio.h"

#include "DialogRangeData.h"
#include "LoadFiles.h"
#include "Utilities.h"
#include "SceneList.h"

#include "CLines.h"

static int TransmitThread(void *rxv);


#define Apply_Button   8

#define Start_Button   150
#define Stop_Button    160
#define Key_Button     170
#define Mode_Buttons   180
#define Mode_AM          0
#define Mode_NBFM        1
#define Mode_USB         2
#define Mode_LSB         3
#define Mode_CW          4

static void control_cb(int control);

RadioPtr FindTransmit(int window);

extern "C" int doFFT2(double *x,double *y,long length,int direction);

// static int doWindow(double *x,double *y,long length,int type);
int freeMemoryTransmit(struct TransmitData *rx);

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
    
    if(!tt.edittext2 || !inTransmit)return 0;
    
   // fprintf(stderr,"frequency %f\n",frequency);
    
    msprintf(value,sizeof(value),"%g",frequency);
    
    tt.edittext2->set_text(value);

    return 0;
}
int Radio::setFc(double frequency)
{
    char value[256];
    
    if(!tt.edittext1 || !inTransmit)return 0;
    
    msprintf(value,sizeof(value),"%g",frequency);
    
    tt.edittext1->set_text(value);
    
    return 0;
}
int Radio::Transmit(struct Scene *scene)
{
    if(!scene)return 1;
    
    if(tt.glui){
        tt.glui->close();
    }
    
    tt.doTransmit=0;
    
    std::vector<std::string> names=rx->device->listAntennas( SOAPY_SDR_TX, rx->channel);
    
    tt.antennaCount=names.size();
    tt.antenna=(char **)cMalloc((unsigned long)(tt.antennaCount*sizeof(char *)),8833);
    for (size_t i=0;i<names.size();++i){
        tt.antenna[i]=strsave((char *)names[i].c_str(),5555);
    }

    names = rx->device->listGains(SOAPY_SDR_TX, rx->channel);
    tt.gainsCount=names.size();
    tt.gains=(char **)cMalloc((unsigned long)(tt.gainsCount*sizeof(char *)),8833);
    tt.gainsMinimum=(double *)cMalloc((unsigned long)(tt.gainsCount*sizeof(double)),8891);
    tt.gainsMaximum=(double *)cMalloc((unsigned long)(tt.gainsCount*sizeof(double)),8892);
    
    for (size_t j = 0; j < tt.gainsCount; j++)
    {
        tt.gains[j]=strsave((char *)names[j].c_str(),5555);
         //printf("Gains %lu %s ",j, tt.gains[j]);
    
        SoapySDR::Range range3=rx->device->getGainRange(SOAPY_SDR_TX, rx->channel, tt.gains[j]);
        tt.gainsMinimum[j]=range3.minimum();
        tt.gainsMaximum[j]=range3.maximum();
    }
    
    SoapySDR::Range range=rx->device->getGainRange(SOAPY_SDR_TX, rx->channel);
    
    //printf("range TX max %g min %g\n",range.maximum(),range.minimum());
    
    tt.gainsMin=range.minimum();
    tt.gainsMax=range.maximum();

    tt.hasGainMode=rx->device->hasGainMode(SOAPY_SDR_TX, rx->channel);
    
    //printf("hasGainMode %d\n",rx->hasGainMode);
    
    if(tt.hasGainMode){
        tt.gainMode=rx->device->getGainMode(SOAPY_SDR_TX, rx->channel);
        //printf("GainMode %d\n",tt.gainMode);
    }
    
    SoapySDR::RangeList rlist=rx->device->getFrequencyRange(SOAPY_SDR_TX, rx->channel);
    tt.frequencyCount=rlist.size();
    tt.frequencyMinimum=(double *)cMalloc((unsigned long)(tt.frequencyCount*sizeof(double)),8894);
    tt.frequencyMaximum=(double *)cMalloc((unsigned long)(tt.frequencyCount*sizeof(double)),8895);
    
    for (size_t j = 0; j < tt.frequencyCount; j++)
    {
        printf("FrequencyRange max %g min %g\n",rlist[j].maximum(),rlist[j].minimum());
        tt.frequencyMinimum[j]=rlist[j].minimum();
        tt.frequencyMaximum[j]=rlist[j].maximum();
    }

    
    tt.hasDCOffset=rx->device->hasDCOffset(SOAPY_SDR_TX, rx->channel);
    
   // printf("hasDCOffset %d\n",tt.hasDCOffset);
    
    if(tt.hasDCOffset){
        tt.DCOffset=rx->device->getDCOffsetMode(SOAPY_SDR_TX, rx->channel);
      //  printf("DCOffset %d\n",tt.DCOffset);
    }
    
    std::vector<double> band=rx->device->listBandwidths(SOAPY_SDR_TX, rx->channel);
    tt.bandwidthsCount=band.size();
    tt.bandwidths=(double *)cMalloc((unsigned long)(tt.bandwidthsCount*sizeof(double)),8896);
    for (size_t j = 0; j < tt.bandwidthsCount; j++)
    {
      //  printf("bandwidth %lu max %g\n",j,band[j]);
        tt.bandwidths[j]=band[j];
    }

    std::vector<std::string> list=rx->device->getStreamFormats(SOAPY_SDR_TX, rx->channel);
    tt.streamFormatCount=list.size();
    tt.streamFormat=(char **)cMalloc((unsigned long)(tt.streamFormatCount*sizeof(double)),8898);
    for (size_t j = 0; j < tt.streamFormatCount; j++)
    {
        //printf("StreamFormats %lu %s\n",j, list[j].c_str());
        tt.streamFormat[j]=strsave((char *)list[j].c_str(),95695);
    }

    std::vector<double> rate=rx->device->listSampleRates(SOAPY_SDR_TX, rx->channel);
    tt.sampleRatesCount=rate.size();
    tt.sampleRates=(double *)cMalloc((unsigned long)(tt.sampleRatesCount*sizeof(double)),8890);
    for (size_t j = 0; j < rx->sampleRatesCount; j++)
    {
        //printf("SampleRates %lu max %g\n",j,rate[j]);
        tt.sampleRates[j]=rate[j];
    }

    size_t length;
    rlist=rx->device->getSampleRateRange(SOAPY_SDR_TX, rx->channel);
    length=rlist.size();
    for (size_t j = 0; j < length; j++)
    {
       // printf("SampleRateRange max %g min %g\n",rlist[j].maximum(),rlist[j].minimum());
    }

    tt.sceneLocal=scene;
    
    inTransmit=1;
    
    tt.foffset=0;
    
    tt.transmitSampleRate=2.0e6;
    
    msprintf(tt.text1,sizeof(tt.text1),"%g",tt.foffset);
    msprintf(tt.text2,sizeof(tt.text2),"%g",rx->f);
    msprintf(tt.text3,sizeof(tt.text3),"%g",tt.transmitSampleRate);

    tt.glui = GLUI_Master.create_glui(rx->driveName);
    
    GLUI_Panel *obj_panel =  tt.glui->add_panel( "Mode" );
    
    tt.group2 =
    tt.glui->add_radiogroup_to_panel(obj_panel,&tt.modetype,Mode_Buttons,control_cb);
    
    tt.glui->add_radiobutton_to_group( tt.group2, "AM" );
    tt.glui->add_radiobutton_to_group( tt.group2, "NBFM" );
    tt.glui->add_radiobutton_to_group( tt.group2, "USB" );
    tt.glui->add_radiobutton_to_group( tt.group2, "LSB" );
  //  tt.glui->add_radiobutton_to_group( tt.group2, "CW" );
    new GLUI_Checkbox( obj_panel, "Tone", (int *)&tt.info.Tone, 3000, control_cb );


    tt.glui->add_column(true);

    obj_panel =  tt.glui->add_panel( "Frequency" );

    tt.edittext2 =
    tt.glui->add_edittext_to_panel(obj_panel,"Center Frequency:", GLUI_EDITTEXT_TEXT, tt.text2 );
    tt.edittext2->w=200;


    tt.edittext1 =
    tt.glui->add_edittext_to_panel(obj_panel, "Offset Frequency:", GLUI_EDITTEXT_TEXT, tt.text1 );
    tt.edittext1->w=200;
    
    tt.edittext3 =
    tt.glui->add_edittext_to_panel(obj_panel, "Transmit Sample Rate:", GLUI_EDITTEXT_TEXT, tt.text3);
    tt.edittext3->w=200;
    
    obj_panel =  tt.glui->add_panel( "Controls" );
    
    tt.talk=new GLUI_Button(obj_panel, "Push to Send", Stop_Button, control_cb);
    
    new GLUI_Button(obj_panel, "Close", 2, control_cb);
    
    tt.glui->add_column(true);

    obj_panel =  tt.glui->add_panel( "Audio Source" );
    
    tt.audio = 0;
    
    // Default RtAudio constructor
    try {
        tt.audio = new RtAudio();
    }
    catch (...) {
        fprintf(stderr,"new RtAudio Failed");
        return 1;
    }
    
    if(tt.audio == NULL){
        fprintf(stderr,"audio %p\n",audio);
        return 0;
    }
    
    // Determine the number of devices available
    int devices = tt.audio->getDeviceCount();
 //   std::cout << "\nFound " << devices << " device(s) ...\n";
    
    // Scan through devices for various capabilities
    
#ifndef RTAUDIO_OLD
    std::vector<unsigned int> id=tt.audio->getDeviceIds();
#else
    std::vector<unsigned int> id;
    for(int n=0;n<devices;++n){
        id.push_back(n);
    }
#endif

    
    
    int find=0;
    RtAudio::DeviceInfo info;
    for (int i=0; i<devices; i++) {
        
        try {
            info = tt.audio->getDeviceInfo(id[i]);
            if(info.inputChannels > 0){
            // Print, for example, the maximum number of output channels for each device
                //std::cout << "device = " << i;
                //std::cout << ": maximum input  channels = " << info.inputChannels;
                //std::cout << " Device Name = " << info.name << '\n';
                new GLUI_Button(obj_panel, info.name.c_str(), 200+i, control_cb);
                if(find == 0){
                    tt.Params.deviceId=id[i];
                    find = 1;
                }
            }

        }
        catch (...) {
            fprintf(stderr,"RtAudio Info Error\n");
            break;
        }
        
    }
    
    if(find == 0)tt.Params.deviceId = tt.audio->getDefaultInputDevice();
    tt.Params.nChannels = 1;
    tt.Params.firstChannel = 0;



    tt.iic=0;
    
    tt.gain_Index=tt.iic;
    
    GLUI_Panel *panel3 = new GLUI_Panel(tt.glui, "RF Gain");
    tt.gain=tt.gainsMin;
    double el = tt.gain;
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

    tt.line_scroll[tt.iic]->set_float_limits( tt.gainsMin, tt.gainsMax );
    
    ++tt.iic;
    
    //printf("%s min %g max %g\n", "gain",rx->gainsMin,rx->gainsMax);
    

    
    
    
    tt.sub_window=glutGetWindow();
    
    tt.glui->set_main_gfx_window(tt.sub_window);
    
    return 0;
}
static void control_cb(int control)
{
    /*
    std::string file_name;
    double f,fc,gain,lineAlpha,scaleFactor;
    double dmin,dmax;
    double sameleRate;
    double bandwidth;
    int audioThreads;
     */
    
    double fc,foffset,transmitSampleRate;
    
    RadioPtr s=(RadioPtr)FindTransmit(glutGetWindow());
    if(!s)return;
        
    sscanf(s->tt.edittext2->get_text(),"%lg", &fc);
    sscanf(s->tt.edittext1->get_text(),"%lg", &foffset);
    sscanf(s->tt.edittext3->get_text(),"%lg", &transmitSampleRate);

    

        if(control == Mode_Buttons)
    {
        fprintf(stderr,"Mode_Buttons %d\n",s->tt.modetype);
    }
    if(control == 2)
    {
        if(s->tt.doTransmit == 1){
            s->tt.doTransmit=0;
            int count=0;
            while(s->tt.doTransmit == 0){
                Sleep2(10);
                if(++count > 200)break;
            }
            s->tt.doTransmit=0;
            s->tt.talk->set_name("Push To Send");
        }
        s->tt.glui->close();
        s->tt.glui=NULL;
        s->inTransmit=0;
        freeMemoryTransmit(&s->tt);
        delete s->tt.audio;

    }
    else if(control == Stop_Button)
    {
        fprintf(stderr,"Stop_Button %d doTransmit %d\n",control,s->tt.doTransmit);
        
        if(s->tt.doTransmit == 1){
            s->tt.doTransmit=0;
            int count=0;
            while(s->tt.doTransmit == 0){
                Sleep2(10);
                if(++count > 2000){
                    fprintf(stderr,"Stop Timed Out \n");
                    break;
                }
            }
            s->tt.doTransmit=0;
            s->tt.talk->set_name("Push To Send");
            return;
        }
        
        if(s->tt.doTransmit == 0){
            s->tt.fc=fc;
            s->tt.foffset=foffset;
            s->tt.transmitSampleRate=transmitSampleRate;
            s->tt.doTransmit=1;
            launchThread((void *)s,TransmitThread);
            s->tt.talk->set_name("Push To Stop");
        }

   }
    else if(control == Key_Button)
    {
        fprintf(stderr,"Key_Button %d\n",control);
    }
    else if(control == 50)
    {
        char value[256];
        double gain;
        int ind=s->tt.gain_Index;
        msprintf(value,sizeof(value),"%0.f",s->tt.line_Index[ind]);
        gain=atof(value);
        s->tt.gain=gain;
        //fprintf(stderr,"value %s gain %g\n",value,gain);
        s->tt.edittext1z[ind]->set_text(value);
        if(s->tt.line_Index_old[ind] != (int)s->tt.line_Index[ind]){
            s->tt.line_Index_old[ind]=(int)s->tt.line_Index[ind];
        }
    }

    else if(control >= 200 && control < 220)
    {
        fprintf(stderr,"Audio Source %d\n",control-200);
        s->tt.Params.deviceId = control-200;
        s->tt.Params.nChannels = 1;
        s->tt.Params.firstChannel = 0;
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
        
        over=1.0;
        
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
    
    SoapySDR::Device *device=rx->device;
    
    if(!device->getFullDuplex(SOAPY_SDR_TX, rx->channel)){

        if(rx->controlProcess >= 0){
            
            rx->controlProcess = -1;
            
            int count=0;
            while(rx->controlProcess == -1){
                Sleep2(10);
                if(++count > 200)break;
            }
            
        }
    
        device->deactivateStream(rx->rxStream);
        
    }

    // fprintf(stderr,"getFullDuplex %d\n",device->getFullDuplex(SOAPY_SDR_TX,rx->channel));
    
    //const double frequency = 27.315e6;  //center frequency to 500 MHz
    double frequency = 87.6e6;  //center frequency to 500 MHz
    double sample_rate = 2e6;    //sample rate to 5 MHz
    float As = 60.0f;
    
    std::vector<size_t> channels;
    
    channels = {0};
    
    frequency = s->tt.fc+s->tt.foffset;
    
    sample_rate=s->tt.transmitSampleRate;
    
    fprintf(stderr,"frequency %g Offset %g sample_rate %g\n",s->tt.fc,s->tt.foffset,sample_rate);
    
    s->tt.info.device=device;
    
    device->setSampleRate(SOAPY_SDR_TX, rx->channel, sample_rate);
    
    fprintf(stderr,"Sample rate: %g MHz\n",sample_rate/1e6);
    
    //Set center frequency
    
    
    device->setFrequency(SOAPY_SDR_TX, rx->channel, frequency);
    
    device->setGain(SOAPY_SDR_TX, rx->channel, s->tt.gain);
    
    //  device->setAntenna(SOAPY_SDR_TX, rx->channel, BAND1);
    
    //Streaming Setup
    
    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
    
    //fprintf(stderr,"txStream %p\n",txStream);
    
    s->tt.info.txStream=txStream;
    
    //size_t MTU=device->getStreamMTU(txStream);
    
    //fprintf(stderr,"MTU %ld\n",MTU);
    
    
    //device->setHardwareTime(0); //clear HW time for easy debugging
    
    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    
    //unsigned int samples=48000;
    unsigned int samples=44100;

    double pi;
    pi=4.0*atan(1.0);
    s->tt.info.dt=1.0/(double)samples;
    s->tt.info.sino=0;
    s->tt.info.coso=1;
    double w=2.0*pi*(1000);
    s->tt.info.sindt=sin(w*s->tt.info.dt);
    s->tt.info.cosdt=cos(w*s->tt.info.dt);

    float Ratio1 = (float)(10000/ (float)samples);
    
    float Ratio2 = (float)(sample_rate/(float)10000);
    
    liquid_ampmodem_type type=LIQUID_AMPMODEM_DSB;
    int flag=0;
    if(s->tt.modetype == Mode_NBFM){
        Ratio1 = (float)(12500/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)12500);
    }else if(s->tt.modetype == Mode_USB){
        type=LIQUID_AMPMODEM_USB;
        Ratio1 = (float)(6000/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)6000);
        flag=1;
    }else if(s->tt.modetype == Mode_LSB){
        type=LIQUID_AMPMODEM_LSB;
        Ratio1 = (float)(6000/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)6000);
        flag=1;
    }
    
    ampmodem demodAM;
    
#ifdef LIQUID_VERSION_4
    demodAM = ampmodem_create(0.5, 0.0, type, flag);
#else
    demodAM = ampmodem_create(0.5, type, flag);
#endif
    
    s->tt.info.modetype=s->tt.modetype;
    
    fprintf(stderr,"Ratio1 %g Ratio2 %g modetype %d\n",Ratio1,Ratio2,s->tt.modetype);
    
    msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
    
    s->tt.info.iqSampler=iqSampler;
    
    msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
    
    s->tt.info.iqSampler2=iqSampler2;
    
    s->tt.info.demodAM=demodAM;
    
    AMmod modulation(1.0);
    
    s->tt.info.am=&modulation;
    
    freqmod mod = freqmod_create(0.5);
    
    s->tt.info.mod=mod;
    
    s->tt.info.extraBytes=0;

    std::vector<void *> buffs(2);
    
    unsigned int bufferFrames = 2040;
    
    try {
 //       s->tt.audio->openStream( NULL, &s->tt.Params, RTAUDIO_SINT16, samples, &bufferFrames, &input, (void *)&s->tt.info );
        
        
#ifndef RTAUDIO_OLD
        RtAudio::StreamOptions options;
        options.flags = RTAUDIO_NONINTERLEAVED;
        s->tt.audio->openStream(NULL,  &s->tt.Params, RTAUDIO_SINT16,
                        (unsigned int)samples, &bufferFrames, &input, &s->tt.info, &options);
#else
        s->tt.audio->openStream(NULL,  &s->tt.Params, RTAUDIO_SINT16,
                        (unsigned int)samples, &bufferFrames, &input, &s->tt.info);
#endif

        
        
        
    }
    catch (...) {
        std::cout << '\n' << "audio->openStream Error" << '\n' << std::endl;
        goto cleanup;
    }
    
    //fprintf(stderr,"getStreamSampleRate %u\n",s->tt.audio->getStreamSampleRate());
    
    fprintf(stderr,"Ready For Voice\n");
    
    try {
        s->tt.audio->startStream();
    }
    catch (...) {
        std::cout << '\n' << "audio->startStream" << '\n' << std::endl;
        goto cleanup;
    }
    
    while(s->tt.doTransmit == 1){
        if ( s->tt.audio->isStreamRunning() ) {
            Sleep2(10);
        }
    }
cleanup:
    
    if (  s->tt.audio->isStreamOpen() )  s->tt.audio->closeStream();
    
    freqmod_destroy(mod);
    
    if(demodAM)ampmodem_destroy(demodAM);
    
    if(!device->getFullDuplex(SOAPY_SDR_TX, rx->channel)){

/*
        extern int AudioReset(struct playData *rx);
        device->activateStream(rx->rxStream);
        {
            extern int rxBuffer(void *rxv);
        
            rx->controlRF=2;
            
            launchThread((void *)rx,rxBuffer);
        }
*/
        Sleep2(200);
        
        device->deactivateStream(txStream);
        
        device->closeStream(txStream);
        
        s->stopPlay(s->rx);

        s->startPlay(s->rx);
        
        s->playRadio(s->rx);
        
    }else{
        Sleep2(200);
        
        device->deactivateStream(txStream);
        
        device->closeStream(txStream);

        device->setFrequency(SOAPY_SDR_TX, rx->channel, frequency);
        
        device->setGain(SOAPY_SDR_TX, rx->channel, 0.0);
    }


    s->tt.doTransmit = -1;
    
    device=NULL;
    
    txStream=NULL;
    
    fprintf(stderr,"Transmit Thread End\n");
    
    return 0;
}

int input( void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
          double /*streamTime*/, RtAudioStreamStatus /*status*/, void *datain )
{
    struct Info *info=(struct Info *)datain;
    unsigned int frames = nBufferFrames;
    short *in=(short *)inputBuffer;
    
    if(info->Tone){
        for(unsigned int k=0;k<frames;++k){
            double sint=info->sino*info->cosdt+info->coso*info->sindt;
            double cost=info->coso*info->cosdt-info->sino*info->sindt;
            info->coso=cost;
            info->sino=sint;
            double v=in[k]+16000*cost;
            if(v > 32000)v=32000;
            if(v < -32000)v=-32000;
            in[k]=(short)(v);
        }
        
        double r=sqrt(info->coso*info->coso+info->sino*info->sino);
        info->coso /= r;
        info->sino /= r;
    }
    SendData(info,frames,in);
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
        buf[k]=0.00002*bufin[k];
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

    if(info->modetype == Mode_AM){
        info->am->modulate(buf2,num,r2);
    }else if(info->modetype == Mode_NBFM){
        freqmod_modulate_block(info->mod, buf2, num, (liquid_float_complex *)r2);
    }else{
        ampmodem_modulate_block(info->demodAM,buf2, num, (liquid_float_complex *)r2);
    }
    
/*
    info->am->modulate(buf,frames,r2);
    
    msresamp_crcf_execute(info->iqSampler2, (liquid_float_complex *)r2, frames, (liquid_float_complex *)buf2, &num2);
*/
    
    msresamp_crcf_execute(info->iqSampler2, (liquid_float_complex *)r2, num, (liquid_float_complex *)buf2, &num2);
    
    float *out=buf2;
    
    if(info->extraBytes > 0){
        float *out2=buf;
        for(int k=0;k<info->extraBytes ;++k){
            out2[2*k]=r[2*k];
            out2[2*k+1]=r[2*k+1];
        }
        int nn=info->extraBytes;
        for(unsigned int k=0;k<num2 ;++k){
            out2[2*nn]=out[2*k];
            out2[2*nn+1]=out[2*k+1];
            ++nn;
        }
        out=out2;
        num2=nn;
        info->extraBytes=0;
    }
    
    std::vector<void *> buffs(2);
    
    int flags(0);

    
    unsigned int tosend;
    
    tosend=num2;
    
    while(1){
        buffs[0] = out;
        if(tosend < 4096){
            for(unsigned int k=0;k<tosend;++k){
                r[2*k]=out[2*k];
                r[2*k+1]=out[2*k+1];
            }
            info->extraBytes=tosend;
            return 0;
        }
        int ret = info->device->writeStream(info->txStream,  &buffs[0], 4096, flags);
        if(ret < 0){
            std::cerr << "writeStream " << ret << "  " << SoapySDR::errToStr(ret) << std::endl;
            //if(ret == SOAPY_SDR_TIMEOUT)Sleep2(100);
            return 0;
        }
        
        if(ret >= (int)tosend)break;
        out += 2*ret;
        tosend -= ret;
        if(tosend <= 0){
            cout << "error: samples sent: " << tosend << "/" << num2 << endl;
            break;
        }
    }
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
int freeMemoryTransmit(struct TransmitData *rx)
{
    if(rx->antenna){
        for (size_t i=0;i<rx->antennaCount;++i){
            cFree((char *)rx->antenna[i]);
        }
        cFree((char *)rx->antenna);
        rx->antenna=NULL;
    }
    
    if(rx->gains){
        for (size_t j = 0; j < rx->gainsCount; j++)
        {
            cFree((char *)rx->gains[j]);
        }
        cFree((char *)rx->gains);
        rx->gains=NULL;
    }
    
    if(rx->streamFormat){
        for (size_t j = 0; j < rx->streamFormatCount; j++)
        {
            cFree((char *)rx->streamFormat[j]);
        }
    }

    
    if(rx->gainsMinimum)cFree((char *)rx->gainsMinimum);
    rx->gainsMinimum=NULL;
    if(rx->gainsMaximum)cFree((char *)rx->gainsMaximum);
    rx->gainsMaximum=NULL;
    if(rx->frequencyMinimum)cFree((char *)rx->frequencyMinimum);
    rx->frequencyMinimum=NULL;
    if(rx->frequencyMaximum)cFree((char *)rx->frequencyMaximum);
    rx->frequencyMaximum=NULL;
    if(rx->bandwidths)cFree((char *)rx->bandwidths);
    rx->bandwidths=NULL;
    if(rx->streamFormat)cFree((char *)rx->streamFormat);
    rx->streamFormat=NULL;
    if(rx->sampleRates)cFree((char *)rx->sampleRates);
    rx->sampleRates=NULL;
    return 1;
}

