//
//  FMRadio.cpp
//  SdrGlut
//
//  Created by Dale on 10/8/16.
//  Copyright © 2016 Dale Ranta. All rights reserved.
//
#include "Radio.h"

#include "RecordIQ.h"

#include "mThread.h"

#include <thread>

#include "DialogFileOpen.h"

#include <math.h>

#include <GL/glui.h>

extern "C" int closeScene(struct Scene *scene);

extern "C" int GridPlotScale(struct uGridPlot *b);

void menu_select(int item);

static void menu_selectl(int item);

static void enterexit(int item);

static void inuse(int item);

void palette_select2(int item);

void setMode(int item);

void SetLighting(unsigned int mode);

static void myReshape(int w, int h);

static void reshape(struct Scene *scene,int wscr,int hscr);

static void moveMouse(int x, int y);

void getMouse(int button, int state, int x, int y);

void keys(unsigned char key, int x, int y);

void keyss(int key, int x, int y);

void display(void);

int dialogSdrData(struct Scene *scene);

extern "C" int DrawLine(int x1, int y1, int x2, int y2);

#define ControlGetSelectionBox	102
#define SdrDialog               103
#define SdrTransmit             104
#define SdrSend                 105
#define SdrReadFile             106
#define SdrSendIQ               107
#define SdrVoiceControl         108

ALvoid DisplayALError(unsigned char *szText, ALint errorCode);

static int initScene(struct Scene *scene);

extern "C" int doFFT2(double *x,double *y,long length,int direction);

int doWindow(double *x,double *y,long length,int type);

static void getMousel(int button, int state, int x, int y);

static void getMousePassive(int x, int y);


static void getMousePassive3(int x, int y);

static int DrawString(int x, int y, char *out);

int DrawBox(uRect *box,int offset);

static GLUI *glui;

int doRadioOpenRA(std::string argStr);

static int doRadioOpen2(SoapySDR::Kwargs deviceArgs);

static void control_cb(int control);

void antennaMenu(int item);
void bandMenu(int item);
void sampleMenu(int item);
void doAudio(int item);

static void keys2(unsigned char key, int x, int y);

static int device;
static GLUI_EditText *edittext1;
static char sfrequency[255]="1";
static char ssamplerate[255]="2";
static char ssmode[255]="AM";
static std::vector<SoapySDR::Kwargs> results;


void doFFTMenu(int item);
void doDirectSampleMode(int item);
void doBiasMode(int item);
void doFilterMenu(int item);

#define Mode_Buttons   180

int datatype[100];

int boxnumber[100];

double *ww;

static int OpenZoomWindow;

static GLUI_Listbox  *box[100];

static GLUI_Listbox  *boxmode;
int modetype;

int iblade;
int iant;

RadioPtr RadioWindowSetFrequency(struct playData *rx);

SNDFILE *sfopen(char *filename)
{
    char strbuffer[256];
    static int pversion=1;
    
    sf_command (NULL, SFC_GET_LIB_VERSION, strbuffer, sizeof (strbuffer)) ;
    
    if(pversion == 1)printf("sndfile version %s\n",strbuffer);
    pversion=0;
    
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof (sfinfo));
    sfinfo.samplerate = 48000;
    sfinfo.channels   = 1; // mono
    sfinfo.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    
    if(!filename)return NULL;
    
    SNDFILE * fid_wav = sf_open(filename, SFM_WRITE, &sfinfo);
    
    return fid_wav;
}

int Radio::controlScan(struct playData *rx)
{
    return 0;
}
int Radio::processScan(struct playData *rx)
{
    if(!rx)return 0;
    scanFrequencies.clear();
    int ns = -1;
    double peak=-160;
    double bw=rx->bw;
    double fStart=0;
    for(int n=20;n<rx->FFTcount;++n){
        if(ampitude[n] >= rx->cutOFF){
            if(ns == -1)fStart=frequencies[n];
            // fprintf(stderr,"n %d ns %d %g %g %g\n",n,ns,s->ampitude[n],s->rx->cutOFF,peak);
            if(ampitude[n] > peak){
                peak=ampitude[n];
                ns=n;
            }
        }else{
            if(ns >= 0){
                if(frequencies[n] < fStart+bw)continue;
                 scanFrequencies.push_back(frequencies[ns]);
                ns=-1;
                peak=-160;
            }
        }
    }
    
    return 0;
}

int setInfo(struct playData *rx)
{
    unsigned int samples=48000;
    
    
    if(rx->decodemode == MODE_CW){
        rx->info1.Tone=1;
    }else{
        rx->info1.Tone=0;
    }
    
    rx->info1.f=1000;
    
    double pi;
    pi=4.0*atan(1.0);
    rx->info1.dt=1.0/(double)samples;
    rx->info1.sino=0;
    rx->info1.coso=1;
    double w=2.0*pi*(rx->info1.f);
    rx->info1.sindt=sin(w*rx->info1.dt);
    rx->info1.cosdt=cos(w*rx->info1.dt);
    
    return 0;
}

int Radio::fftIndex(double frequency)
{
    if(!rx)return -1;
    int index=(int)(0.5+rx->FFTcount*((frequency - rx->fc)+0.5*rx->samplerate)/rx->samplerate);
    if(index >= 0 && index < rx->FFTcount-1)return index;
    return -1;
}


Radio *Radio::findMate(struct playData *rx)
{
    if(!Root)return NULL;
    
    if(!rx)return NULL;
    
    RadioPtr f;
    CWinPtr w;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSdrRadio){
            f=(RadioPtr)w;
            if(f->rx->device == rx->device && f->rx != rx){
                return f;
            }
        }
        w=w->CNext;
    }
    
    return NULL;
}
int Radio::playRadio(struct playData *rx)
{
    if(!rx)return 0;

    if(rx->nreceive <= 1){
        rx->pplayRadio(rx);
        return 0;
    }
    
    if(!Root)return 1;
    
    RadioPtr f=findMate(rx);
    if(f){
        f->rx->pplayRadio(f->rx);
    }else{
        fprintf(stderr,"Radio::playRadio error second window not found\n");
    }
    
    rx->pplayRadio(rx);

    return 0;
}
int Radio::stopPlay(struct playData *rx)
{
    if(!rx)return 0;

    if(rx->nreceive <= 1){
        rx->pstopPlay(rx);
        return 0;
    }
    
    RadioPtr f=findMate(rx);
    if(f){
        if(rx->channel == 0){
            f->rx->pstopPlay(f->rx);
            rx->pstopPlay(rx);
        }else{
            rx->pstopPlay(rx);
            f->rx->pstopPlay(f->rx);
        }
        return 0;
    }
    
    rx->pstopPlay(rx);
    
    return 0;
}
int Radio::startPlay(struct playData *rx)
{
    if(!rx)return 0;
    
    if(rx->nreceive <= 1){
        rx->pstartPlay(rx);
        return 0;
    }
    
    RadioPtr f=findMate(rx);
    if(f){
        if(rx->channel == 0){
            rx->pstartPlay(rx);
            f->rx->pstartPlay(f->rx);
        }else{
            f->rx->pstartPlay(f->rx);
            rx->pstartPlay(rx);
        }
        return 0;
    }

    fprintf(stderr,"Radio::startPlay error second window not found\n");

    return 0;
}
int Radio::setFrequencyDuo(struct playData *rx)
{
    if(!rx)return 0;
    
    if(rx->fc < 0.5*rx->samplerate)rx->fc=0.5*rx->samplerate;
    
    try{
        rx->device->setFrequency(SOAPY_SDR_RX,rx->channel,"RF",rx->fc-rx->foffset);
    }
    catch(...)
    {
        return 0;
    }

    
    rx->averageGlobal=0;
    
    setInfo(rx);
    
    return 0;
}
int Radio::closeScenes()
{
    stopPlay(rx);
    backGroundEvents = 0;
    if(rx->nreceive <= 1){
        if (FindScene(scenel))closeScene(lines->scene);
        if (FindScene(scenel2))closeScene(lines2->scene);
        if (FindScene(scene))closeScene(scene);
        return 0;
    }
    
    RadioPtr f=findMate(rx);
    if(f){
        if(rx->channel == 0){
            if (FindScene(f->scenel))closeScene(f->lines->scene);
            if (FindScene(f->scenel2))closeScene(f->lines2->scene);
            if (FindScene(f->scene))closeScene(f->scene);
            if (FindScene(scenel))closeScene(lines->scene);
            if (FindScene(scenel2))closeScene(lines2->scene);
            if (FindScene(scene))closeScene(scene);
        }else{
            if (FindScene(scenel))closeScene(lines->scene);
            if (FindScene(scenel2))closeScene(lines2->scene);
            if (FindScene(scene))closeScene(scene);
            if (FindScene(f->scenel))closeScene(f->lines->scene);
            if (FindScene(f->scenel2))closeScene(f->lines2->scene);
            if (FindScene(f->scene))closeScene(f->scene);
        }
    }
    
    return 0;
}
int Radio::doSoundRecord()
{
    struct RecordSoundStruct *s=&rs;
    time_t now;
    int nSound;
    
    nSound=sizeof(s->on)/sizeof(int);
    
    for(int k=0;k<nSound;++k){
        if(!s->on[k])continue;
        time(&now);
        if(s->state[k]){
            //fprintf(stderr,"end %ld now %ld\n",(s->start[k]+s->stop[k]) , now);
            if((s->start[k]+s->stop[k]) < now){
                fprintf(stderr,"Close 3 Mode %s File %s\n",s->mode[k],s->FilePath[k]);
                s->state[k]=0;
                if(rx->audioOutput)sf_close(rx->audioOutput);
                rx->audioOutput=NULL;
                break;
            }else{
                for(int kk=0;kk<nSound;++kk){
                    if(!s->on[kk] || kk == k)continue;
                    if(s->start[kk] > s->start[k] && s->start[kk] < now && (s->start[kk]+s->stop[kk]) >= now){
                        fprintf(stderr,"Close 2 Mode %s File %s\n",s->mode[k],s->FilePath[k]);
                        if(rx->audioOutput)sf_close(rx->audioOutput);
                        s->state[k]=0;
                        char buff1[256],buff2[256];
                        sprintf(buff1,"%lf",s->frequency[kk]);
                        sprintf(buff2,"%s",s->mode[kk]);
                        sendMessageGlobal(&buff1[0],&buff2[0],M_SEND);
                        fprintf(stderr,"Open 2 Mode %s File %s\n",s->mode[kk],s->FilePath[kk]);
                        rx->audioOutput=sfopen(s->FilePath[kk]);
                        if(!rx->audioOutput){
                            fprintf(stderr,"Error Opening %s To Write\n",s->FilePath[kk]);
                        }else{
                            s->state[kk]=1;
                        }
                        return 0;
                    }
                }
                return 0;
            }
        }
    }
    
    for(int k=0;k<nSound;++k){
        if(!s->on[k])continue;
        time(&now);
        if(s->start[k] <= now && (s->start[k]+s->stop[k]) >= now){
            if(rx->audioOutput)sf_close(rx->audioOutput);
            fprintf(stderr,"Open 1 Mode %s File %s\n",s->mode[k],s->FilePath[k]);
            char buff1[256],buff2[256];
            sprintf(buff1,"%lf",s->frequency[k]);
            sprintf(buff2,"%s",s->mode[k]);
            sendMessageGlobal(&buff1[0],&buff2[0],M_SEND);
            rx->audioOutput=sfopen(s->FilePath[k]);
            if(!rx->audioOutput){
                fprintf(stderr,"Error Opening %s To Write\n",s->FilePath[k]);
            }else{
                s->state[k]=1;
            }
            return 0;
        }

    }

    return 0;
}
Radio::Radio(struct Scene *scene,SoapySDR::Kwargs deviceArgs): CWindow(scene)
{
    OpenError=TRUE;

	zerol((char *)&water, sizeof(water));

    zerol((char *)&dd, sizeof(dd));
    
    zerol((char *)&bb, sizeof(bb));
    
    zerol((char *)&qq, sizeof(qq));

    zerol((char *)&pd, sizeof(pd));
    
    zerol((char *)&rs, sizeof(rs));
    
    zerol((char *)&start,&end-&start+1);
    
    
    mstrncpy(rs.FilePath[0],(char *)"sound01.wav",sizeof(rs.FilePath[0]));
    mstrncpy(rs.FilePath[1],(char *)"sound02.wav",sizeof(rs.FilePath[1]));
    mstrncpy(rs.FilePath[2],(char *)"sound03.wav",sizeof(rs.FilePath[2]));
    mstrncpy(rs.FilePath[3],(char *)"sound04.wav",sizeof(rs.FilePath[3]));
    mstrncpy(rs.FilePath[4],(char *)"sound05.wav",sizeof(rs.FilePath[4]));

    for(int k=0;k<5;++k){
        time(&rs.start[k]);
        mstrncpy(rs.mode[k],(char *)"FM",sizeof(rs.mode[k]));

    }

    zerol((char *)&rxs,(&rxs.end-&rxs.start)+1);
    
    rxs.mRadio=this;
    
   // fprintf(stderr,"diff %ld %ld \n",(long)(&rxs.end-&rxs.start),(long)((char *)(&rxs.end)-(char *)(&rxs.start)));
    
    rxs.deviceToOpen=deviceArgs;

	getPaletteByName((char *)"ps", (unsigned char *)&pd.palette);

	pd.sType = 2;
    
    backGroundEvents=0;

    scanRun=0;
    
    scanWait=0;
    
    lineDumpInterval=0.1;
    lineTime=rtime()+lineDumpInterval;
    
    lineAlpha=0.1;
    
    FFTlength=131072;
    
    range=NULL;
    range3=NULL;
    magnitude=NULL;
    magnitude2=NULL;
    magnitude3=NULL;

    frequencies=NULL;
    ampitude=NULL;
    
    
    range=(double *)cMalloc(FFTlength*sizeof(double),9851);
    range3=(double *)cMalloc(FFTlength*sizeof(double),9851);
    magnitude=(double *)cMalloc(FFTlength*sizeof(double),9851);
    magnitude2=(double *)cMalloc(FFTlength*sizeof(double),9851);
    magnitude3=(double *)cMalloc(FFTlength*sizeof(double),9851);

    frequencies=(double *)cMalloc(FFTlength*sizeof(double),9851);
    ampitude=(double *)cMalloc(FFTlength*sizeof(double),9851);
    
    if(!range || !magnitude || !frequencies || !ampitude || !magnitude2 || !range3 || !magnitude3)return;
    
    zerol((char *)range,FFTlength*sizeof(double));
    zerol((char *)range3,FFTlength*sizeof(double));
    zerol((char *)magnitude,FFTlength*sizeof(double));
    zerol((char *)magnitude3,FFTlength*sizeof(double));

    zerol((char *)frequencies,FFTlength*sizeof(double));
    zerol((char *)ampitude,FFTlength*sizeof(double));
    
    flagsflag=0;
    
    pauseTimeDelta=3.0;
    
    pauseTime=rtime();
    
    pauseChannel=0;

    char sdevice[10];
    
    sprintf(sdevice,"%d",device);
    
    {
        char *list[]={NULL,(char *)"-fc",sfrequency,(char *)"-gain" ,(char *)"0.25",(char *)"-device",sdevice,
                      (char *)"-samplerate",ssamplerate,ssmode};
        
        //char *list[]={NULL,"-fc","27.1e6" ,"-f" ,"27.3235e6" ,"-lsb" ,"-gain" ,"1","-device",sdevice};

        RadioStart ((int)sizeof(list)/sizeof(char *), list,&rxs);
        
    }

    rx=&rxs;
    
    
    if(!rx)return;

    
    //fprintf(stderr,"Radio::Radio\n");
    
    
    OpenError=FALSE;
}

