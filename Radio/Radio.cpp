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

#include <math.h>

#include <GL/glui.h>

extern "C" int closeScene(struct Scene *scene);

extern "C" int GridPlotScale(struct uGridPlot *b);

void menu_select(int item);

static void menu_selectl(int item);

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

ALvoid DisplayALError(unsigned char *szText, ALint errorCode);

static int initScene(struct Scene *scene);

extern "C" int doFFT2(double *x,double *y,long length,int direction);

int doWindow(double *x,double *y,long length,int type);

static void getMousel(int button, int state, int x, int y);

static void getMousePassive(int x, int y);

static int DrawString(int x, int y, char *out);

int DrawBox(uRect *box,int offset);

static GLUI *glui;

int doRadioOpenRA(void);

static int doRadioOpen2(void);

static void control_cb(int control);

void antennaMenu(int item);
void bandMenu(int item);
void sampleMenu(int item);
void doAudio(int item);

static void keys2(unsigned char key, int x, int y);

static int device;
static GLUI_EditText *edittext1;
static GLUI_EditText *edittext5;
static char sfrequency[255]="1";
static char ssamplerate[255]="2";

void doFFTMenu(int item);
void doDirectSampleMode(int item);
void doBiasMode(int item);
void doFilterMenu(int item);

Radio::Radio(struct Scene *scene): CWindow(scene)
{
    OpenError=TRUE;

	zerol((char *)&water, sizeof(water));

	zerol((char *)&dd, sizeof(dd));

    zerol((char *)&pd, sizeof(pd));
    
    zerol((char *)&rxs,&rxs.end-&rxs.start);

	getPaletteByName((char *)"ps", (unsigned char *)&pd.palette);

	pd.sType = 2;
    
    backGroundEvents=0;
    
    char sdevice[10];
    
    sprintf(sdevice,"%d",device);
    
    {
        char *list[]={NULL,(char *)"-fc",sfrequency ,(char *)"-fm" ,(char *)"-gain" ,(char *)"0.25",(char *)"-device",sdevice,
                      (char *)"-samplerate",ssamplerate};
        
        //char *list[]={NULL,"-fc","27.1e6" ,"-f" ,"27.3235e6" ,"-lsb" ,"-gain" ,"1","-device",sdevice};

        RadioStart ((int)sizeof(list)/sizeof(char *), list,&rxs);
        
    }

    rx=&rxs;
    
    if(!rx)return;

    
    //fprintf(stderr,"Radio::Radio\n");
    
    lineDumpInterval=0.1;
    lineTime=rtime()+lineDumpInterval;
    
    lineAlpha=0.1;
    
    FFTlength=32768;
    
    range=NULL;
    dose=NULL;
    
    lreal=NULL;
    limag=NULL;
    

    range=(double *)cMalloc(FFTlength*sizeof(double),9851);
    dose=(double *)cMalloc(FFTlength*sizeof(double),9851);
    
    lreal=(double *)cMalloc(FFTlength*sizeof(double),9851);
    limag=(double *)cMalloc(FFTlength*sizeof(double),9851);
    
    

    if(!range || !dose || !lreal || !lreal)return;
    
    zerol((char *)lreal,FFTlength*sizeof(double));
    zerol((char *)limag,FFTlength*sizeof(double));
    
    OpenError=FALSE;
}

static int doRadioOpen2(void)
{
    struct SceneList *list;
    struct Scene *scene;
    int FileType;
    
    FileType=FileTypeSdrRadio;

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

    RadioPtr w = new Radio(scene);
    
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
    
    glutSetWindow(w->lines2->window);
    glutSetWindowTitle(w->rx->driveName);
    
    glutSetWindow(w->window);
    glutSetWindowTitle(w->rx->driveName);
    mstrncpy(w->windowName,w->rx->driveName,sizeof(w->windowName));

    
    w->backGroundEvents=1;
   return 1;
}

