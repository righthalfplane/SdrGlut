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


int AudioFile::setDialogFrame(int frame)
{
    if(!dd.line_scroll || !inDialog)return 0;
    
    dd.scroll=frame;

    dd.line_scroll->set_float_val(frame);
    
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

    dd.edittext5 =
    dd.glui->add_edittext( "Time(sec):", GLUI_EDITTEXT_TEXT, dd.text5);
    dd.edittext5->w=200;

    
    GLUI_Panel *panel3 = new GLUI_Panel(dd.glui, "Time");

    dd.line_scroll =
    new GLUI_Scrollbar( panel3, "Time", GLUI_SCROLL_HORIZONTAL,
                       &dd.scroll, 23, control_cb );
    
    dd.line_scroll->set_float_limits(0,(int)play.sfinfo.frames,GLUI_LIMIT_CLAMP);

    dd.iic=0;
    panel3 = new GLUI_Panel(dd.glui, "Set Playback Time");
    dd.edittext1z[dd.iic] =
    dd.glui->add_edittext_to_panel(panel3, "Seconds", GLUI_EDITTEXT_TEXT, dd.text1z );
    dd.edittext1z[dd.iic]->w=160;
    new GLUI_Button(panel3, "Set Time", 888, control_cb);
    
    
    msprintf(dd.text3,sizeof(dd.text3),"%g",play.gain);
    dd.edittext3 =
    dd.glui->add_edittext( "Gain:", GLUI_EDITTEXT_TEXT, dd.text3);
    dd.edittext3->w=200;

    
    new GLUI_Button(dd.glui, "Set Gain", 4, control_cb);
    
    new GLUI_Button(dd.glui, "Play", 200, control_cb);
    
    new GLUI_Button(dd.glui, "Pause", 300, control_cb);
    
    new GLUI_Button(dd.glui, "Close", 2, control_cb);

    
    dd.sub_window=dd.glui->get_glut_window_id();
   
    dd.glui->set_main_gfx_window(dd.sub_window);

  //  printf("this %p sub_window %d\n",this,dd.sub_window);

	
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
        
        long frame=(long)((sec));
        
        //fprintf(stderr,"hours %.1f min %.1f sec %.1f frame %ld\n",hours,min,sec,frame);
        
        s->play.setFrameNumber=frame;
        
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