static int doRadioOpen2(SoapySDR::Kwargs deviceArgs)
{
    struct SceneList *list;
    struct Scene *scene;
    int FileType;
    
    
    SoapySDR::Device *devicer = SoapySDR::Device::make(deviceArgs);
    
    int nreceive=(int)devicer->getNumChannels(SOAPY_SDR_RX);
    if(nreceive > 1)nreceive=1;
    
    int ntransmit=(int)devicer->getNumChannels(SOAPY_SDR_TX);
    if(ntransmit > 1)ntransmit=1;
        
    printf("receive channels %d transmit channels %d\n",nreceive,ntransmit);
    
    SoapySDR::Device::unmake(devicer);

    FileType=FileTypeSdrRadio;
    
    for(int nd=0;nd<nreceive;++nd){
        
        device=nd;
        
        list=SceneNext();
        if(list == NULL)
        {
            WarningPrint("doRadioOpen : Error Allocation Scene Memory File\n",FileType);
            return 1;
        }
        scene=&list->scene;
        zerol((char *)scene,sizeof(struct Scene));
        SceneInit(scene);
        scene->windowType=FileTypeSdrRadio;

        RadioPtr w = new Radio(scene,deviceArgs);
        
        if(w == NULL){
            WarningBatch((char *)"Radio of Memory");
            return 1;
        }
        
        if(w->OpenError != FALSE){
            delete w;
            return 0;
        }
        
        w->OpenWindows(scene);
        
        w->LoadFile(scene,NULL,FileType);

        myAppl=(CWinPtr)w;
        
        AddWindowList(myAppl);
        
        if(w->lines2){
            glutSetWindow(w->lines2->window);
            glutSetWindowTitle(w->rx->driveName);
        }
        
        glutSetWindow(w->window);
        glutSetWindowTitle(w->rx->driveName);
        mstrncpy(w->windowName,w->rx->driveName,sizeof(w->windowName));
        w->backGroundEvents=1;
    }
   return 1;
}

int doRadioOpenRA(std::string argStr)
{
    
    size_t length;
    
/*
    std::string argStr="driver=redpitaya";
    argStr="";
 */
    
    results = SoapySDR::Device::enumerate(argStr);
    
    length=results.size();
    
    device=0;

    if(length == 0){
        fprintf(stderr,"Error: enumerate Found No Devices - Try Again !\n");
        return 0;
    }
    
    // std::cout << "results.size " << results.size() << std::endl;
    
    SoapySDR::Kwargs deviceArgs;

    if(glui){
        glui->close();
    }
    
    glui = GLUI_Master.create_glui("Select Device");
    
    msprintf(sfrequency,sizeof(sfrequency),"%g",101.5);
    
    msprintf(ssamplerate,sizeof(ssamplerate),"%.0f",2000000.0);
    
    GLUI_Panel *obj_panel =  glui->add_panel( "Parameters" );

    edittext1 =
    glui->add_edittext_to_panel( obj_panel,  "Frequency(MHZ):", GLUI_EDITTEXT_TEXT, sfrequency );
    edittext1->w=200;
    
    modetype=0;
    boxmode=glui->add_listbox_to_panel(obj_panel, "Mode : ",&modetype, Mode_Buttons-10, control_cb);
    boxmode->add_item(0,"AM");
    boxmode->add_item(1,"NAM");
    boxmode->add_item(2,"FM");
    boxmode->add_item(3,"NBFM");
    boxmode->add_item(4,"USB");
    boxmode->add_item(5,"LSB");
    boxmode->add_item(6,"CW");

    glui->add_checkbox_to_panel(obj_panel, "Open Zoom Window", &OpenZoomWindow, 999, control_cb);

    obj_panel =  glui->add_panel( "Device" );
    
    int nb=0;
    
    int iopen=0;
    
    for(size_t k=0;k<length;++k){
        string name;
        
        name="";
        try {

            deviceArgs = results[k];
            for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
                printf("%s=%s\n",it->first.c_str(),it->second.c_str());
                if (it->first == "driver") {
                    if(it->second == "audio")break;
                    if(it->second == "redpitaya"){
                        name=it->second;
                    }
                } else if (it->first == "device") {
                    if(it->second == "HackRF One")continue;
                    name=it->second;
                } else if (it->first == "label") {
                    name=it->second;
                }
            }
            if(name != ""){
                
                SoapySDR::Device *devicer = SoapySDR::Device::make(deviceArgs);
                
                ++iopen;
                
                new GLUI_Button(obj_panel, (char *)name.c_str(), (int)(2+k), control_cb);
                
                datatype[nb]=0;
                
                box[nb]=glui->add_listbox_to_panel(obj_panel, "Sample Rate : ",&datatype[nb], Mode_Buttons+nb, control_cb);

                boxnumber[k]=nb;
                
                std::vector<double> rate=devicer->listSampleRates(SOAPY_SDR_RX,0);
                for (size_t j = 0; j < rate.size(); j++)
                {
                    char data[256];
                    unsigned long long irate=rate[j];
                    sprintf(data,"%llu\n",irate);
                    box[nb]->add_item((int)j,data);

                }
                SoapySDR::Device::unmake(devicer);
                ++nb;

            }
            
        } catch(const std::exception &e) {
            std::string streamExceptionStr = e.what();
            printf("doRadioOpen Error: %s\n",streamExceptionStr.c_str());
        }

            
    }
    
    if(iopen == 0){
        new GLUI_Button(glui, "Error: No SDR Device Found" , 200, control_cb);
    }
    
    new GLUI_Button(glui, "Close", 1, control_cb);

    glui->set_main_gfx_window( glutGetWindow() );
    
    return 0;
}