int doRadioOpenRA(void)
{
    
    size_t length;

    std::string argStr;
    
    SoapySDRKwargs *results = SoapySDRDevice_enumerate(NULL, &length);

    device=0;

    if(length == 0){
        fprintf(stderr,"Error: SoapySDRDevice_enumerate Found No Devices - Try Again !\n");
        return 0;
    }
    
    // std::cout << "results.size " << results.size() << std::endl;
    
    SoapySDRKwargs deviceArgs;
    
    if(glui){
        glui->close();
    }
    
    glui = GLUI_Master.create_glui("Select Device");
    
    msprintf(sfrequency,sizeof(sfrequency),"%g",100.0e6);
    
    msprintf(ssamplerate,sizeof(ssamplerate),"%.0f",2000000.0);
    
    edittext1 =
    glui->add_edittext( "Frequency:", GLUI_EDITTEXT_TEXT, sfrequency );
    edittext1->w=200;
    
    edittext5 =
    glui->add_edittext( "Sample Rate:", GLUI_EDITTEXT_TEXT, ssamplerate);
    edittext5->w=200;

    for(size_t k=0;k<length;++k){
        
            deviceArgs = results[k];
        
            for (size_t j = 0; j < deviceArgs.size; j++){
                if (strcmp(deviceArgs.keys[j],"driver")) {
                } else if (strcmp(deviceArgs.vals[j],"label")) {
                    new GLUI_Button(glui, deviceArgs.vals[j] , (int)(2+k), control_cb);
                }
            }

    }
    
    if(length == 0){
        new GLUI_Button(glui, "Error Device not Found" , 2, control_cb);
    }
    
    new GLUI_Button(glui, "Quit", 1, control_cb);

    glui->set_main_gfx_window( glutGetWindow() );
    
    return 0;
}

