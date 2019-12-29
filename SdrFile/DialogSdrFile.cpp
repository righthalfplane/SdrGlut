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

#include "SdrFile.h"

#include "DialogSdrFile.h"
#include "LoadFiles.h"
#include "Utilities.h"
#include "SceneList.h"

#include "CLines.h"


static void control_cb(int control);

int SdrFile::setDialogRange(double pmin,double pmax)
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

int SdrFile::setDialogPower(double power)
{
    char value[256];
    
    if(!dd.edittext1 || !inDialog)return 0;
    
    msprintf(value,sizeof(value),"%.0f",power);
    
    dd.edittext6->set_text(value);
    
    return 0;
}
int SdrFile::setDialogFrequency(double frequency)
{
    char value[256];
    
    if(!dd.edittext1 || !inDialog)return 0;
    
    msprintf(value,sizeof(value),"%g",frequency);
    
    dd.edittext1->set_text(value);
    
    return 0;
}
int SdrFile::setDialogFc(double frequency)
{
    char value[256];
    
    if(!dd.edittext2 || !inDialog)return 0;
    
    msprintf(value,sizeof(value),"%g",frequency);
    
    dd.edittext2->set_text(value);
    
    return 0;
}
int SdrFile::dialogSdrFile(struct Scene *scene)
{
	if(!scene)return 1;
	
	if(dd.glui){
		dd.glui->close();
	}
	
	dd.sceneLocal=scene;

    inDialog=1;
	
	dd.glui = GLUI_Master.create_glui(windowName);
			
	msprintf(dd.text1,sizeof(dd.text1),"%g",play.f);
    msprintf(dd.text2,sizeof(dd.text2),"%g",play.fc);
    msprintf(dd.text3,sizeof(dd.text3),"%g",play.gain);
    msprintf(dd.text4,sizeof(dd.text4),"%g",lineAlpha);
    msprintf(dd.text5,sizeof(dd.text5),"%g",play.samplerate);
    msprintf(dd.text15,sizeof(dd.text15),"%g",play.bw);
    msprintf(dd.text6,sizeof(dd.text6),"%.0f",power);
    msprintf(dd.text7,sizeof(dd.text7),"%g",pd.sPmin);
    msprintf(dd.text8,sizeof(dd.text8),"%g",pd.sPmax);
    msprintf(dd.text16,sizeof(dd.text16),"%g",play.scaleFactor);

	dd.edittext1 =
	dd.glui->add_edittext( "Frequency:", GLUI_EDITTEXT_TEXT, dd.text1 );
	dd.edittext1->w=200;
	
    dd.edittext2 =
    dd.glui->add_edittext( "Center Frequency:", GLUI_EDITTEXT_TEXT, dd.text2 );
    dd.edittext2->w=200;
    
    dd.edittext3 =
    dd.glui->add_edittext( "Gain:", GLUI_EDITTEXT_TEXT, dd.text3);
    dd.edittext3->w=200;
    
    dd.edittext4 =
    dd.glui->add_edittext( "Alpha:", GLUI_EDITTEXT_TEXT, dd.text4);
    dd.edittext4->w=200;
    
    dd.edittext5 =
    dd.glui->add_edittext( "Sample Rate:", GLUI_EDITTEXT_TEXT, dd.text5);
    dd.edittext5->w=200;
    
    dd.edittext6 =
    dd.glui->add_edittext( "Power:", GLUI_EDITTEXT_TEXT, dd.text6);
    dd.edittext6->w=200;
    
    new GLUI_Checkbox( dd.glui, "Set Scale Power", &pd.UsePlotScales, 3, control_cb );
    
    dd.edittext7 =
    dd.glui->add_edittext( "Power Min", GLUI_EDITTEXT_TEXT, dd.text7);
    dd.edittext7->w=200;
    
    dd.edittext8 =
    dd.glui->add_edittext( "Power Max:", GLUI_EDITTEXT_TEXT, dd.text8);
    dd.edittext8->w=200;
    
	new GLUI_Button(dd.glui, "Set Frequency", 5, control_cb);
	
    new GLUI_Button(dd.glui, "Set Gain", 4, control_cb);
    
    new GLUI_Button(dd.glui, "Apply", 8, control_cb);
    
	new GLUI_Button(dd.glui, "Close", 2, control_cb);
    
    dd.glui->add_column(true);


    dd.edittext15 =
    dd.glui->add_edittext( "Bandwidth:", GLUI_EDITTEXT_TEXT, dd.text15);
    dd.edittext15->w=200;

    dd.edittext16 =
    dd.glui->add_edittext( "Scale Factor:", GLUI_EDITTEXT_TEXT, dd.text16);
    dd.edittext16->w=200;

    
    dd.useagc=play.gainMode;
    
    dd.check_box = new GLUI_Checkbox( dd.glui, "Use Automatic Agc", &dd.useagc, 7, control_cb );
    
    if(!play.hasGainMode)dd.check_box->disable();

    dd.iic=0;
    GLUI_Panel *panel3 = new GLUI_Panel(dd.glui, "Set Playback Time");
    msprintf(dd.text1z,sizeof(dd.text1z),"%d",0);
    dd.edittext1z[dd.iic] =
    dd.glui->add_edittext_to_panel(panel3, "Hours", GLUI_EDITTEXT_TEXT, dd.text1z );
    dd.edittext1z[dd.iic]->w=160;
    ++dd.iic;
    dd.edittext1z[dd.iic] =
    dd.glui->add_edittext_to_panel(panel3, "Minutes", GLUI_EDITTEXT_TEXT, dd.text1z );
    dd.edittext1z[dd.iic]->w=160;
    ++dd.iic;
    dd.edittext1z[dd.iic] =
    dd.glui->add_edittext_to_panel(panel3, "Seconds", GLUI_EDITTEXT_TEXT, dd.text1z );
    dd.edittext1z[dd.iic]->w=160;
    new GLUI_Button(panel3, "Set Time", 888, control_cb);

    
    dd.sub_window=glutGetWindow();
    
	dd.glui->set_main_gfx_window(dd.sub_window );
	
	return 0;
}
static void control_cb(int control)
{
	std::string file_name;
    double f,fc,gain,lineAlpha,scaleFactor;
    double dmin,dmax;
    double sameleRate;
    double bandwidth;
    
    SdrFilePtr s=(SdrFilePtr)FindSceneSdrFile(glutGetWindow());
    if(!s)return;
    
    
    struct DialogSdrData *dd=&s->dd;
	
    sscanf(dd->edittext1->get_text(),"%lg", &f);
    sscanf(dd->edittext2->get_text(),"%lg", &fc);
    sscanf(dd->edittext3->get_text(),"%lg", &gain);
    sscanf(dd->edittext4->get_text(),"%lg", &lineAlpha);
    sscanf(dd->edittext5->get_text(),"%lg", &sameleRate);
    sscanf(dd->edittext15->get_text(),"%lg", &bandwidth);
    sscanf(dd->edittext16->get_text(),"%lg", &scaleFactor);
    sscanf(dd->edittext7->get_text(),"%lg", &dmin);
    sscanf(dd->edittext8->get_text(),"%lg", &dmax);
    
    if(control == 4){
        s->play.gain=gain;
        s->play.scaleFactor=scaleFactor;
        if(s->pd.UsePlotScales)
        {
            s->pd.sPmin=dmin;
            
            s->pd.sPmax=dmax;
           
        }
        
    }
    else if(control == 2)
	{
        s->inDialog=0;
		dd->glui->close();
		dd->glui=NULL;
	}
    else if(control == 5)
    {
        s->play.fc=fc;
        s->play.f=f;
        s->setFrequency(&s->play);
    }
    else if(control == 888)
    {
        float hours=0;
        sscanf(dd->edittext1z[0]->get_text(),"%f", &hours);
        float min=0;
        sscanf(dd->edittext1z[1]->get_text(),"%f", &min);
        float sec=0;
        sscanf(dd->edittext1z[2]->get_text(),"%f", &sec);
        
        long frame=(long)((3600*hours+60*min+sec)*10);
        
        //fprintf(stderr,"hours %.1f min %.1f sec %.1f frame %ld\n",hours,min,sec,frame);
        
        s->play.setFrameNumber=frame;
        
        s->play.controlProcess = -3;
        
   }
    else if(control == 8)
    {
        s->play.frame=-1;
        s->stopPlay(&s->play);
        
        s->play.scaleFactor=scaleFactor;

        s->play.gain=gain;
        s->play.fc=fc;
        s->play.f=f;
        s->lineAlpha=lineAlpha;
        s->play.samplerate=sameleRate;
        //myAppl->play.bandwidth=bandwidth;

        if(s->pd.UsePlotScales)
        {
            s->pd.sPmin=dmin;
            s->pd.sPmax=dmax;
        }
        
        for(int n=0;n<s->length;++n){
            s->lreal[n]=0;
            s->limag[n]=0;
        }
        
        for(int y=0;y<s->water.ysize*2;++y){
            int ns=3*s->water.xsize*y;
            for(int n=0;n<s->length;++n){
                s->water.data[ns+3*n]=255;
                s->water.data[ns+3*n+1]=255;
                s->water.data[ns+3*n+2]=255;
            }
        }
        
        s->water.nline=0;
        
        fseek(s->play.infile, 0, SEEK_SET);
        
        s->play.frame=0;
        
        s->initPlay(&s->play);
        
        s->playFile(&s->play);
        
        
    }
    
    
	glutPostRedisplay();
}

EXTERN CWinPtr Root;
SdrFilePtr FindSceneSdrFile(int window)
{
    SdrFilePtr f;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSDR){
            f=(SdrFilePtr)w;
            if(f->dd.sub_window == window)return f;
        }
        w=w->CNext;
    }
    
    return NULL;
    
}
SdrFilePtr FindSdrFileWindow(int window)
{
    SdrFilePtr f;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSDR){
            f=(SdrFilePtr)w;
            if(f->window1 == window)return f;
        }
        w=w->CNext;
    }
    
    return NULL;
    
}