static void control_cb(int control)
{
    if(control == 1 || control == 200){
        glui->close();
        glui = NULL;
        return;
    }
    
    // printf("control %d\n",control);
    printf("modetype %d\n",modetype);

    
    if(control >= Mode_Buttons-10){
        return;
    }
    
    glui->close();
    glui = NULL;
    
    device = control-2;
    
    int bn=boxnumber[device];
    
    msprintf(ssamplerate,sizeof(ssamplerate),"%s",box[bn]->get_item_ptr(datatype[bn])->text.c_str());
    
    msprintf(ssmode,sizeof(ssmode),"%s",boxmode->get_item_ptr(modetype)->text.c_str());

    SoapySDR::Kwargs deviceArgs=results[device];
    
    iant=0;
    iblade=0;
    for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
        if (it->first == "driver") {
            if(it->second == "bladerf"){
                printf("Blade Found\n");
                iblade=1;
            }else if(it->second == "uhd"){
                printf("AntSDR Found\n");
                iant=1;
            }
        }
    }


    doRadioOpen2(deviceArgs);
}

int Radio::LoadFile(struct Scene *scene,char *name, int fileType)
{
    return 1;
}
int Radio::initPlay(struct playData4 *rx)
{
    return 0;
}

void Radio::playRadio (struct playData4 *rx)
{
    return ;
}

Radio::~Radio()
{
 //   fprintf(stderr,"~Radio() start\n");
    
    rx->psdrDone(rx);

    if(range)cFree((char *)range);
    range=NULL;
    
    if(range3)cFree((char *)range3);
    range3=NULL;

    if(magnitude)cFree((char *)magnitude);
    magnitude=NULL;
    
    if(magnitude2)cFree((char *)magnitude2);
    magnitude2=NULL;
    
    if(magnitude3)cFree((char *)magnitude3);
    magnitude3=NULL;
    
    if(frequencies)cFree((char *)frequencies);
    frequencies=NULL;
    
    if(ampitude)cFree((char *)ampitude);
    ampitude=NULL;
        
    if(water.data)cFree((char *)water.data);
    water.data=NULL;

    sceneClean(scene);

    if(bb.glui){
        glutSetWindow(bb.sub_window);
        printf("bb.sub_window %d\n",bb.sub_window);
        bb.glui->close();
    }
    bb.glui=NULL;

    if(dd.glui){
        glutSetWindow(dd.sub_window);
        printf("dd.sub_window %d\n",dd.sub_window);
        dd.glui->close();
    }
    dd.glui=NULL;


    if(inTransmit){
        extern int freeMemoryTransmit(struct TransmitData *rx);
        inTransmit=0;
        if(tt.glui){
            freeMemoryTransmit(&tt);
            glutSetWindow(tt.sub_window);
            tt.glui->close();
        }
        tt.glui=NULL;
    }
//    fprintf(stderr,"~Radio() end\n");

}
int Radio::welch(double *real,double *imag,int *lengthi)
{

    int length=*lengthi;
//#define WELCH
#ifdef WELCH
    double *rsum=range3;
    double *isum=magnitude3;
    for(int n=0;n<length;++n){
        rsum[n]=0;
        isum[n]=0;
    }
    int ncut=4;
    
    length=length/ncut;
    
    for(int k=0;k<ncut*2-1;++k){
        doWindow(real+k*length/2,imag+k*length/2,length,rx->FFTfilter);
        
        for(int n=0;n<length;++n){
            real[n+k*length/2] *= pow(-1.0,n);
            imag[n+k*length/2] *= pow(-1.0,n);
        }
        doFFT2(real+k*length/2,imag+k*length/2,length,1);

        for(int n=0;n<length;++n){
            rsum[n]+=real[n+k*length/2];
            isum[n]+=imag[n+k*length/2];
        }
    }
    
    for(int n=0;n<length;++n){
        real[n]=rsum[n]/(double)(2*ncut-1);
        imag[n]=isum[n]/(double)(2*ncut-1);
    }

    *lengthi=length;
    
#else
    doWindow(real,imag,length,rx->FFTfilter);
    
    for(int n=0;n<length;++n){
        real[n] *= pow(-1.0,n);
        imag[n] *= pow(-1.0,n);
    }
    int direction=1;
    doFFT2(real,imag,length,direction);
#endif
    return 0;
}
int Radio::updateLine()
{
    
    double *real,*imag;
    double amin,amax,v;
    
    if(!rx)return 0;
    
    if(rtime() < lineTime)return 0;

    lineTime=rtime()+lineDumpInterval;
    
    if(rx->controlRF != 2)return 0;
        
    if(rx->FFTcount > FFTlength){
        printf(" FFTlength %ld\n",FFTlength);
        return 1;
    }
    
    int length=rx->FFTcount;
    
    for(int k=0;k<length;++k){
        rx->reals[k]=rx->real[k];
        rx->imags[k]=rx->imag[k];
    }

    real=rx->reals;
    imag=rx->imags;
    
    welch(real,imag,&length);
    
    amin =  0.0;
    amax =  0.0;
    int nn=0;
    for(int n=10;n<length-10;++n){
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        double mag=(1.0-lineAlpha)*magnitude[length-n-1]+v*lineAlpha;
        amin +=  mag;
        amax +=  v;
        ++nn;
     }
    
    amin /= nn;
    amax /= nn;

    double shift=-90-amin;
    rx->shiftGlobal = amax;
    if(rx->aminGlobal3 == 0.0)rx->aminGlobal3=shift;
    rx->aminGlobal3 = 0.9*rx->aminGlobal3+0.1*shift;
 //   shift=rx->aminGlobal3;

    //printf("shift %g amin %g \n",shift,amin);
    
    amin =  1e33;
    amax = -1e33;

    double rmin=  1e33;
    double rmax= -1e33;
    
    float dx=rx->samplerate;
    double ddx=(double)rx->samplerate/(double)(length);
    long nf=0;
    int voice=voiceSpectrum;
    for(int n=0;n<length;++n){
        double r;
        r=rx->fc-0.5*rx->samplerate+n*ddx;
        if(r < rmin)rmin=r;
        if(r > rmax)rmax=r;
        range[n]=r;
        if(abs(range[n]-rx->f) < dx)
        {
            dx=abs(range[n]-rx->f);
            nf=n;
        }
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        magnitude[length-n-1]=((1.0-lineAlpha)*magnitude[length-n-1]+v*lineAlpha)+shift;
        v=magnitude[length-n-1];
        magnitude2[length-n-1]=v+rx->scaleFactor;
        if(v < amin)amin=v;
        if(v > amax)amax=v;
        if(voice){
            range3[n]=range[n];
            magnitude3[n]=magnitude2[n];
        }
    }
    
  //printf("a amin %g amax %g \n",amin,amax);

    
    if(FindScene(scenel)){
        lines->plotPutData(scenel,range,magnitude2,length,1L);
        
        uGridPlotPtr Plot;
        Plot=lines->lines->Plot;
        if(!Plot)return 1;

        //printf("amin %g amax %g\n",amin,amax);
        
       if(!Plot->xManualControl){
            Plot->xAutoMaximum=TRUE;
            Plot->xAutoMinimum=TRUE;
       }

        GridPlotScale(Plot);
        
        if(!Plot->xManualControl){
            Plot->xAutoMaximum=FALSE;
            Plot->xAutoMinimum=FALSE;
            Plot->xSetMaximum=rmax;
            Plot->xSetMinimum=rmin;
            rmax=rx->fv+0.5*rx->viewWindow;
            if(rmax > rx->fc+0.5*rx->samplerate)rmax=rx->fc+0.5*rx->samplerate;
            rmin=rx->fv-0.5*rx->viewWindow;
            if(rmin < rx->fc-0.5*rx->samplerate)rmin=rx->fc-0.5*rx->samplerate;
            if(rmin < 0)rmin=0;
            rx->rmin=rmin;
            rx->rmax=rmax;
            Plot->xSetMaximum=rmax;
            Plot->xSetMinimum=rmin;
        }else{
            rmax=Plot->xSetMaximum;
            rmin=Plot->xSetMinimum;

        }
                
        if(rx->cutOFFSearch){
            for(int k=0;k<length;++k){
                frequencies[k]=range[k];
                if(magnitude2[k] > ampitude[k]){
                    ampitude[k]=magnitude2[k];
                }
            }
        }
        
        if(scanRun == 1 && scanWait != 1){
            int ifound=0;
            for(vector<double>::size_type k=0;k<scanFrequencies.size();++k){
               scanFound[k]=0;
                int n1=fftIndex(scanFrequencies[k]-0.3*rx->bw);
                int n2=fftIndex(scanFrequencies[k]+0.3*rx->bw);
                if(n1 < 0 || n2 < 0)continue;
                for(int m=n1;m<=n2;++m){
                    if(magnitude2[m] > rx->cutOFF){
                        scanFound[k]=1;
                        ifound=1;
                        break;
                    }
                }
            }
            if(ifound){
                rx->muteScan = 0;
                if(scanFound[pauseChannel]){
                    if(rtime() < pauseTime)goto FoundTime;
                }
                
                pauseTime=rtime()+pauseTimeDelta;
                int lastChannel=pauseChannel;
                if(++pauseChannel >= (int)scanFrequencies.size())pauseChannel=0;
                for(vector<double>::size_type k=pauseChannel;k<scanFrequencies.size();++k){
                    if(scanFound[k]){
                        pauseChannel=(int)k;
                        //fprintf(stderr,"Select channel %d\n",pauseChannel);
                        rx->f=scanFrequencies[k];
                        //fprintf(stderr,"k %d 1 Frequency Selected %.4f\n",(int)k,scanFrequencies[k]/1e6);
                        setFrequency3(rx);
                        goto FoundTime;
                    }
                }
                for(vector<double>::size_type k=0;(int)k<pauseChannel;++k){
                    if(scanFound[k]){
                        pauseChannel=(int)k;
                        //fprintf(stderr,"Select channel %d\n",pauseChannel);
                        rx->f=scanFrequencies[k];
                        //fprintf(stderr,"k %d  2 Frequency Selected %.4f\n",(int)k,scanFrequencies[k]/1e6);
                        setFrequency3(rx);
                        goto FoundTime;
                    }
                }
                pauseChannel=lastChannel;
            }else{
                rx->muteScan = 1;
            }
        }
    }
FoundTime:
    long ns,ne,nsub;
    nsub=length/20;
    ns=nf-nsub;
    if(ns < 0)ns=0;
    ne=nf+nsub;
    if(ne >= length){
        ne=length-1-nf+nsub;
    }else{
        ne=2*nsub+1;
    }
    
    if(FindScene(scenel2))lines2->plotPutData(scenel2,&range[ns],&magnitude2[ns],ne,0L);

    if(water.data == NULL)return 0;
    
    pd.dmin=amin;
    pd.dmax=amax;
    
    
    setDialogRange(amin,amax);
        
    amin=water.amin;
    amax=water.amax;
    
    if(water.nline >= water.ysize)water.nline=0;
    
    
    // fprintf(stderr,"water length %ld\n",length);
    

    long n1=fftIndex(rx->f-0.5*rx->bw);
    if(n1 < 0)n1=nf-5;
    long n2=fftIndex(rx->f+0.5*rx->bw);
    if(n2 < 0)n2=nf+5;

    double meterMax=magnitude2[nf];
    int nmin,nmax;
    nmin=length-1;
    nmax=0;
    for(int n=0;n<length;++n){
        if(range[n] <= rmin)nmin=n;
        if(range[n] <= rmax)nmax=n;
        if(n >= n1 && n <= n2){
            if(magnitude2[n] > meterMax)meterMax=magnitude2[n];
        }
    }
    
    // fprintf(stderr,"nf %ld mag %g meterMax %g\n",nf,magnitude2[nf],meterMax);
    
    setDialogPower(meterMax);
    
    rx->meterMax=meterMax;
    
    unsigned char *wateric=(unsigned char *)range;
    
    FloatToImage(magnitude2,length,&pd,wateric);
    
    int ns1=3*water.xsize*(water.ysize-water.nline-1);
    int ns2=3*water.xsize*water.ysize+3*water.xsize*(water.ysize-1-water.nline++);

    double dxn = -1;
    if(nmax-nmin){
        dxn=(double)(nmax-nmin)/(double)(length-1);
    }else{
        nmin=0;
        dxn = 1;
    }
    
    double dxw=(double)(water.xsize-1)/(double)(length-1);
    
//    static long count=1;
    
//     fprintf(stderr,"nmin %d nmax %d dxn %g rmin %g rmax %g length %d count %ld\n",nmin,nmax,dxn,rmin,rmax,length,count++);
    
    
//        fprintf(stderr,"water.xsize %d water.ysize %d nmin %d nmax %d dxw %g\n",water.xsize,water.ysize,nmin,nmax,dxw);
    
    int ics=wateric[(int)(2*dxn+nmin)];
    
    for(int nnn=2;nnn<length-2;++nnn){
        int ic;
        
        int n=nnn*dxn+nmin+0.5;
        
        int next=(nnn+1)*dxn+nmin+0.5;
        
        ic=wateric[n];
 
        int nn=nnn*dxw+0.5;
        
        int nn2=next*dxw+0.5;

//            fprintf(stderr,"nn %d nn2 %d nnn %d n %d next %d ic %d ics %d\n",nn,nn2,nnn,n,next,ic,ics);

        if(ic > ics)ics=ic;
        
        if(nn == nn2)continue;
        
        ic=ics;
        
        ics=wateric[next];
        
        
        water.data[ns1+3*nn]=pd.palette[3*ic];
        water.data[ns1+3*nn+1]=pd.palette[3*ic+1];
        water.data[ns1+3*nn+2]=pd.palette[3*ic+2];
        
        water.data[ns2+3*nn]=pd.palette[3*ic];
        water.data[ns2+3*nn+1]=pd.palette[3*ic+1];
        water.data[ns2+3*nn+2]=pd.palette[3*ic+2];
        
    }
 
    InvalRectMyWindow(scene);
    
    return 0;
}