static void control_cb(int control)
{
    glui->close();
	glui = NULL;
    if(control == 1){
        return;
    }
    
    device = control-2;
    
    doRadioOpen2();
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
  //  fprintf(stderr,"~Radio() \n");
    
    rx->psdrDone(rx);

    if(range)cFree((char *)range);
    range=NULL;
    
    if(dose)cFree((char *)dose);
    dose=NULL;
    
    if(lreal)cFree((char *)lreal);
    lreal=NULL;
    
    if(limag)cFree((char *)limag);
    limag=NULL;
        
    if(water.data)cFree((char *)water.data);
    water.data=NULL;

    sceneClean(scene);
	
    if(inDialog){
        inDialog=0;
        if(dd.glui){
            glutSetWindow(dd.sub_window);
            dd.glui->close();
        }
        dd.glui=NULL;
    }
    
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

}
int Radio::updateLine()
{
    
    double *real,*imag;
    double amin,amax,v;
    
    if(rtime() < lineTime)return 0;

    lineTime=rtime()+lineDumpInterval;
    
    if(rx->controlRF != 2)return 0;
    
    if(rx->FFTcount > FFTlength){
        printf(" FFTlength %ld\n",FFTlength);
        return 1;
    }
    
    int length=rx->FFTcount;
    
    setDialogPower(rx->m_SMeter.GetAve());
    
    real=rx->real;
    imag=rx->imag;
    
    double average=0;
    for(int k=0;k<length;++k){
        average += sqrt(real[k]*real[k]+imag[k]*imag[k]);
    }
    average /= length;
    
   //static int drops=0;
   //if(rx->averageGlobal == 0)drops=0;
    if(rx->averageGlobal == 0)rx->averageGlobal=average;
    rx->averageGlobal = 0.9*rx->averageGlobal+0.1*average;
    if(average < 0.1*rx->averageGlobal){
       // printf("Device '%s' Drop out average %g averageGlobal %g drops %d\n",rx->driveName,average,rx->averageGlobal,++drops);
        return 0;
    }
    
    doWindow(real,imag,length,rx->FFTfilter);

    for(int n=0;n<length;++n){
        real[n] *= pow(-1.0,n);
        imag[n] *= pow(-1.0,n);
    }
    
    doFFT2(real,imag,length,1);
    
    amin=water.amin;
    amax=water.amax;
    
    double rmin=  1e33;
    double rmax= -1e33;
    
    float dx=rx->samplerate;
    double ddx=(double)rx->samplerate/(double)(length);
    long nf=0;
    for(int n=0;n<length;++n){
        double r;
        r=rx->fc-0.5*rx->samplerate+(n+0.5)*ddx;
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
        lreal[length-n-1]=(1.0-lineAlpha)*lreal[length-n-1]+v*lineAlpha;
        v=lreal[length-n-1];
        if(v < amin)amin=v;
        if(v > amax)amax=v;
    }
    
    if(FindScene(scenel)){
        lines->plotPutData(scenel,range,lreal,length,0L);
        
        uGridPlotPtr Plot;
        Plot=lines->lines->Plot;
        if(!Plot)return 1;

       Plot->xAutoMaximum=TRUE;
       Plot->xAutoMinimum=TRUE;

        GridPlotScale(Plot);
        
        Plot->xAutoMaximum=FALSE;
        Plot->xAutoMinimum=FALSE;
        Plot->xSetMaximum=rmax;
        Plot->xSetMinimum=rmin;
        if(rx->cutOFFSearch > 0){
            //fprintf(stderr,"length %d cutOFF %g bw %g\n",length,rx->cutOFF,rx->bw);
            int itwas = -1;
            int ns=0;
            for(int k=0;k<length;++k){
                if(range[ns]+rx->bw > range[k]){
                    continue;
                }
               // fprintf(stderr,"low %g high %g\n",range[ns],range[k]);
                double dmin = -160;
                int nn = -1;
                double rr=0;
                for(int i=ns;i<k;++i){
                    if(i >= length)break;
                    rr=lreal[i];
                    // fprintf(stderr,"i %d rr %g dmin %g\n",i,rr,dmin);
                    if(rr > dmin){
                        dmin=rr;
                        nn=i;
                    }
                }
                ns=k;
                if(nn > 0){
                    char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};
                    
                    static int count;
                    
                    if(dmin > rx->cutOFF){
                      // fprintf(stderr,"dmin %g range %g nn %d itwas %d\n",dmin,range[nn],nn,itwas);
                        if(nn-itwas > 5){
                           WarningPrint("F%d,%0.4f,%s\n",count++,range[nn]/1e6,Mode_Names[rx->decodemode]);
                        }
                        itwas=nn;
                    }
                }
            }
            
            
            rx->cutOFFSearch=0;
        }
    }
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
    
    if(FindScene(scenel2))lines2->plotPutData(scenel2,&range[ns],&lreal[ns],ne,0L);

    if(water.data == NULL)return 0;
    
    pd.dmin=amin;
    pd.dmax=amax;
    
    
    setDialogRange(amin,amax);
        
    amin=water.amin;
    amax=water.amax;
    
    if(water.nline >= water.ysize)water.nline=0;
    
    FloatToImage(lreal,length,&pd,water.ic);
    
    int ns1=3*water.xsize*(water.ysize-water.nline-1);
    int ns2=3*water.xsize*water.ysize+3*water.xsize*(water.ysize-1-water.nline++);
    
    
    // fprintf(stderr,"water length %ld\n",length);

    for(int n=2;n<length-2;++n){
        int ic;
        
        ic=water.ic[n];

        if(water.ic[n-1] > ic)ic=water.ic[n-1];
        if(water.ic[n-2] > ic)ic=water.ic[n-2];
        if(water.ic[n+1] > ic)ic=water.ic[n+1];
        if(water.ic[n+2] > ic)ic=water.ic[n+2];

        
        water.data[ns1+3*n]=pd.palette[3*ic];
        water.data[ns1+3*n+1]=pd.palette[3*ic+1];
        water.data[ns1+3*n+2]=pd.palette[3*ic+2];
        
        

        water.data[ns2+3*n]=pd.palette[3*ic];
        water.data[ns2+3*n+1]=pd.palette[3*ic+1];
        water.data[ns2+3*n+2]=pd.palette[3*ic+2];
        
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
    
    updateLine();
    
    return 0;
}
int Radio::sendMessage(char *m1,char *m2,int type)
{
    char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};

    if(type == M_SEND){
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
        rx->fc=ff+rx->bw;

        setFrequency(rx);
        
        glutSetWindow(wnd);

    }
    
    return 0;
}

