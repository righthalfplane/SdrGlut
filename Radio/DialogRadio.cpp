/*
 *  DialogRangeData.c
 *  
 *
 *  Created by Dale Ranta on 3/3/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
#include "firstFile.h"
#include <cstdio>
#include <cstdlib>

#include "Radio.h"

#include "DialogRangeData.h"
#include "LoadFiles.h"
#include "Utilities.h"
#include "SceneList.h"

#include "CLines.h"



static GLUI_Checkbox *check_box;

static void control_cb(int control);
static void control_cb2(int control);

extern "C" int doFFT2(double *x,double *y,long length,int direction);

#define Mode_Buttons   180

int Radio::dialogSend(struct Scene *scene)
{
    if(!scene)return 1;
    
    if(bb.glui){
        bb.glui->close();
    }
    
    bb.sceneLocal=scene;
    
    bb.glui = GLUI_Master.create_glui(rx->driveName);
    
    GLUI_Panel *obj_panel =  bb.glui->add_panel( "Mode" );

    bb.group2 =
    bb.glui->add_radiogroup_to_panel(obj_panel,&bb.modetype,Mode_Buttons,control_cb2);
    
    bb.glui->add_radiobutton_to_group( bb.group2, "Float" );
    bb.glui->add_radiobutton_to_group( bb.group2, "Short int" );
    bb.glui->add_radiobutton_to_group( bb.group2, "Signed char" );
    bb.glui->add_radiobutton_to_group( bb.group2, "UnSigned char" );

    obj_panel =  bb.glui->add_panel( "Send Information" );


    new GLUI_Checkbox( obj_panel, "Frequency", &bb.frequencyFlag, 10, control_cb2 );
    
    new GLUI_Checkbox( obj_panel, "Demodulate Mode", &bb.demodulationFlag, 11, control_cb2 );

    obj_panel =  bb.glui->add_panel( "Tcp-Address" );
    
    
    bb.edittext1 =
    bb.glui->add_edittext_to_panel( obj_panel, "", GLUI_EDITTEXT_TEXT, bb.text1);
    bb.edittext1->w=200;


    obj_panel =  bb.glui->add_panel( "Commands" );
    
    new GLUI_Button(obj_panel, "Send", 2,control_cb2);
    
    new GLUI_Button(obj_panel, "Stop", 4, control_cb2);
    
    new GLUI_Button(obj_panel, "Close", 6, control_cb2);

    bb.sub_window=glutGetWindow();
    
    //bb.sub_window=bb.glui->get_glut_window_id();
    
    bb.glui->set_main_gfx_window(bb.sub_window);

    return 0;
}
static void control_cb2(int control)
{
    RadioPtr s=(RadioPtr)FindSceneRadio(glutGetWindow());
    if(!s)return;
    
    sscanf(s->bb.edittext1->get_text(),"%s",s->bb.text1);
    
    s->rx->demodulationFlag=s->bb.demodulationFlag;
    
    s->rx->frequencyFlag=s->bb.frequencyFlag;

    if(control == Mode_Buttons)
    {
        fprintf(stderr,"Mode_Buttons %d\n",s->bb.modetype);
    }else if(control == 2){
        (*s->rx->pStartSend)(s->rx,s->bb.text1,s->bb.modetype);
   } else if(control == 4){
       s->rx->controlSend = -1;
       fprintf(stderr,"Stop Command Send\n");
    } else if(control == 6){
        s->bb.glui->close();
        s->bb.glui=NULL;
    }
    glutPostRedisplay();
}
// static int doWindow(double *x,double *y,long length,int type);

int Radio::setDialogBandWidth(double bandwidth)
{
    char value[256];

    if(!dd.edittext15 || !inDialog)return 0;

    msprintf(value,sizeof(value),"%g",bandwidth);

    dd.edittext15->set_text(value);
    
    return 0;
}

int Radio::setDialogSampleRate(double samplerate)
{
    char value[256];

    if(!dd.edittext5 || !inDialog)return 0;

    msprintf(value,sizeof(value),"%g",samplerate);
    
    dd.edittext5->set_text(value);

    return 0;
}

int Radio::setDialogRange(double pmin,double pmax)
{
    char value[256];
    
    // fprintf(stderr," use %ld pmin %g pmax %g\n",(long)pd.UsePlotScales,pmin,pmax);
    
    if(pd.UsePlotScales)return 0;
    
    pd.sPmin=pmin;
    
    pd.sPmax=pmax;
    
    
    if(!dd.edittext7 || !dd.edittext8 || !inDialog)return 0;
    
    msprintf(value,sizeof(value),"%g",pmin);
    
    dd.edittext7->set_text(value);
    
    msprintf(value,sizeof(value),"%g",pmax);
    
    dd.edittext8->set_text(value);
    
    return 0;
}

int Radio::setDialogPower(double power)
{
    char value[256];
    
    if(!dd.edittext1 || !inDialog)return 0;
    
    msprintf(value,sizeof(value),"%.0f",power);
    
    dd.edittext6->set_text(value);
    
    return 0;
}
int Radio::setDialogFrequency(double frequency)
{
    char value[256];
    
    if(!dd.edittext1 || !inDialog)return 0;
    
    msprintf(value,sizeof(value),"%g",frequency);
    
    dd.edittext1->set_text(value);
    
    return 0;
}
int Radio::setDialogFc(double frequency)
{
    char value[256];
    
    if(!dd.edittext2 || !inDialog)return 0;
    
    msprintf(value,sizeof(value),"%g",frequency);
    
    dd.edittext2->set_text(value);
    
    return 0;
}
int Radio::dialogRadio(struct Scene *scene)
{
	if(!scene)return 1;
	
	if(dd.glui){
		dd.glui->close();
	}
	
	dd.sceneLocal=scene;

    inDialog=1;
	
	dd.glui = GLUI_Master.create_glui(rx->driveName);
			
	msprintf(dd.text1,sizeof(dd.text1),"%g",rx->f);
    msprintf(dd.text2,sizeof(dd.text2),"%g",rx->fc);
    msprintf(dd.text22,sizeof(dd.text22),"%g",rx->foffset);
    msprintf(dd.text3,sizeof(dd.text3),"%g",rx->gain);
    msprintf(dd.text4,sizeof(dd.text4),"%g",lineAlpha);
    msprintf(dd.text5,sizeof(dd.text5),"%g",rx->samplerate);
    msprintf(dd.text15,sizeof(dd.text15),"%g",rx->bandwidth);
    msprintf(dd.text16,sizeof(dd.text16),"%g",rx->scaleFactor);
    msprintf(dd.text17,sizeof(dd.text17),"%d",rx->audioThreads);
    msprintf(dd.text6,sizeof(dd.text6),"%.0f",power);
    msprintf(dd.text7,sizeof(dd.text7),"%g",pd.sPmin);
    msprintf(dd.text8,sizeof(dd.text8),"%g",pd.sPmax);
	
    GLUI_Panel *obj_panel =  dd.glui->add_panel( "Parameters" );

    dd.edittext1 =
	dd.glui->add_edittext_to_panel( obj_panel, "Frequency:", GLUI_EDITTEXT_TEXT, dd.text1 );
	dd.edittext1->w=200;
	
    dd.edittext2 =
    dd.glui->add_edittext_to_panel( obj_panel, "Center Frequency:", GLUI_EDITTEXT_TEXT, dd.text2 );
    dd.edittext2->w=200;
    
    dd.edittext22 =
    dd.glui->add_edittext_to_panel( obj_panel, "Frequency Offset:", GLUI_EDITTEXT_TEXT, dd.text22 );
    dd.edittext22->w=200;
    
    dd.edittext5 =
    dd.glui->add_edittext_to_panel( obj_panel, "Sample Rate:", GLUI_EDITTEXT_TEXT, dd.text5);
    dd.edittext5->w=200;
    

    obj_panel =  dd.glui->add_panel( "Gain + FFT Alpha" );
    
    dd.edittext3 =
    dd.glui->add_edittext_to_panel( obj_panel, "Gain:", GLUI_EDITTEXT_TEXT, dd.text3);
    dd.edittext3->w=200;
    
    dd.edittext4 =
    dd.glui->add_edittext_to_panel( obj_panel,"Alpha:", GLUI_EDITTEXT_TEXT, dd.text4);
    dd.edittext4->w=200;
    
    obj_panel =  dd.glui->add_panel( "Power" );

    dd.edittext6 =
    dd.glui->add_edittext_to_panel( obj_panel,"Power:", GLUI_EDITTEXT_TEXT, dd.text6);
    dd.edittext6->w=200;
    
    new GLUI_Checkbox( obj_panel, "Set Scale Power", &pd.UsePlotScales, 3, control_cb );
    
    dd.edittext7 =
    dd.glui->add_edittext_to_panel( obj_panel, "Power Min", GLUI_EDITTEXT_TEXT, dd.text7);
    dd.edittext7->w=200;
    
    dd.edittext8 =
    dd.glui->add_edittext_to_panel( obj_panel, "Power Max:", GLUI_EDITTEXT_TEXT, dd.text8);
    dd.edittext8->w=200;
  
    dd.edittext16 =
    dd.glui->add_edittext_to_panel( obj_panel,  "Scale Factor:", GLUI_EDITTEXT_TEXT, dd.text16);
    dd.edittext16->w=200;

    
    obj_panel =  dd.glui->add_panel( "Commands" );

	new GLUI_Button(obj_panel, "Set Frequency", 5, control_cb);
	
    new GLUI_Button(obj_panel, "Set Gain", 4, control_cb);
    
    new GLUI_Button(obj_panel, "Apply", 8, control_cb);
    
	new GLUI_Button(obj_panel, "Close", 2, control_cb);
    
    dd.glui->add_column(true);

    obj_panel =  dd.glui->add_panel( "Search" );
    
    msprintf(dd.text20,sizeof(dd.text20),"%g",rx->cutOFF);

    dd.edittext20 =
    dd.glui->add_edittext_to_panel( obj_panel,  "Cut Off (db):", GLUI_EDITTEXT_TEXT, dd.text20);
    dd.edittext20->w=200;

  
    msprintf(dd.text21,sizeof(dd.text21),"%g",pauseTimeDelta);

    dd.edittext21 =
    dd.glui->add_edittext_to_panel( obj_panel,  "Pause Time:", GLUI_EDITTEXT_TEXT, dd.text21);
    dd.edittext21->w=200;
    
    dd.search= new GLUI_Button(obj_panel, "Search", 2222, control_cb);
    
    new GLUI_Button(obj_panel, "Write", 2223, control_cb);
    
    dd.scan=new GLUI_Button(obj_panel, "Scan", 2224, control_cb);
    
    obj_panel =  dd.glui->add_panel( "Bandwidth + Audio Thread Count" );

    dd.edittext15 =
    dd.glui->add_edittext_to_panel( obj_panel, "Bandwidth:", GLUI_EDITTEXT_TEXT, dd.text15);
    dd.edittext15->w=200;
    
    
    dd.edittext17 =
    dd.glui->add_edittext_to_panel( obj_panel, "Audio Threads:", GLUI_EDITTEXT_TEXT, dd.text17);
    dd.edittext17->w=200;
    

    
    dd.useagc=rx->gainMode;
    
    check_box = new GLUI_Checkbox( dd.glui, "Use Automatic Agc", &dd.useagc, 7, control_cb );
    
    if(!rx->hasGainMode)check_box->disable();

    obj_panel =  dd.glui->add_panel( "Controls" );

    dd.iic=0;
    
    for(size_t i=0;i<rx->gainsCount;++i){
        GLUI_Panel *panel3 = new GLUI_Panel(obj_panel, rx->gains[dd.iic]);
        
		double el =rx->device->getGain(SOAPY_SDR_RX, rx->channel,  rx->gains[dd.iic]);;
		msprintf(dd.text1z,sizeof(dd.text1z),"%.0f",el);
        dd.edittext1z[dd.iic] =
        dd.glui->add_edittext_to_panel(panel3, "", GLUI_EDITTEXT_TEXT, dd.text1z );
        dd.edittext1z[dd.iic]->w=160;

        //fprintf(stderr,"el %g\n",el);
        
        dd.line_Index[dd.iic]=el;
        
        dd.line_scroll[dd.iic] =
        new GLUI_Scrollbar( panel3, rx->gains[dd.iic], GLUI_SCROLL_HORIZONTAL,
                           &dd.line_Index[dd.iic], 23+dd.iic,
                           control_cb );
        
        dd.line_scroll[dd.iic]->set_float_limits( rx->gainsMinimum[dd.iic], rx->gainsMaximum[dd.iic] );

        //printf("%s min %g max %g\n", rx->gains[dd.iic],rx->gainsMinimum[dd.iic],rx->gainsMaximum[dd.iic]);
        
        //new GLUI_Button(panel3, "Apply", 30, control_cb);
        ++dd.iic;
    
    }
    
    dd.gain_Index=dd.iic;
    
    GLUI_Panel *panel3 = new GLUI_Panel(obj_panel, "gain");
    double el = rx->device->getGain(SOAPY_SDR_RX, rx->channel);
    msprintf(dd.text1z,sizeof(dd.text1z),"%.0f",el);
    dd.edittext1z[dd.iic] =
    dd.glui->add_edittext_to_panel(panel3, "", GLUI_EDITTEXT_TEXT, dd.text1z );
    dd.edittext1z[dd.iic]->w=160;
    
    dd.line_Index[dd.iic]=el;
    
    //fprintf(stderr,"el %g\n",el);

    dd.line_scroll[dd.iic] =
    new GLUI_Scrollbar( panel3, "gain", GLUI_SCROLL_HORIZONTAL,
                       &dd.line_Index[dd.iic], 50,
                       control_cb );
    
    dd.line_scroll[dd.iic]->set_float_limits( rx->gainsMin, rx->gainsMax );
    
    ++dd.iic;

    //printf("%s min %g max %g\n", "gain",rx->gainsMin,rx->gainsMax);

    dd.sub_window=glutGetWindow();
    
    //dd.sub_window=dd.glui->get_glut_window_id();
    
	dd.glui->set_main_gfx_window(dd.sub_window);
	
	return 0;
}
static void control_cb(int control)
{
	std::string file_name;
    double f,fc,gain,lineAlpha,scaleFactor;
    double pauseTimeDelta;
    double dmin,dmax;
    double sameleRate;
    double bandwidth;
    double cutOFF;
    double foffset;
    int audioThreads;
    
    RadioPtr s=(RadioPtr)FindSceneRadio(glutGetWindow());
    if(!s)return;
	
    sscanf(s->dd.edittext1->get_text(),"%lg", &f);
    sscanf(s->dd.edittext2->get_text(),"%lg", &fc);
    sscanf(s->dd.edittext22->get_text(),"%lg", &foffset);
    sscanf(s->dd.edittext3->get_text(),"%lg", &gain);
    sscanf(s->dd.edittext4->get_text(),"%lg", &lineAlpha);
    sscanf(s->dd.edittext5->get_text(),"%lg", &sameleRate);
    sscanf(s->dd.edittext15->get_text(),"%lg", &bandwidth);
    sscanf(s->dd.edittext16->get_text(),"%lg", &scaleFactor);
    sscanf(s->dd.edittext17->get_text(),"%d", &audioThreads);
    sscanf(s->dd.edittext7->get_text(),"%lg", &dmin);
    sscanf(s->dd.edittext8->get_text(),"%lg", &dmax);
    sscanf(s->dd.edittext20->get_text(),"%lg", &cutOFF);
    sscanf(s->dd.edittext21->get_text(),"%lg", &pauseTimeDelta);

    
    if(control == 2222){
        if(s->rx->cutOFFSearch == 0){
            s->rx->cutOFF=cutOFF;
            s->rx->cutOFFSearch=1;
            for(int n=0;n<s->FFTlength;++n){
                s->frequencies[n]=0;
                s->ampitude[n] = -160;
            }
            s->dd.search->set_name("Stop Search");
            fprintf(stderr,"start Search\n");
        }else{
            s->rx->cutOFFSearch=0;
            s->dd.search->set_name("Search");
            s->processScan(s->rx);
            fprintf(stderr,"stop Search\n");
       }
    } else if(control == 2223){
        
        for(vector<double>::size_type k=0;k<s->scanFrequencies.size();++k){
            static int count;
            char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};
            WarningPrint("F%d,%0.4f,%s\n",count++,s->scanFrequencies[k]/1e6,Mode_Names[s->rx->decodemode]);
        }
    } else if(control == 2224){
        s->pauseTimeDelta=pauseTimeDelta;
        s->pauseTime=rtime()+s->pauseTimeDelta;
        s->pauseChannel=0;
        s->rx->cutOFF=cutOFF;
        
        if(s->scanFrequencies.size() == 0){
            fprintf(stderr,"Cannot Scan - Need to Load or Search for frequencies\n");
            return;
        }
        
        s->scanWait=0;

        if(s->scanRun == 0){
            s->dd.scan->set_name("Stop Scan");
            s->scanRun=1;
            fprintf(stderr,"Start Scan scanCount %ld\n",(long)s->scanFrequencies.size());
       }else{
            s->scanRun=0;
            s->dd.scan->set_name("Scan");
            s->setFrequency2(s->rx);
           fprintf(stderr,"Stop Scane\n");
      }
   
     }else if(control == 4){
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
        s->inDialog=0;
		s->dd.glui->close();
		s->dd.glui=NULL;
	}
    else if(control == 5)
    {
        s->rx->foffset=foffset;
        s->rx->fc=fc;
        s->rx->f=f;
        s->setFrequency2(s->rx);
    }
    else if(control == 7)
    {
        if(s->rx->hasGainMode){
            bool automatic=s->dd.useagc;
            s->rx->gainMode=s->dd.useagc;
            s->rx->device->setGainMode(SOAPY_SDR_RX, s->rx->channel, automatic);
        }
    }
    else if(control == 23  || control == 24  || control == 25  || control == 26)
    {
        char value[256];
        int ind=control-23;
        msprintf(value,sizeof(value),"%0.f",s->dd.line_Index[ind]);
        s->dd.edittext1z[ind]->set_text(value);
        if(s->dd.line_Index_old[ind] != (int)s->dd.line_Index[ind]){
            s->rx->device->setGain(SOAPY_SDR_RX, s->rx->channel, s->rx->gains[ind], s->dd.line_Index[ind]);
            s->dd.line_Index[ind]=(int)s->dd.line_Index_old[ind];
      }
    }
    else if(control == 50)
    {
        char value[256];
        int ind=s->dd.gain_Index;
        msprintf(value,sizeof(value),"%0.f",s->dd.line_Index[ind]);
        s->dd.edittext1z[ind]->set_text(value);
        if(s->dd.line_Index_old[ind] != (int)s->dd.line_Index[ind]){
            s->rx->device->setGain(SOAPY_SDR_RX, s->rx->channel, s->dd.line_Index[ind]);
            s->dd.line_Index_old[ind]=(int)s->dd.line_Index[ind];
        }
    }
    else if(control == 8)
    {
        s->stopPlay(s->rx);
        
        s->rx->scaleFactor=scaleFactor;
        s->rx->gain=gain;
        s->rx->foffset=foffset;
        s->rx->fc=fc;
        s->rx->f=f;
        s->lineAlpha=lineAlpha;
        s->rx->samplerate=sameleRate;
        s->rx->bandwidth=bandwidth;
        s->rx->audioThreads=audioThreads;

        RadioPtr ff=s->findMate(s->rx);
        if(ff){
            ff->rx->samplerate=sameleRate;
            ff->rx->bandwidth=bandwidth;
        }
        
        if(s->pd.UsePlotScales)
        {
            s->pd.sPmin=dmin;
            s->pd.sPmax=dmax;
        }
        
        
        for(int y=0;y<s->water.ysize*2;++y){
            int ns=3*s->water.xsize*y;
            for(int n=0;n<s->water.xsize;++n){
                s->water.data[ns+3*n]=255;
                s->water.data[ns+3*n+1]=255;
                s->water.data[ns+3*n+2]=255;
            }
        }
        
        s->startPlay(s->rx);
        
        s->playRadio(s->rx);
        
        s->water.nline=0;
        
    }
    
	glutPostRedisplay();
}
EXTERN CWinPtr Root;
RadioPtr FindSceneRadio(int window)
{    
    RadioPtr r;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSdrRadio){
            r=(RadioPtr)w;
            if(r->dd.sub_window == window)return r;
            if(r->bb.sub_window == window)return r;
        }
        w=w->CNext;
    }
    
    return NULL;

}
RadioPtr FindSdrRadioWindow(int window)
{
    RadioPtr f;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSdrRadio){
            f=(RadioPtr)w;
            if(f->window1 == window)return f;
        }
        w=w->CNext;
    }
    
    return NULL;
    
}
RadioPtr FindSdrRadioWindow(struct playData *rx)
{
    RadioPtr f;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSdrRadio){
            f=(RadioPtr)w;
            if(f->rx == rx)return f;
        }
        w=w->CNext;
    }
    
    return NULL;
    
}