int Radio::UpdateTemperatureColors(struct Scene *scene)
{
    if(!scene)return 1;
    
    for(int n=0;n<768;++n){
        pd.palette[n]=g.palette[n];
    }
    
    return 0;
}

int Radio::BackGroundEvents(struct Scene *scene)
{
    if(!scene || !backGroundEvents)return 1;
    
    if(!rx)return 0;
    
    if(reset.reset){
        
        if(reset.frequency != rx->f){
            float fl;
            fl=rx->f=reset.frequency;
            if(fabs(fl-rx->fc) > 0.5*rx->samplerate){
                rx->fc=fl;
               // fprintf(stderr,"shift f %g fc %g\n",rx->f,rx->fc);
                setFrequency2(rx);
                // fprintf(stderr,"setFrequency2\n");
            }else{
              //  fprintf(stderr,"no shift f %g fc %g\n",rx->f,rx->fc);
                setDialogFrequency(rx->f);
                
                setFrequency(rx->f);
                
                setDialogFc(rx->fc);
                
                setFrequencyCoefficients(rx);
                
                if(FindScene(scenel2)){
                    SetFrequencyScene(scenel2, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
                }
                
                if(FindScene(scenel)){
                    SetFrequencyScene(scenel, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
                }
            }
            
        }
        
        if(reset.decodemode != rx->decodemode){
            setMode(reset.decodemode);
        }
        reset.reset=0;
    }
    
    if(rx->matchFrequencies){
        //fprintf(stderr,"matchFrequencies this %p gf.broadCastFrequency %d\n",this,gf.broadCastFrequency);
        if(gf.broadCastFrequency)RadioWindowSetFrequency(rx);
        rx->matchFrequencies=0;
    }
        
    updateLine();
    
    doSoundRecord();
    
    return 0;
}
int Radio::sendMessage(char *m1,char *m2,int type)
{
    char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};

    if(type == M_SEND){
        if(!rx)return 0;
       // fprintf(stderr,"Radio m1 %s m2 %s type %d\n",m1,m2,type);
        for(unsigned int k=0;k<sizeof(Mode_Names)/sizeof(char *);++k){
            if(!strcmp(m2,Mode_Names[k]))type=k;
        }
        
        double ff=atof(m1)*1e6;
        
        //fprintf(stderr,"f %g mode %s\n",ff,Mode_Names[type]);
        
       int wnd=glutGetWindow();
        
       glutSetWindow(window1);
        
       if(type != rx->decodemode){
            //fprintf(stderr,"type = %d\n",type);
            setMode(type);
        }
        
        rx->f=ff;
        
        if(fabs(rx->fc-ff) > 0.5*rx->samplerate){
            rx->fc=ff+rx->bw;
        }

        setFrequency2(rx);
        
        glutSetWindow(wnd);
    }else if(type == M_SEND2){
        if(!rx)return 0;
        // fprintf(stderr,"Radio m1 %s m2 %s type %d\n",m1,m2,type);
        for(unsigned int k=0;k<sizeof(Mode_Names)/sizeof(char *);++k){
            if(!strcmp(m2,Mode_Names[k]))type=k;
        }
        
        double ff=atof(m1)*1e6;
        
        //fprintf(stderr,"f %g mode %s\n",ff,Mode_Names[type]);
        
        int wnd=glutGetWindow();
        
        glutSetWindow(window1);
        
        if(type != rx->decodemode){
            //fprintf(stderr,"type = %d\n",type);
            setMode(type);
        }
        
        rx->f=ff;
        rx->fc=ff;

        setFrequency2(rx);
        
        glutSetWindow(wnd);

    }else if(type == M_FREQUENCY_SCAN){
        if(!strcmp(m1,"0")){
            //fprintf(stderr,"Start scan\n");
            scanFrequencies.clear();
            scanWait=1;
            return 0;
        }else if(!strcmp(m1,"-1")){
            //fprintf(stderr,"Stop scan scanCount %d\n",scanCount);
            scanWait=0;
            pauseChannel=0;
           return 0;
        }
        //fprintf(stderr,"Radio m1 %s m2 %s type %d\n",m1,m2,type);
        scanFrequencies.push_back(1.0e6*atof(m1));
   }
    
    return 0;
}

int Radio::SetFrequency(struct Scene *scene,double f,double bw, int message)
{
    char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};

    static int count;
    
    if(!rx)return 0;

    if(message == M_MUTE){
        rx->mute = !rx->mute;
        return 0;
    }else if(message == M_SAVE){
        char buff[256];
        msprintf(buff,sizeof(buff),"F%d,%0.4f,%s\n",count++,rx->f/1e6,Mode_Names[rx->decodemode]);
        WriteToWindow(buff);
        //WarningPrint("F%d,%0.4f,%s\n",count++,rx->f/1e6,Mode_Names[rx->decodemode]);
        return 0;
    }else if(message == M_SCAN){
        if(scanWait == 0){
            scanWait=1;
        }else{
            scanWait=0;
        }
        return 0;
   } else if(message == M_FREQUENCY){
        
        rx->f=f;
        if(fabs(f-rx->fc) > 0.5*rx->samplerate){
            rx->fc=f;
            setFrequency2(rx);
           // fprintf(stderr,"M_FREQUENCY setFrequency2\n");
            
            rx->matchFrequencies=1;

            return 0;
       }
        setFrequencyCoefficients(rx);
       //fprintf(stderr,"M_FREQUENCY setFrequencyCoefficients\n");
        rx->matchFrequencies=1;
        return 0;
    }
    
    rx->f=f;
    
    fdown=f;
    
    fcdown=rx->fc;
    
    fcount=0;
    
    setFrequency2(rx);
    
    return 0;
}
int Radio::setFrequencyCoefficients(struct playData *rx)
{
    float pi;
    
    if(!rx)return 0;

    pi=4.0*atan(1.0);
    rx->dt=1.0/(double)rx->samplerate;
    rx->sino=0;
    rx->coso=1;
    rx->w=2.0*pi*(rx->fc - rx->f);
    rx->sindt=sin(rx->w*rx->dt);
    rx->cosdt=cos(rx->w*rx->dt);

    return 0;
}