int Radio::SetFrequency(struct Scene *scene,double f,double bw, int message)
{
    char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};

    static int count;
    
    if(message == M_MUTE){
        rx->mute = !rx->mute;
        return 0;
    }else if(message == M_SAVE){
        WarningPrint("F%d,%0.4f,%s\n",count++,rx->f/1e6,Mode_Names[rx->decodemode]);
        return 0;
    } else if(message == M_FREQUENCY){
        
        rx->f=f;
        if(fabs(f-rx->fc) > 0.5*rx->samplerate){
            rx->fc=f;
            setFrequency(rx);
            return 0;
       }
        setFrequencyCoefficients(rx);
        return 0;
    }
    
    rx->f=f;
    
    fdown=f;
    
    fcdown=rx->fc;
    
    fcount=0;
    
    setFrequency(rx);
    
    return 0;
}
int Radio::setFrequencyCoefficients(struct playData *rx)
{
    float pi;
    pi=4.0*atan(1.0);
    rx->dt=1.0/(double)rx->samplerate;
    rx->sino=0;
    rx->coso=1;
    rx->w=2.0*pi*(rx->fc - rx->f);
    rx->sindt=sin(rx->w*rx->dt);
    rx->cosdt=cos(rx->w*rx->dt);

    return 0;
}
int Radio::setFrequency(struct playData *rx)
{
    
    
    try {
        
        if(rx->device){
            rx->device->setFrequency(SOAPY_SDR_RX, 0, rx->fc);
        }
        
        setDialogFrequency(rx->f);
        
        setFrequency(rx->f);
        
        setDialogFc(rx->fc);
        
        setFrequencyCoefficients(rx);
        
        rx->aminGlobal=0;
        
        rx->amaxGlobal=0;
        
        rx->averageGlobal=0;
        
        rx->m_SMeter.Reset();
        
        if(FindScene(scenel2)){
            SetFrequencyGlobal(scenel2, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
        }
        
        if(FindScene(scenel)){
            SetFrequencyGlobal(scenel, rx->f, rx->bw, M_FREQUENCY_BANDWIDTH);
        }
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
    
    xsize=(int)rx->FFTcount;
    
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
    
    int nf=(int)(0.5+(scene->xResolution)*((rx->f-rx->fc)+0.5*rx->samplerate)/(double)rx->samplerate);
    
    int nf1=(int)(0.5+(scene->xResolution)*((rx->f-rx->fc)+0.5*rx->samplerate+0.5*rx->bw)/(double)rx->samplerate);

    int nf2=(int)(0.5+(scene->xResolution)*((rx->f-rx->fc)+0.5*rx->samplerate-0.5*rx->bw)/(double)rx->samplerate);
    
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
    
    f=(unsigned long)images->rx->f;
    
    fc=(unsigned long)images->rx->fc;
    
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
            msprintf(value,sizeof(value),"Frequency: %010ld Hz   Center Frequency: %010ld Hz Power: %.0f db MUTE",f,fc,images->rx->m_SMeter.GetAve());
        }else{
            msprintf(value,sizeof(value),"Frequency: %010ld Hz   Center Frequency: %010ld Hz Power: %.0f db",f,fc,images->rx->m_SMeter.GetAve());
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
    
    //printf("DrawBox %d %d %d %d\n",box->x,box->x+box->xsize,box->y+offset,box->y-box->ysize+offset);
    
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
    
    window1 = glutCreateSubWindow(window,0,170,960,170);
    glutMouseFunc(getMousel);
    glutMotionFunc(moveMouse);
    glutDisplayFunc(displayc);
    glutKeyboardFunc(keys2);

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
    glutAddMenuEntry("Start Audio Recording", START_AUDiO);
    glutAddMenuEntry("Stop  Audio Recording", STOP_AUDiO);
    glutAddMenuEntry("IQ Recording", START_IQ);
    
    int menu5=glutCreateMenu(doFFTMenu);
    glutAddMenuEntry("1024", FFT_1024);
    glutAddMenuEntry("2048", FFT_2048);
    glutAddMenuEntry("4096", FFT_4096);
    glutAddMenuEntry("8192", FFT_8192);
    glutAddMenuEntry("16384", FFT_16384);
    glutAddMenuEntry("32768", FFT_32768);
    
    
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
    
    glutCreateMenu(menu_selectl);
    glutAddMenuEntry("Sdr Dialog...", SdrDialog);
    glutAddMenuEntry("Transmit...", SdrTransmit);
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
    scenel=&list->scene;
    zerol((char *)scenel,sizeof(struct Scene));
    
    initScene(scenel);
    
    lines = CLines::CLinesOpen(scenel,window);
    
    lines->plotPutData(scenel,range,dose,rx->FFTcount,-1L);
    
    lines->sceneSource=sceneOpen;
    
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
    
    lines2->plotPutData(scenel2,range,dose,rx->FFTcount,-1L);
    
    lines2->sceneSource=sceneOpen;
    
    lines2->wShift=0;

//    lines2->sdr=NULL;

    lines2->lines->Plot->yLogScale=0;
    
    lines2->lines->Plot->gridHash=1;
/*
    lines2->lines->Plot->yAutoMaximum=FALSE;
    lines2->lines->Plot->yAutoMinimum=FALSE;
    lines2->lines->Plot->ySetMaximum=-20;
    lines2->lines->Plot->ySetMinimum=-120;
    lines2->lines->Plot->yMajorStep=20;
*/

    glutSetWindow(list->window);
    glutPositionWindow(20,444);
    glutReshapeWindow(600, 200);

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
    
    sdr->rx->device->setAntenna(SOAPY_SDR_RX, 0, sdr->rx->antenna[item]);
    
    return;
}
void bandMenu(int item){
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;

    //printf("item %d bandwidths %.0f\n",item,sdrOpen->rx->bandwidths[item]);
    
    sdr->rx->pstopPlay(sdr->rx);
    
    sdr->rx->bandwidth=sdr->rx->bandwidths[item];
    
    sdr->setDialogBandWidth(sdr->rx->bandwidth);
    
    sdr->rx->pstartPlay(sdr->rx);
    
    sdr->rx->pplayRadio(sdr->rx);

    return;
}
void sampleMenu(int item){
    
    struct SceneList *list;
    RadioPtr sdr;
    
    list=SceneFindByNumber(glutGetWindow());
    if(!list){
        sdr=FindSdrRadioWindow(glutGetWindow());
    }else{
        sdr=(RadioPtr)FindScene(&list->scene);
    }
    
    if(!sdr)return;

    
    double sameleRate=sdr->rx->sampleRates[item];
    //printf("item %d sampleRate %.0f\n",item,sameleRate);
 
    sdr->rx->pstopPlay(sdr->rx);
    
    sdr->rx->samplerate=sameleRate;
    
    sdr->setDialogSampleRate(sameleRate);

    sdr->rx->pstartPlay(sdr->rx);
    
    sdr->rx->pplayRadio(sdr->rx);

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
    sdr->lines2->wShift = 0;
    sdr->rx->wShift = 0;
    
    switch(item){
        case MODE_AM:
            sdr->rx->decodemode = MODE_AM;
            break;
        case MODE_NAM:
            sdr->rx->decodemode = MODE_NAM;
            break;
        case MODE_USB:
            sdr->lines->wShift = 1;
            sdr->lines2->wShift = 1;
            sdr->rx->wShift = 1;
            sdr->rx->decodemode = MODE_USB;
            break;
        case MODE_LSB:
            sdr->lines->wShift  = -1;
            sdr->lines2->wShift = -1;
            sdr->rx->wShift = -1;
            sdr->rx->decodemode = MODE_LSB;
            break;
        case MODE_CW:
            sdr->lines->wShift  = -1;
            sdr->lines2->wShift = -1;
            sdr->rx->wShift = -1;
            sdr->rx->decodemode = MODE_CW;
            break;
        case MODE_FM:
            sdr->rx->decodemode = MODE_FM;
             break;
        case MODE_NBFM:
            sdr->rx->decodemode = MODE_NBFM;
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

    sdr->rx->pSetAudio(sdr->rx,name,START_AUDiO);
    
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
    
    SoapySDR::ArgInfo arg;
    
    sdr->rx->device->writeSetting(sdr->rx->biasMode,value);
    
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
            dialogSaveC(sdr->scene, AudioSave,0,NULL);
            break;
        case STOP_AUDiO:
            sdr->rx->pSetAudio(sdr->rx,NULL,START_AUDiO);
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
    
    for(int n=0;n<myAppl->FFTlength;++n){
        myAppl->lreal[n]=0;
        myAppl->limag[n]=0;
    }
    
    for(int y=0;y<myAppl->water.ysize*2;++y){
        int ns=3*myAppl->water.xsize*y;
        for(int n=0;n<myAppl->water.xsize;++n){
            myAppl->water.data[ns+3*n]=255;
            myAppl->water.data[ns+3*n+1]=255;
            myAppl->water.data[ns+3*n+2]=255;
        }
    }
    
    myAppl->rx->psdrSetMode(myAppl->rx);

    myAppl->setFrequency(myAppl->rx);
    
   // myAppl->rx->pstopPlay(myAppl->rx);
    
   // myAppl->rx->pstartPlay(myAppl->rx);
    
    //myAppl->rx->pplayRadio(myAppl->rx);
    
    myAppl->water.nline=0;
    
    return 0;
}
int Radio::mMenuSelectl(struct Scene *scene,int item)
{
    
    if(!scene)return 1;
    
	switch (item)
	{

        case SdrDialog:
            dialogRadio(scene);
            return 0;
            
        case SdrTransmit:
            Transmit(scene);
            return 0;
            
	case ControlGetSelectionBox:
		return 0;

	case ControlQuit:
		dialogQuitC();
		break;

	case ControlClose:

		try {
			backGroundEvents = 0;
			if (FindScene(scenel))closeScene(lines->scene);
			if (FindScene(scenel2))closeScene(lines2->scene);
			if (FindScene(scene))closeScene(scene);
		}
		catch (...)
		{
			fprintf(stderr, "exception while closing scenes\n");
		}
        break;
            
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
            WarningPrint("F%d,%0.4f,%s\n",count++,sdr->rx->f/1e6,Mode_Names[sdr->rx->decodemode]);
        }
    }
    
   // fprintf(stderr,"Radio keys - key %d key %c \n",key,key);
    
}

static void getMousel(int button, int state, int x, int y)
{
    struct SceneList *list;
    RadioPtr sdr;
    
    
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
                sdr->setFrequency(sdr->rx);
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
                sdr->setFrequency(sdr->rx);
           }
        }

    }
    //sdr->getMouse(button, state, x, y);
    
    
}

