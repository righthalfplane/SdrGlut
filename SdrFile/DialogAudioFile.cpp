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

#include "AudioFile.h"

#include "DialogSdrFile.h"
#include "LoadFiles.h"
#include "Utilities.h"
#include "SceneList.h"

#include "CLines.h"

extern "C" int closeScene(struct Scene *scene);

static void control_cb(int control);

int AudioFile::setDialogRange(double pmin,double pmax)
{
    
    return 0;
}

int AudioFile::setDialogPower(double power)
{
    
    return 0;
}


int AudioFile::setDialogFrame(float frame)
{
    if(!dd.line_scroll || !inDialog)return 0;
    
    dd.scroll=frame;

    dd.line_scroll->set_float_val(frame);
    
    msprintf(dd.text1z,sizeof(dd.text1z),"%.1f",(double)frame/(float)play.sfinfo.samplerate);

    dd.edittext1z[2]->set_text(dd.text1z);
    
//    printf("frame %d\n",frame);
 
    return 0;
}
int AudioFile::setDialogFrequency(double frequency)
{
    
    return 0;
}
int AudioFile::setDialogFc(double frequency)
{
    return 0;
}
int AudioFile::dialogSdrFile(struct Scene *scene)
{
	if(!scene)return 1;
	
	if(dd.glui){
		dd.glui->close();
	}
	
	dd.sceneLocal=scene;

    inDialog=1;
	
	dd.glui = GLUI_Master.create_glui(windowName);
			
    
    // dd.glui->add_column(true);
 
    
    dd.mute=0;
    
    dd.check_box = new GLUI_Checkbox( dd.glui, "Mute", &dd.mute, 7, control_cb );
    
    msprintf(dd.text5,sizeof(dd.text5),"%g",play.sfinfo.frames/(float)play.sfinfo.samplerate);


    
    GLUI_Panel *panel3 = new GLUI_Panel(dd.glui, "Time(sec)");
    
    dd.edittext5 =
    dd.glui->add_edittext_to_panel(panel3, "Final Time   :", GLUI_EDITTEXT_TEXT, dd.text5);
    dd.edittext5->w=300;

    dd.iic=2;
    msprintf(dd.text1z,sizeof(dd.text1z),"%.0f",0);
    dd.edittext1z[dd.iic] =
    dd.glui->add_edittext_to_panel(panel3, "Current Time:", GLUI_EDITTEXT_TEXT, dd.text1z );
    dd.edittext1z[dd.iic]->w=300;

    dd.line_scroll =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.scroll, 23, control_cb );
    
    dd.line_scroll->set_float_limits(0,(int)play.sfinfo.frames,GLUI_LIMIT_CLAMP);
    
    dd.line_scroll->w=300;

    dd.iic=0;
    panel3 = new GLUI_Panel(dd.glui, "Set Playback Time(sec)");
    dd.edittext1z[dd.iic] =
    dd.glui->add_edittext_to_panel(panel3, "Seconds", GLUI_EDITTEXT_TEXT, dd.text1z );
    dd.edittext1z[dd.iic]->w=160;
    new GLUI_Button(panel3, "Set Time", 888, control_cb);
    
    

    panel3 = new GLUI_Panel(dd.glui, "Audio Equalizer");
        
    GLUI_StaticText *p = new GLUI_StaticText(panel3, "-12 db    300    +12 db");
    p->set_alignment(GLUI_ALIGN_CENTER);

    dd.scroll1 =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.vscroll1, 2000, control_cb );
    dd.scroll1->set_text("Bar 1");
    
    dd.scroll1->set_float_limits(-12,12,GLUI_LIMIT_CLAMP);

    p = new GLUI_StaticText(panel3, "-12 db    500    +12 db");
    p->set_alignment(GLUI_ALIGN_CENTER);

    dd.scroll2 =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.vscroll2, 2001, control_cb );
    
    dd.scroll2->set_float_limits(-12,12,GLUI_LIMIT_CLAMP);

    p = new GLUI_StaticText(panel3, "-12 db    1K    +12 db");
    p->set_alignment(GLUI_ALIGN_CENTER);

    dd.scroll3 =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.vscroll3, 2002, control_cb );
    
    dd.scroll3->set_float_limits(-12,12,GLUI_LIMIT_CLAMP);

    p = new GLUI_StaticText(panel3, "-12 db    2K    +12 db");
    p->set_alignment(GLUI_ALIGN_CENTER);

    dd.scroll4 =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.vscroll4, 2003, control_cb );
    
    dd.scroll4->set_float_limits(-12,12,GLUI_LIMIT_CLAMP);

    p = new GLUI_StaticText(panel3, "-12 db    4K    +12 db");
    p->set_alignment(GLUI_ALIGN_CENTER);

    dd.scroll5 =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.vscroll5, 2004, control_cb );
    
    dd.scroll5->set_float_limits(-12,12,GLUI_LIMIT_CLAMP);
    
    p = new GLUI_StaticText(panel3, "-12 db    8K    +12 db");
    p->set_alignment(GLUI_ALIGN_CENTER);

    dd.scroll6 =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.vscroll6, 2005, control_cb );
    
    dd.scroll6->set_float_limits(-12,12,GLUI_LIMIT_CLAMP);
    
    p = new GLUI_StaticText(panel3, "-12 db    16K    +12 db");
    p->set_alignment(GLUI_ALIGN_CENTER);

    dd.scroll7 =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.vscroll7, 2006, control_cb );
    
    dd.scroll7->set_float_limits(-12,12,GLUI_LIMIT_CLAMP);

    panel3 = new GLUI_Panel(dd.glui, "Gain");

    
    msprintf(dd.text3,sizeof(dd.text3),"%g",play.gain);
    dd.edittext3 =
    dd.glui->add_edittext_to_panel(panel3,"Gain:", GLUI_EDITTEXT_TEXT, dd.text3);
    dd.edittext3->w=320;

    
    new GLUI_Button(panel3, "Set Gain", 4, control_cb);
    
    new GLUI_Button(dd.glui, "Play", 200, control_cb);
    
    new GLUI_Button(dd.glui, "Pause", 300, control_cb);
    
    new GLUI_Button(dd.glui, "Close", 2, control_cb);

    
    dd.sub_window=dd.glui->get_glut_window_id();
   
    dd.glui->set_main_gfx_window(dd.sub_window);

  //  printf("this %p sub_window %d\n",this,dd.sub_window);

	
	return 0;
}
static int setgain(float value,class Poly *p)
{
    if(value > -0.9 && value < 0.9){
        p->gain=0.25;
    }else if(value > 1){
        p->gain=0.25+value*0.5;
    }else if(value < -1.0){
        p->gain=0.25/fabs(value);
    }
    return 0;
}
static void control_cb(int control)
{
	std::string file_name;
    double gain;
    double sameleRate;
    int window;
    
    window=glutGetWindow();
    AudioFilePtr s=(AudioFilePtr)FindSceneAudioFile(window);
//    printf("control_cb %p %d\n",s,window);
    if(!s)return;
    
    
    struct DialogSdrData *dd=&s->dd;
	
    sscanf(dd->edittext3->get_text(),"%lg", &gain);
    sscanf(dd->edittext5->get_text(),"%lg", &sameleRate);
    
    if(control == 4){
        s->play.gain=gain;
    }
    else if(control == 2)
	{
        s->inDialog=0;
		dd->glui->close();
		dd->glui=NULL;
        closeScene(s->scene);
	}
    else if(control == 2000)
    {
       // printf("vscroll1 %f\n",dd->vscroll1);
        setgain(dd->vscroll1,s->p1);
    }
    else if(control == 2001)
    {
        //printf("vscroll2 %f\n",dd->vscroll2);
        setgain(dd->vscroll2,s->p2);
    }
    else if(control == 2002)
    {
     //   printf("vscroll3 %f\n",dd->vscroll3);
        setgain(dd->vscroll3,s->p3);
    }
    else if(control == 2003)
    {
     //   printf("vscroll4 %f\n",dd->vscroll4);
        setgain(dd->vscroll4,s->p4);
    }
    else if(control == 2004)
    {
      //  printf("vscroll5 %f\n",dd->vscroll5);
        setgain(dd->vscroll5,s->p5);
    }
    else if(control == 2005)
    {
      //  printf("vscroll6 %f\n",dd->vscroll6);
        setgain(dd->vscroll6,s->p6);
    }
    else if(control == 2006)
    {
       // printf("vscroll7 %f\n",dd->vscroll7);
        setgain(dd->vscroll7,s->p7);
    }
    else if(control == 200)
    {
        s->wait=0;
    }
    else if(control == 300)
    {
        s->wait=1;
    }
    else if(control == 23)
    {
      //  printf("23 dd.scroll %d\n",dd->scroll);
        s->play.frame = (int)dd->scroll/s->play.size;
        sf_count_t ret=sf_seek(s->play.infile, dd->scroll, SEEK_SET);
        if(ret < 0)printf("sf_seek error\n");
    
    }
    else if(control == 888)
    {
        float sec=0;
        sscanf(dd->edittext1z[0]->get_text(),"%f", &sec);
        
//        long frame=(long)((sec));
        
        //fprintf(stderr,"hours %.1f min %.1f sec %.1f frame %ld\n",hours,min,sec,frame);
        
        s->play.setFrameNumber=sec;
        
        s->play.controlProcess = -3;
        
    }else if(control == 7){
        s->mute=dd->mute;
//        printf("mute %p %d\n",s,s->mute);
    }else if(control == 8){
        s->play.frame=-1;
        s->stopPlay(&s->play);
                
        sf_seek(s->play.infile, 0, SEEK_SET);
        
        s->play.frame=0;
        
        s->initPlay(&s->play);
        
        s->playFile(&s->play);
        
    }

	glutPostRedisplay();
}

EXTERN CWinPtr Root;
AudioFilePtr FindSceneAudioFile(int window)
{
    AudioFilePtr f;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeAudio){
            f=(AudioFilePtr)w;
            if(f->dd.sub_window == window)return f;
        }
        w=w->CNext;
    }
    
    return NULL;
    
}
AudioFilePtr FindAudioFileWindow(int window)
{
    AudioFilePtr f;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeAudio){
            f=(AudioFilePtr)w;
            if(f->window1 == window)return f;
        }
        w=w->CNext;
    }
    
    return NULL;
    
}