int Radio::setFrequency3(struct playData *rx)
{
    
    if(!rx)return 0;

    setDialogFrequency(rx->f);
    
    setFrequency(rx->f);
    
    setDialogFc(rx->fc);
    
    setFrequencyCoefficients(rx);
    
    rx->aminGlobal=0;
    
    rx->amaxGlobal=0;
    
    rx->aminGlobal2=0;
    
    rx->amaxGlobal2=0;
    
    rx->aminGlobal3=0;
    
    rx->amaxGlobal3=0;
    
    rx->averageGlobal=0;
        
    if(FindScene(scenel2)){
        SetFrequencyScene(scenel2, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
    }
    
    if(FindScene(scenel)){
        SetFrequencyScene(scenel, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
    }

    return 0;
}
int Radio::setFrequency2(struct playData *rx)
{
    
    if(!rx)return 0;
    
    try {
        
        if(rx->device){
            if(rx->fc < 0.0)rx->fc=rx->f;
            //fprintf(stderr," Radio::setFrequency %g\n  ",rx->fc);
            setFrequencyDuo(rx);
        }
        
        setFrequency3(rx);
        
        adjustView(0);
        
    }
    catch (...)
    {
        fprintf(stderr, "exception Radio::setFrequency\n");
    }

    return 0;
}
int Radio::stopPlay(struct playData4 *rx)
{
    
    return 0;
}
int Radio::SetWindow(struct Scene *scene)
{
    
    CheckSceneBuffer(scene);
    
    int xsize=(int)scene->xResolution;
    
    int ysize=(int)scene->yResolution/2;
    
    water.DRect.x=0;
    water.DRect.y=0;
    water.DRect.xsize=xsize;
    water.DRect.ysize=ysize;
    
    //xsize=(int)rx->FFTcount;
    
    if(ysize == water.ysize && xsize == water.xsize && water.data)return 0;
    
    water.amin=1e60;
    water.amax=-1e60;
    
    water.SRect.x=0;
    water.SRect.y=0;
    water.SRect.xsize=xsize;
    water.SRect.ysize=ysize;
    
    water.nline=0;
    
    if(water.data)cFree((char *)water.data);
    
    water.data=(unsigned char *)cMalloc(2*xsize*ysize*3,9999);
    
    if(!water.data)return 1;
    
    water.ysize=ysize;
    water.xsize=xsize;
    
    for(int y=0;y<ysize*2;++y)
    {
        for(int x=0;x<xsize;++x){
            int nn=y*(xsize*3)+x*3;
            water.data[nn]=255;
            water.data[nn+1]=255;
            water.data[nn+2]=255;
        }
    }
    
    return 0;
}

static int flip(unsigned char *b,int xsize,int ysize)
{
    unsigned char *p1,*p2,c1,c2,c3;
    int x,y,xinc;
    
    if(!b)return 1;
    
    xinc=xsize*3;
    
    for(y=0;y<ysize/2;++y){
        p1=b+y*xinc;
        p2=b+(ysize-1-y)*xinc;
        for(x=0;x<xsize;++x){
            c1 = *p1;
            c2 = *(p1+1);
            c3 = *(p1+2);
            *p1++ = *(p2);
            *p1++ = *(p2+1);
            *p1++ = *(p2+2);
            *p2++ = c1;
            *p2++ = c2;
            *p2++ = c3;
        }
    }
    
    if(ysize & 1){
        y=ysize/2;
        p1=b+y*xinc;
        for(x=0;x<xsize;++x){
            c1 = *p1;
            c2 = *(p1+1);
            c3 = *(p1+2);
            *p1++ = c1;
            *p1++ = c2;
            *p1++ = c3;
        }
        
    }
    return 0;
}

int Radio::Display(struct Scene *scene)
{
    if(!scene)return 1;
    
    //cout<<std::this_thread::get_id()<<endl;
    
    Radio::SetWindow(scene);
    
    water.SRect.y=water.ysize+1-water.nline;
    
    // printf("water.nline %d water.ysize %d\n",water.nline,water.ysize);
    
    // printf("swater.xs %d swater.ys %d dwater.xs %d dwater.ys %d\n",(int)water.xsize,(int)water.ysize*2,(int)scene->xResolution,(int)scene->yResolution);

    WriteRect24ToRect24(water.data, &water.SRect,water.xsize, water.ysize*2,
                        scene->dpp,&water.DRect,scene->xResolution,scene->yResolution/2);
    
    // printf("x %d y %d %d %d\n",water.DRect.x,water.DRect.y,water.DRect.xsize,water.DRect.ysize);
    
    
    flip(scene->dpp,(int)scene->xResolution,(int)scene->yResolution/2);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glRasterPos2i(0,0);		// Position at base of window
    
    if(scene->dpp != NULL){
        glDrawPixels( (int)scene->xResolution, (int)scene->yResolution/2, GL_RGB, GL_UNSIGNED_BYTE, scene->dpp);
    }
    
    /* DrawPalette(scene,(int)scene->xResolution,(int)scene->yResolution,0); */
    
    glRasterPos2i(0,0);		// Position at base of window
    
    int nf=(int)(0.5+(scene->xResolution)*((rx->f-rx->rmin))/(rx->rmax-rx->rmin));
    
    int nf1=(int)(0.5+(scene->xResolution)*((rx->f-rx->rmin)+0.5*rx->bw)/(rx->rmax-rx->rmin));

    int nf2=(int)(-0.5+(scene->xResolution)*((rx->f-rx->rmin)-0.5*rx->bw)/(rx->rmax-rx->rmin));
    
    if((nf1-nf2) < 6){
        nf1=nf+3;
        nf2=nf-3;
    }
    
    if(rx->wShift > 0){
        nf2=nf;
    }else if(rx->wShift < 0){
        nf1=nf;
    }
    
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, w, 0, h/2);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        glRasterPos2i(0,0);		/* Position at base of window */
        
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
        
        DrawLine(nf1, 0, nf1, (int)scene->yResolution/2);
        
        DrawLine(nf2, 0, nf2, (int)scene->yResolution/2);
        
        glDisable(GL_COLOR_LOGIC_OP);
    
    }

    
    glutSwapBuffers();
    
    return 0;
}

static void myReshape(int w, int h)
{
    struct SceneList *list;
    struct Scene *scene;
    list=SceneFindByNumber(glutGetWindow());
    
    if(!list)return;
    scene=&list->scene;
    
    reshape(scene,w,h);
    
    glutSetWindow(list->window);
}

static void reshape(struct Scene *scene,int wscr,int hscr)
{
    double xmin,ymin,xmax,ymax;
    
    RadioPtr s=(RadioPtr)FindScene(scene);
    if(!s)return;

    s->w=wscr; s->h=hscr;
    glViewport(0,0,(GLsizei)s->w,(GLsizei)s->h);
    
    /* WarningPrint("reshape %d %d\n",w,h); */
    
    // printf("Csdr reshape wscr %d hscr %d\n",wscr,hscr);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    xmin=ymin=0.0; xmax=ymax=1.0;
    if(s->w<=s->h){
        ymax=s->h*(GLfloat)s->h/(GLfloat)s->w;
        xmax=s->w;
    }else{
        xmax=s->w*(GLfloat)s->w/(GLfloat)s->h;
        ymax=s->h;
    }
    
    gluOrtho2D(xmin,xmax,ymin,ymax);
    glMatrixMode(GL_MODELVIEW);
    
    //glLoadIdentity();
    
    scene->xResolution=s->w;
    
    scene->yResolution=s->h;
    
    glutSetWindow(s->window1);
    glutPositionWindow(0,s->h/2);
    glutReshapeWindow(s->w, s->h/2);
    
    glutSetWindow(s->window2);
    glutPositionWindow(0,20);
    glutReshapeWindow(s->w, s->h/2-20);
    
  //  printf("s->window1 %d s->window2 %d s->window3 %d\n",s->window1,s->window2,s->window3);
}

static void displayc(void)
{
    struct SceneList *list;
    struct Scene *scene;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        return;
    }
    scene=&list->scene;
    
    RadioPtr images=(RadioPtr)FindScene(scene);
    if(!images)return;
    
    //fprintf(stderr,"glut %d win1 %d win %d\n",glutGetWindow(),images->window1,list->window);
    
    glutSetWindow(images->window1);
    
    images->Display(scene);
    
    glutSetWindow(list->window);
    
    //glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glClearColor(0.0, 0.0, 1.0, 0.0);

    glRasterPos2i(0,0);
    
    glColor3f(1.0,1.0,1.0);
    
    unsigned long f,fc;
    double fs;
    
    f=(unsigned long)images->rx->f;
    
    fc=(unsigned long)images->rx->fc;
    
    fs=images->rx->shiftGlobal;

    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, images->w, 0, images->h);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        glRasterPos2i(0,0);        /* Position at base of window */
        
        char value[256];
        
        if(images->rx->mute){
            msprintf(value,sizeof(value),"Frequency: %010ld Hz   Center Frequency: %010ld Hz Power: %.0f db Avg: %.0f db MUTE",f,fc,images->rx->meterMax,fs);
        }else{
            msprintf(value,sizeof(value),"Frequency: %010ld Hz   Center Frequency: %010ld Hz Power: %.0f db Avg: %.0f db ",f,fc,images->rx->meterMax,fs);
        }
        
        DrawString(20, (int)scene->yResolution-15, value);
        
        DrawBox(&images->box,(int)scene->yResolution-20);
        
    }

    glutSwapBuffers();

}
int DrawBox(uRect *box,int offset)
{
    if(box->xsize <= 0)return 0;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND); //Enable blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function.

    
    glColor4f(1.0, 1.0, 1.0, 0.75);
    glBegin(GL_QUADS);
    glVertex2f(box->x, box->y+offset);
    glVertex2f(box->x+box->xsize, box->y+offset);
    glVertex2f(box->x+box->xsize, box->y-box->ysize+offset);
    glVertex2f(box->x, box->y-box->ysize+offset);
    glEnd();
    
    glDisable(GL_BLEND);
    
    //static int count=0;
    
    //printf("count %d DrawBox %d %d %d %d\n",count++,box->x,box->x+box->xsize,box->y+offset,box->y-box->ysize+offset);
    
    //DrawLine(box->x, box->y+offset, box->x+box->xsize, box->y-box->ysize+offset);

    
    return 0;
}

int Radio::OpenWindows(struct Scene *scene)
{
    struct SceneList *list;
    
    struct Scene *sceneOpen=scene;
    
    if(!scene)return 1;
    
    list=SceneFindScene(scene);
    if(list == NULL)
    {
        WarningPrint("FMRadio::OpenWindows : Error Could Not Find Scene\n");
        return 1;
    }
    
    scene->ThreadCount=8;

    if( scene->stereoType == StereoDoubleBuffered){
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO);
    }else if(scene->stereoType == StereoSingleBuffered){
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH  | GLUT_STEREO);
    }else{
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    }
    
  //  printf("this %p\n",this);
    
    glutInitWindowSize(960,340);
    window = glutCreateWindow(scene->WindowTitle);
    list->window=window;
    glutReshapeFunc(myReshape);
    glutKeyboardFunc(keys2);
    glutSpecialFunc( keyss );
    glutMouseFunc(getMousel);
    glutMotionFunc(moveMouse);
    glutPassiveMotionFunc(getMousePassive);
#ifdef USE_JOYSTICK
    glutJoystickFunc(JoyStick, 20);