void Radio::getMouse(int button, int state, int x, int y)
{

    if(button == 3){
        float fl,bw;
        bw=rx->bw*0.5;
        if(rx->wShift != 0)bw /= 2;
        fl=rx->f-bw;
        if(fl < 0)fl=-fl;
        rx->f=fl;
        if(fabs(fl-rx->fc) > 0.5*rx->samplerate){
            rx->fc=fl;
        }
        setFrequency(rx);
        return;
    }else if(button == 4){
        float fl,bw;
        bw=rx->bw*0.5;
        if(rx->wShift != 0)bw /= 2;
        fl=rx->f+bw;
        if(fl < 0)fl=-fl;
        rx->f=fl;
        if(fabs(fl-rx->fc) > 0.5*rx->samplerate){
            rx->fc=fl;
        }
        setFrequency(rx);
        return;
    }else if(button != 0){
        return;
    }
    
   	if(state == GLUT_DOWN)
    {
        double fclick;
        
        fclick=rx->fc-0.5*rx->samplerate+x*(double)rx->samplerate/(scene->xResolution);
        
        rx->f=fclick;
        
        fdown=fclick;
        
        fcdown=rx->fc;
        
        fcount=0;

        setFrequency(rx);
        
        // printf("fclick %f button %d state %d x %d y %d\n",fclick,button,state,x,y);
    }else{
        setFrequency(rx);
    }
}