#endif
    glutDisplayFunc(displayc);
    
    glutSetCursor( GLUT_CURSOR_CROSSHAIR );
    
    window1 = glutCreateSubWindow(window,0,170,960,170);
    glutMouseFunc(getMousel);
    glutMotionFunc(moveMouse);
    glutDisplayFunc(displayc);
    glutKeyboardFunc(keys2);
    glutPassiveMotionFunc(getMousePassive3);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glClearColor(1.0,1.0,1.0,0.);
    
    SetLighting(3);
    
    glEnable(GL_LIGHTING);
    
    glShadeModel(GL_SMOOTH);
    
    
    int palette_menu=glutCreateMenu(palette_select2);
    
    glutAddMenuEntry("Apricot Blue->Pink", 0);
    glutAddMenuEntry("Carnation Red->White", 1);
    glutAddMenuEntry("Ether Blue->Yellow", 2);
    glutAddMenuEntry("GrayScale", 3);
    glutAddMenuEntry("GrayScale-Banded", 4);
    glutAddMenuEntry("GrayScale-Inverted", 5);
    glutAddMenuEntry("Hot Metal", 6);
    glutAddMenuEntry("Lava Waves", 7);
    glutAddMenuEntry("Macintosh System Table", 8);
    glutAddMenuEntry("Malachite Green->Blue", 9);
    glutAddMenuEntry("Morning Glory Blue->Tan", 10);
    glutAddMenuEntry("PeanutButter&Jelly", 11);
    glutAddMenuEntry("Ps", 12);
    glutAddMenuEntry("Purple Haze", 13);
    glutAddMenuEntry("Rainbow", 14);
    glutAddMenuEntry("Rainbow-Banded", 15);
    glutAddMenuEntry("Rainbow-High Black", 16);
    glutAddMenuEntry("Rainbow-Inverted", 17);
    glutAddMenuEntry("Rainbow-Low Black", 18);
    glutAddMenuEntry("Rainbow-Striped", 19);
    glutAddMenuEntry("Saturn PastelYellow->Purple", 20);
    glutAddMenuEntry("Seismic Blue->White->Red", 21);
    glutAddMenuEntry("Space Black->Purple->Yellow", 22);
    glutAddMenuEntry("Supernova Green->Purple->Yellow", 23);
    glutAddMenuEntry("System", 24);
    glutAddMenuEntry("Green-White", 27);

    
    int menu3=glutCreateMenu(setMode);
    glutAddMenuEntry("AM", MODE_AM);
    glutAddMenuEntry("NAM", MODE_NAM);
    glutAddMenuEntry("FM", MODE_FM);
    glutAddMenuEntry("NBFM", MODE_NBFM);
    glutAddMenuEntry("USB", MODE_USB);
    glutAddMenuEntry("LSB", MODE_LSB);
    glutAddMenuEntry("CW", MODE_CW);

    int antenna=0;
    if(rx->antennaCount > 0){
        antenna=glutCreateMenu(antennaMenu);
        for(size_t i=0;i<rx->antennaCount;++i){
            glutAddMenuEntry(rx->antenna[i], (int)i);
        }
    }
    
    int bandwidth=0;
    if(rx->bandwidthsCount > 0){
        bandwidth=glutCreateMenu(bandMenu);
        for(size_t i=0;i<rx->bandwidthsCount;++i){
            char number[256];
            sprintf(number,"%0.f",rx->bandwidths[i]);
            glutAddMenuEntry(number, (int)i);
        }
        
    }
    
    int samplerate=0;
    if(rx->sampleRatesCount > 0){
        samplerate=glutCreateMenu(sampleMenu);
        for(size_t i=0;i<rx->sampleRatesCount;++i){
            char number[256];
            sprintf(number,"%0.f",rx->sampleRates[i]);
            glutAddMenuEntry(number, (int)i);
        }
        
    }

    int menu4=glutCreateMenu(doAudio);
    glutAddMenuEntry("Audio Recording", START_AUDiO);
    glutAddMenuEntry("IQ Recording", START_IQ);
    
    int menu5=glutCreateMenu(doFFTMenu);
    glutAddMenuEntry("1024", FFT_1024);
    glutAddMenuEntry("2048", FFT_2048);
    glutAddMenuEntry("4096", FFT_4096);
    glutAddMenuEntry("8192", FFT_8192);
    glutAddMenuEntry("16384", FFT_16384);
    glutAddMenuEntry("32768", FFT_32768);
    glutAddMenuEntry("65536", FFT_65536);
    glutAddMenuEntry("131072", FFT_131072);
    
    
    int menu6=glutCreateMenu(doFilterMenu);
    glutAddMenuEntry("RECTANGULAR", FILTER_RECTANGULAR);
    glutAddMenuEntry("HANN", FILTER_HANN);
    glutAddMenuEntry("HAMMING", FILTER_HAMMING);
    glutAddMenuEntry("FLATTOP", FILTER_FLATTOP);
    glutAddMenuEntry("BLACKMANHARRIS", FILTER_BLACKMANHARRIS);
    glutAddMenuEntry("BLACKMANHARRIS7", FILTER_BLACKMANHARRIS7);
    
    int menu62=-1;
    if(rx->directSampleMode){
        menu62=glutCreateMenu(doDirectSampleMode);
        glutAddMenuEntry("Off", FFT_1024);
       // glutAddMenuEntry("?", FFT_2048);
        glutAddMenuEntry("On", FFT_4096);
    }
 
    int menu63=-1;

    if(rx->biasMode!= ""){
        menu63=glutCreateMenu(doBiasMode);
        glutAddMenuEntry("Off", FFT_1024);
        // glutAddMenuEntry("?", FFT_2048);
        glutAddMenuEntry("On", FFT_4096);
    }
    
    int menu64=-1;
    flags = rx->device->getSettingInfo();
    if (flags.size()) {
        flagsmenu=new int[flags.size()];
        int count=0;
        for(size_t k=0;k<flags.size();++k){
            fprintf(stderr,"k %d %s %s type %d\n",(int)k,flags[k].key.c_str(),flags[k].value.c_str(),(int)flags[k].type);
            if(flags[k].type == flags[k].BOOL){
                ++count;
                flagsmenu[k]=glutCreateMenu(doBiasMode);
                if(rx->device->readSetting(flags[k].key) == "true"){
                    glutAddMenuEntry("true",(int)(5000+2*k));
                    glutAddMenuEntry("",(int)(5000+2*k+1));
                }else{
                    glutAddMenuEntry("",(int)(5000+2*k));
                    glutAddMenuEntry("false",(int)(5000+2*k+1));
                }
            }
        }
        if(count > 0){
            menu64=glutCreateMenu(doBiasMode);
            for(size_t k=0;k<flags.size();++k){
                if(flags[k].type == flags[k].BOOL){
                    glutAddSubMenu(flags[k].key.c_str(), flagsmenu[k]);
                }
            }
            flagsflag=1;
        }
   }
    
    glutEntryFunc(enterexit);
    
    glutMenuStateFunc(inuse);
    
    glutCreateMenu(menu_selectl);
    
    glutAddMenuEntry("Scan Frequency File...", SdrReadFile);
    glutAddMenuEntry("VoiceControl...", SdrVoiceControl);
    glutAddMenuEntry("SDR Dialog...", SdrDialog);
    if(rx->ntransmit)glutAddMenuEntry("Transmit...", SdrTransmit);
    glutAddMenuEntry("Send...", SdrSendIQ);
    glutAddMenuEntry("--------------------", -1);
    glutAddSubMenu("Palette", palette_menu);
    glutAddSubMenu("Mode", menu3);
    if(rx->antennaCount > 0)glutAddSubMenu("Antenna", antenna);
    if(rx->bandwidthsCount > 0)glutAddSubMenu("Bandwidth", bandwidth);
    if(rx->sampleRatesCount > 0)glutAddSubMenu("SampleRate", samplerate);
    glutAddSubMenu("Recording", menu4);
    glutAddSubMenu("FFT Size", menu5);
    glutAddSubMenu("Window Filter", menu6);
    if(rx->directSampleMode)glutAddSubMenu("Direct Sample Mode", menu62);
    if(rx->biasMode!= "")glutAddSubMenu("Voltage Bias", menu63);
    if(flags.size() > 0)glutAddSubMenu("Options", menu64);


    glutAddMenuEntry("--------------------", -1);
    glutAddMenuEntry("Close", ControlClose);
    glutAddMenuEntry("Quit", ControlQuit);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    
    list=SceneNext();
    if(list == NULL)
    {
        WarningPrint("FMRadio : Error Allocation Scene Memory File\n");
        return 0;
    }
    
    //printf("1 list->window %d\n",list->window);
    
    scenel=&list->scene;
    
    zerol((char *)scenel,sizeof(struct Scene));
    
    initScene(scenel);
    

    lines = CLines::CLinesOpen(scenel,window);
    

    lines->plotPutData(scenel,range,magnitude2,rx->FFTcount,-1L);
    
    lines->sceneSource=sceneOpen;
    
    lines->Frequency=rx->f;
    
    lines->BandWidth=rx->bw;

    lines->wShift=0;
    
//    lines->sdr=NULL;
    
    window2=list->window;
    
    lines->lines->Plot->yLogScale=0;
    
    lines->lines->Plot->gridHash=1;
    lines->lines->Plot->yAutoMaximum=FALSE;
    lines->lines->Plot->yAutoMinimum=FALSE;
    lines->lines->Plot->ySetMaximum=-20;
    lines->lines->Plot->ySetMinimum=-130;
    lines->lines->Plot->yMajorStep=20;
    if(OpenZoomWindow){
        list=SceneNext();
        if(list == NULL)
        {
            WarningPrint("FMRadio : Error Allocation Scene Memory File\n");
            return 0;
        }
        
        scenel2=&list->scene;
        
        zerol((char *)scenel2,sizeof(struct Scene));
        
        initScene(scenel2);
        
        lines2 = CLines::CLinesOpen(scenel2,-1000);
        
        lines2->plotPutData(scenel2,range,magnitude2,rx->FFTcount,-1L);
        
        lines2->sceneSource=sceneOpen;
        
        lines2->wShift=0;

        lines2->lines->Plot->yLogScale=0;
        
        lines2->lines->Plot->gridHash=1;
        
        lines2->Frequency=rx->f;
        
        lines2->BandWidth=rx->bw;
        
        window3=list->window;

        glutSetWindow(list->window);
        glutPositionWindow(20,444);
        glutReshapeWindow(600, 200);
    }
    return 0;
}
void antennaMenu(int item){
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    //fprintf(stderr,"setAntenna '%s'\n",sdr->rx->antenna[item]);
    
    sdr->rx->device->setAntenna(SOAPY_SDR_RX, sdr->rx->channel, sdr->rx->antenna[item]);
    
    return;
}
void bandMenu(int item){
    RadioPtr sdr;
    
    sdr=FindSdrRadioWindow(glutGetWindow());
    
    if(!sdr)return;

    //printf("item %d bandwidths %.0f\n",item,sdrOpen->rx->bandwidths[item]);
    
    sdr->rx->bandwidth=sdr->rx->bandwidths[item];
    
    RadioPtr f=sdr->findMate(sdr->rx);
    if(f){
        f->rx->bandwidth=sdr->rx->bandwidths[item];
    }
    
    sdr->stopPlay(sdr->rx);
    
    sdr->setDialogBandWidth(sdr->rx->bandwidth);
    
    if(f){
        f->setDialogBandWidth(sdr->rx->bandwidth);
    }

    sdr->startPlay(sdr->rx);
    
    sdr->playRadio(sdr->rx);

    return;
}
void sampleMenu(int item){
    
    RadioPtr sdr;
    
    sdr=FindSdrRadioWindow(glutGetWindow());
    
    if(!sdr)return;

    double sameleRate=sdr->rx->sampleRates[item];
 
    sdr->stopPlay(sdr->rx);
    
    sdr->rx->samplerate=sameleRate;
    
    sdr->setDialogSampleRate(sameleRate);
    
    RadioPtr f=sdr->findMate(sdr->rx);
    if(f){
        f->rx->samplerate=sameleRate;
        f->setDialogSampleRate(sameleRate);
    }

    sdr->startPlay(sdr->rx);
    
    sdr->playRadio(sdr->rx);

    return;
}
void setMode(int item)
{
    struct SceneList *list;
    RadioPtr sdr;
    
    //fprintf(stderr,"1 setMode %d\n",item);
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    //fprintf(stderr,"2 setMode %d\n",item);

    sdr->lines->wShift = 0;
    if(sdr->lines2)sdr->lines2->wShift = 0;
    sdr->rx->wShift = 0;
    
    switch(item){
        case MODE_AM:
            sdr->rx->decodemode = MODE_AM;
            sdr->rx->bw = 10000;
            break;
        case MODE_NAM:
            sdr->rx->decodemode = MODE_NAM;
            sdr->rx->bw = 5000;
          break;
        case MODE_USB:
            sdr->lines->wShift = 1;
            if(sdr->lines2)sdr->lines2->wShift = 1;
            sdr->rx->wShift = 1;
            sdr->rx->decodemode = MODE_USB;
            sdr->rx->bw = 6000;
            break;
        case MODE_LSB:
            sdr->lines->wShift  = -1;
            if(sdr->lines2)sdr->lines2->wShift = -1;
            sdr->rx->wShift = -1;
            sdr->rx->decodemode = MODE_LSB;
            sdr->rx->bw = 6000;
            break;
        case MODE_CW:
            sdr->lines->wShift  = -1;
            if(sdr->lines2)sdr->lines2->wShift = -1;
            sdr->rx->wShift = -1;
            sdr->rx->decodemode = MODE_CW;
            sdr->rx->bw = 500;
            break;
        case MODE_FM:
            sdr->rx->decodemode = MODE_FM;
            sdr->rx->bw = 200000;
             break;
        case MODE_NBFM:
            sdr->rx->decodemode = MODE_NBFM;
            sdr->rx->bw = 12500;
           break;
    }
    sdr->resetDemod();

}
void AudioSave(struct Scene *scene,char *name)
{
    if(!scene || !name)return;
    
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;

   // sdr->rx->pSetAudio(sdr->rx,name,START_AUDiO);
    
    mstrncpy(scene->FilePathIQ,name,sizeof(*scene->FilePathIQ));
    
    return;
}
void doFilterMenu(int item)
{
    int ifft = -1;
    
    switch(item){
        case FILTER_RECTANGULAR:
        case FILTER_HANN:
        case FILTER_HAMMING:
        case FILTER_FLATTOP:
        case FILTER_BLACKMANHARRIS:
        case FILTER_BLACKMANHARRIS7:
            ifft=item;
            break;
    }
    
    if(ifft == -1)return;
    
    struct SceneList *list;
    RadioPtr sdr;

    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    sdr->rx->FFTfilter=ifft;

}



void doBiasMode(int item)
{
    
    std::string value;
    
    
    switch(item){
        case FFT_1024:
            value="false";
            break;
        case FFT_4096:
            value="true";
            break;
    }
    
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    if(item > 4999){
        int k=(item-5000)/2;
        if(item & 1){
            value="false";
        }else{
            value="true";
        }
        fprintf(stderr,"doBiasMode item %d key %s value %s\n",item,sdr->flags[k].key.c_str(),value.c_str());
        sdr->rx->device->writeSetting(sdr->flags[k].key,value);
        sdr->flagsflag=1;
    }else{
       sdr->rx->device->writeSetting(sdr->rx->biasMode,value);
    }
    
    // sdr->rx->FFTcount=ifft;
    
    return;
}

void doDirectSampleMode(int item)
{
    
    std::string value;
    
    switch(item){
        case FFT_1024:
            value="0";
            break;
        case FFT_2048:
            value="1";
            break;
        case FFT_4096:
            value="2";
            break;
    }

    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    SoapySDR::ArgInfo arg;
    
    sdr->rx->device->writeSetting("direct_samp",value);
    
   // sdr->rx->FFTcount=ifft;

    return;
}
void doFFTMenu(int item)
{
    int ifft=0;
    
    switch(item){
        case FFT_1024:
        case FFT_2048:
        case FFT_4096:
        case FFT_8192:
        case FFT_16384:
        case FFT_32768:
        case FFT_65536:
        case FFT_131072:
            ifft=item;
           break;
    }
    
    if(ifft == 0)return;
    
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    sdr->rx->FFTcount=ifft;
}
void doAudio(int item)
{
    // unsigned long freq,samp;
    // char name[256];
    
    fprintf(stderr,"setAudio item %d\n",item);
    
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;

    switch(item){
        case START_AUDiO:
           // dialogSaveC(sdr->scene, AudioSave,0,NULL);
            sdr->dialogTime();
            break;
        case START_IQ:
            RecordIQ();
            break;
    }
}
void palette_select2(int item)
{
    
    struct SceneList *list;
    RadioPtr sdr;

    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    fprintf(stderr,"palette_select2 %d sdr %p window %d list %p\n",item,sdr,glutGetWindow(),list);

    if(!sdr)return;
    
    getPalette(item,(char *)g.palname,(char *)g.palette);
    UpdateTemperatureColors(sdr->scene);
    glutPostRedisplay();
    
}