int Radio::FindPoint(struct Scene *scene,int x,int y)
{
    return 0;
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

static void getMousePassive(int x, int y)
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
    
    sdr->box.x=0;
    sdr->box.y=0;
    sdr->box.xsize=0;
    sdr->box.ysize=0;
    
    int up;
    
    if(y > 20){
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
    
}
 
int doWindow(double *x,double *y,long length,int type)
{
    //double w[length];
    double w[32768];
    int i;
    
    if(!x || !y)return 1;
    
    switch(type){
            
        case FILTER_RECTANGULAR:

            for(i=0; i<length; i++)
                w[i] = 1.0;
            
            break;
            
#if LIQUID_VERSION_NUMBER >= 1003002
#define WINDOWS_LONG_NAMES 1
#endif

            
        case FILTER_HANN:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hann(i, (int)length);
#else
                w[i]=hann(i, (int)length);
#endif
            }
            break;

            
            
        case FILTER_HAMMING:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hamming(i, (int)length);
#else
                w[i]=hamming(i, (int)length);
#endif
            }
            break;
            
        case FILTER_FLATTOP:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_flattop(i, (int)length);
#else
                w[i]=flattop(i, (int)length);
#endif
            }
            break;
            
            
        case FILTER_BLACKMANHARRIS:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris(i, (int)length);
#else
                w[i]=blackmanharris(i, (int)length);
#endif
            }
            break;
            
        case FILTER_BLACKMANHARRIS7:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris7(i, (int)length);
#else
                w[i]=blackmanharris7(i, (int)length);
#endif
            }
            break;
    }
                                 
    for(i=0; i<length; i++){
        double amp;
        amp=w[i];
        x[i]=amp*x[i];
        y[i]=amp*y[i];
    }
    
    
    
    return 0;
    
}