int Radio::resetDemod()
{
    
    RadioPtr myAppl=this;
        
    for(int y=0;y<myAppl->water.ysize*2;++y){
        int ns=3*myAppl->water.xsize*y;
        for(int n=0;n<myAppl->water.xsize;++n){
            myAppl->water.data[ns+3*n]=255;
            myAppl->water.data[ns+3*n+1]=255;
            myAppl->water.data[ns+3*n+2]=255;
        }
    }
    
    myAppl->rx->psdrSetMode(myAppl->rx);
        
    myAppl->water.nline=0;
        
    setFrequency(rx->f);
        
    if(FindScene(scenel2)){
        SetFrequencyScene(scenel2, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
    }
    
    if(FindScene(scenel)){
        SetFrequencyScene(scenel, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
    }
    
    adjustView(0);
    
    return 0;
}
int Radio::mMenuSelectl(struct Scene *scene,int item)
{
    
    if(!scene)return 1;

	switch (item)
	{
            
        case SdrReadFile:
            dialogFunctionPtr=this;
            dialogFileOpen((struct Scene *)NULL);
            return 0;

        case SdrDialog:
            dialogRadio(scene);
            return 0;
            
        case SdrTransmit:
            Transmit(scene);
            return 0;
                        
        case SdrSendIQ:
            dialogSendIQ(scene);
            return 0;
            
        case SdrVoiceControl:
            doVoiceControl(scene);
            return 0;

            

	case ControlGetSelectionBox:
		return 0;

	case ControlQuit:
		dialogQuitC();
		break;

	case ControlClose:

            RadioPtr sdr;
            sdr=(RadioPtr)FindScene(scene);
            sdr->closeScenes();
           
    }
    
    //menu_select(item);

    return 0;
}
static void keys2(unsigned char key, int x, int y)
{
    char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};

    struct SceneList *list;
    RadioPtr sdr;
    static int count=0;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(sdr){
        if(key == 'm'){
           sdr->rx->mute = !sdr->rx->mute;
        }else if(key == 's'){
            char buff[256];
            msprintf(buff,sizeof(buff),"F%d,%0.4f,%s\n",count++,sdr->rx->f/1e6,Mode_Names[sdr->rx->decodemode]);
            sdr->WriteToWindow(buff);

           // WarningPrint("F%d,%0.4f,%s\n",count++,sdr->rx->f/1e6,Mode_Names[sdr->rx->decodemode]);
        }else if(key == ' '){
            if(sdr->scanWait == 0){
                sdr->scanWait=1;
            }else{
                sdr->scanWait=0;
            }
        }else if(key == 't'){
            float aBuff[6000];
            for(int n=0;n<6000;++n)aBuff[n]=cos(3.1414*n/200.);
            double start=rtime();
            for(int n=0;n<1000;++n){
              //  doAudio2(aBuff,sdr->rx);
            }

            double end=rtime();
            
            printf("Total Time in processFile %.2f Seconds\n",end-start);
        }
    }
    
    fprintf(stderr,"Radio keys - key %d key %c \n",key,key);
    
}

static void getMousel(int button, int state, int x, int y)
{
    struct SceneList *list;
    RadioPtr sdr;
    
    //printf("button %d state %d\n",button,state);
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
        if(sdr){
            sdr->getMouse(button,state, x, y);
            return;
        }
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    

    if(state == GLUT_DOWN)
    {

        // printf("getMousel %d %d fc %f f %f \n",x,y,sdr->rx.fc,sdr->rx.f);
        
        int up;
        
        if(y > 10){
            up=0;
        }else{
            up=1;
        }
        
        long long fl;
        
        fl=(long long)sdr->rx->f;
        
        int start=119;
        
        for(int k=0;k<10;++k){
            if(x >= start+k*9 && x <= start+(k+1)*9){
                double value=pow(10.0,9-k);
                fl=(long long)fl/value;
                fl=(long long)fl*value;
                if(up == 1){
                    fl += value;
                }else{
                    fl -= value;
               }
                if(fl < 0)fl=-fl;
                sdr->rx->f=fl;
                if(fabs(fl-sdr->rx->fc) > 0.5*sdr->rx->samplerate){
                    sdr->rx->fc=fl;
                }
                sdr->setFrequency2(sdr->rx);
                sdr->rx->matchFrequencies=1;
                break;
            }
        }
        
        fl=(long long)sdr->rx->fc;
        
        start=425;
        
        for(int k=0;k<10;++k){
            if(x >= start+k*9 && x <= start+(k+1)*9){
                double value=pow(10.0,9-k);
                fl=(long long)fl/value;
                fl=(long long)fl*value;
                if(up == 1){
                    fl += value;
                }else{
                    fl -= value;
                }
                if(fl < 0)fl=-fl;
                sdr->rx->fc=fl;
                if(fabs(fl-sdr->rx->f) > 0.5*sdr->rx->samplerate){
                    sdr->rx->f=fl;
                }
                sdr->setFrequency2(sdr->rx);
           }
        }

    }
    //sdr->getMouse(button, state, x, y);
    
    
}

void Radio::adjustView(int button)
{
    
    double dx=(rx->samplerate*0.01);
    if(button == 3){
        rx->viewWindow -= dx;
    }else if( button == 4){
        rx->viewWindow += dx;
    }
    
    if(rx->viewWindow < dx)rx->viewWindow=dx;
    
    if(rx->viewWindow >= rx->samplerate){
        rx->viewWindow=rx->samplerate;
        rx->fv=rx->fc;
    }else{
       rx->fv=rx->f;
    }
    
    //printf("f %g samplerate %g fv %g viewWindow %g\n",rx->f,rx->samplerate,rx->fv,rx->viewWindow);
}


void Radio::getMouse(int button, int state, int x, int y)
{
    if(!rx)return;

    if(button == 3  && state == 0){
        double fl,bw;
        bw=rx->bw;
      //  if(rx->wShift != 0)bw /= 2;
        if(rx->wShift != 0)bw = 500;
        fl=rx->f-bw;
        if(fl < 0)fl=-fl;
        rx->f=fl;
        if(fabs(fl-rx->fc) > 0.5*rx->samplerate){
            rx->fc=fl;
            setFrequency2(rx);
           // fprintf(stderr,"setFrequency2\n");
        }else{
            setDialogFrequency(rx->f);
            
            setFrequency(rx->f);
            
            setDialogFc(rx->fc);

            setFrequencyCoefficients(rx);
            
            if(FindScene(scenel2)){
                SetFrequencyScene(scenel2, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
            }
            
            if(FindScene(scenel)){
                SetFrequencyScene(scenel, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
            }

            //fprintf(stderr,"setFrequencyCoefficients\n");
            rx->matchFrequencies=1;
            
       }
        adjustView(0);
        return;
    }else if(button == 4 && state == 0){
        double fl,bw;
        bw=rx->bw;
        //if(rx->wShift != 0)bw /= 2;
        if(rx->wShift != 0)bw=500;
        fl=rx->f+bw;
        if(fl < 0)fl=-fl;
        rx->f=fl;
        if(fabs(fl-rx->fc) > 0.5*rx->samplerate){
            rx->fc=fl;
            setFrequency2(rx);
            //fprintf(stderr,"setFrequency2\n");
       }else{
           setDialogFrequency(rx->f);
           
           setFrequency(rx->f);
           
           setDialogFc(rx->fc);

            setFrequencyCoefficients(rx);
           
           if(FindScene(scenel2)){
               SetFrequencyScene(scenel2, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
           }
           
           if(FindScene(scenel)){
               SetFrequencyScene(scenel, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
           }
           
           rx->matchFrequencies=1;

            //fprintf(stderr,"setFrequencyCoefficients\n");
       }
        adjustView(0);
        return;
    }else if(button != 0){
        return;
    }

    static double fsave;
   	if(state == GLUT_DOWN)
    {
        double fclick;
        
        fclick=rx->rmin+x*(double)(rx->rmax-rx->rmin)/(scene->xResolution);
        
        rx->f=fclick;
        
        fdown=fclick;
        
        fcdown=rx->fv;
        
        fsave=rx->fv;
        
        fcount=0;

        setFrequency2(rx);
        
        // printf("fclick %f button %d state %d x %d y %d\n",fclick,button,state,x,y);
    }else{
        if(fsave != rx->fv){
            setFrequency2(rx);
        }
        rx->matchFrequencies=1;
    }
    adjustView(0);
}

int Radio::FindPoint(struct Scene *scene,int x,int y)
{
    return 0;
}
static void enterexit(int item)
{
    RadioPtr sdr;
    
   if(item == GLUT_ENTERED){
       
        sdr=FindSdrRadioWindow(glutGetWindow());
       
        if(!sdr)return;
       
        // if(!sdr->inuseflag)sdr->flagsflag=1;
        
       // fprintf(stderr,"enterexit item %d\n",item);
    }
}
static void inuse(int item)
{
    RadioPtr sdr;
    
    sdr=FindSdrRadioWindow(glutGetWindow());
    
    if(!sdr)return;

    if(item == GLUT_MENU_NOT_IN_USE){
        sdr->inuseflag=0;
     //   fprintf(stderr,"not inuse sdr %p\n",sdr);
        
        RadioPtr f;
        CWinPtr w;
        
        w=Root;
        while(w){
            if(w->scene->windowType == FileTypeSdrRadio){
                f=(RadioPtr)w;
                if(f->inuseflag){
        //            fprintf(stderr,"found inuse sdr %p\n",f);
                    return;
                }
            }
            w=w->CNext;
        }

        if(sdr->flagsflag && (sdr->flags.size() > 0) && sdr->flagsmenu && !sdr->inuseflag){
            for(size_t k=0;k<sdr->flags.size();++k){
                if(sdr->flags[k].type == sdr->flags[k].BOOL){
                    glutSetMenu(sdr->flagsmenu[k]);
                    //int menu=glutGetMenu();
                    //fprintf(stderr,"k %d menu %d\n",(int)k,menu);
                    if(sdr->rx->device->readSetting(sdr->flags[k].key) == "true"){
                        glutChangeToMenuEntry(1,"(x) true",(int)(5000+2*k));
                        glutChangeToMenuEntry(2,"(  ) false",(int)(5000+2*k+1));
                    }else{
                        glutChangeToMenuEntry(1,"(  ) true",(int)(5000+2*k));
                        glutChangeToMenuEntry(2,"(x) false",(int)(5000+2*k+1));
                    }
                }
            }
            sdr->flagsflag=0;
        }
  //      fprintf(stderr,"not inuse done %p\n",sdr);
        sdr->inuseflag=0;
    }else{
   //     fprintf(stderr,"inuse sdr %p\n",sdr);
        sdr->inuseflag=1;
    }
    
    sdr->rx->averageGlobal=0;
}
static void menu_selectl(int item)
{
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
    sdr->mMenuSelectl(sdr->scene,item);

}

static int initScene(struct Scene *scene)
{
    
    if(!scene)return 1;
    
    SceneInit(scene);
    scene->windowType=FileTypeLines;
    
    scene->scale.showPalette=1;
    
    scene->scale.updateTemperatureScale=1;
    
    scene->scale.logscale=1;
        
    return 0;
}
static int DrawString(int x, int y, char *out)
{
    int h;
    
    if(!out)return 1;
    
    glPushMatrix();
    glRasterPos2i( x, y);
    while((h=*out++))
    {
        
        
        //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, h);
        //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, h);
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, h);
        /* glutBitmapCharacter(GLUT_BITMAP_8_BY_13, h); */
        
    }
    
    glPopMatrix();
    
    return 0;
}

static void getMousePassive3(int x, int y)
{
    struct SceneList *list;
    RadioPtr sdr;
    int window=glutGetWindow();

    list=SceneFindByNumber(window);
    if(!list){
        sdr=FindSdrRadioWindow(window);
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;
    
   //printf("getMousePassive3 %p list %p window %d\n",sdr,list,window);
    
    sdr->box.x=0;
    sdr->box.y=0;
    sdr->box.xsize=0;
    sdr->box.ysize=0;

    glutSetCursor( GLUT_CURSOR_CROSSHAIR );
}
static void getMousePassive(int x, int y)
{
        struct SceneList *list;
        RadioPtr sdr;
        
        int window=glutGetWindow();
        list=SceneFindByNumber(window);
        if(!list){
            sdr=FindSdrRadioWindow(window);
        }else{
            sdr=(RadioPtr)FindScene(&list->scene);
        }
        
        
        if(!sdr)return;

   // printf("getMousePassive %p list %p window %d\n",sdr,list,window);

    sdr->box.x=0;
    sdr->box.y=0;
    sdr->box.xsize=0;
    sdr->box.ysize=0;
    
    int up;
        

    //fprintf(stderr,"p x %d y %d\n",x,y);
    
    glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);

    if(y > 15){
        return;
    }else if(y > 10){
        up=0;
    }else{
        up=1;
    }
        

    int start=119;
    
    for(int k=0;k<10;++k){
        if(x >= start+k*9 && x <= start+(k+1)*9){
            if(up == 1){
               //printf("1 getMousePassive %d %d\n",x,y);
                sdr->box.x=start+k*9;
                sdr->box.y=20;
                sdr->box.xsize=9;
                sdr->box.ysize=9;
                
            }else{
                //printf("2 getMousePassive %d %d\n",x,y);
                sdr->box.x=start+k*9;
                sdr->box.y=10;
                sdr->box.xsize=9;
                sdr->box.ysize=9;
            }
        }
    }
    
    start=425;
    for(int k=0;k<10;++k){
        if(x >= start+k*9 && x <= start+(k+1)*9){
          if(up == 1){
                //printf("3 getMousePassive %d %d\n",x,y);
                sdr->box.x=start+k*9;
                sdr->box.y=20;
                sdr->box.xsize=9;
                sdr->box.ysize=9;
            }else{
                //printf("4 getMousePassive %d %d\n",x,y);
                sdr->box.x=start+k*9;
                sdr->box.y=10;
                sdr->box.xsize=9;
                sdr->box.ysize=9;
            }
        }
    }
}
static void moveMouse(int x, int y)
{
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
        return;
    }
    
    if(!sdr)return;
    

    struct Scene *scene;
    
    scene=sdr->scene;
    
    if(!scene)return;
    
    double fmove=sdr->fcdown-0.5*sdr->rx->samplerate+x*(double)sdr->rx->samplerate/(scene->xResolution);
    
    double diff=fmove-sdr->fdown;
    
    sdr->rx->fc=sdr->fcdown-diff;
    
    sdr->rx->fv=sdr->rx->fc;
    
}
 
int doWindow(double *x,double *y,long length,int type)
{    
    int i;
    
    if(!ww){
        ww=(double *)cMalloc(131072*sizeof(double),77777);
        if(!ww){
            fprintf(stderr,"doWindow out of memory\n");
            return 1;
        }
    }
    
    if(!x || !y)return 1;
    
    switch(type){
            
        case FILTER_RECTANGULAR:

            for(i=0; i<length; i++)
                ww[i] = 1.0;
            
            break;
            

            
        case FILTER_HANN:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                ww[i]=liquid_hann(i, (int)length);
#else
                ww[i]=hann(i, (int)length);
#endif
            }
            break;

            
            
        case FILTER_HAMMING:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                ww[i]=liquid_hamming(i, (int)length);
#else
                ww[i]=hamming(i, (int)length);
#endif
            }
            break;
            
        case FILTER_FLATTOP:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                ww[i]=liquid_flattop(i, (int)length);
#else
                ww[i]=flattop(i, (int)length);
#endif
            }
            break;
            
            
        case FILTER_BLACKMANHARRIS:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                ww[i]=liquid_blackmanharris(i, (int)length);
#else
                ww[i]=blackmanharris(i, (int)length);
#endif
            }
            break;
            
        case FILTER_BLACKMANHARRIS7:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                ww[i]=liquid_blackmanharris7(i, (int)length);
#else
                ww[i]=blackmanharris7(i, (int)length);
#endif
            }
            break;
    }
    
    for(i=0; i<length; i++){
        double amp;
        amp=ww[i];
        x[i]=amp*x[i];
        y[i]=amp*y[i];
    }
    
    return 0;
    
}

