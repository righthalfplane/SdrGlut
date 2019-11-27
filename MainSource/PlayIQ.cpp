//
//  RecordIQ.cpp
//  SdrGlut
//
//  Created by Dale Ranta on 3/12/19.
//  Copyright © 2019 Dale Ranta. All rights reserved.
//

#include <stdio.h>

#include "SdrFile.h"

#include "audiolib.h"

#include "Utilities.h"

#include "PlayIQ.h"

#include <GL/glui.h>
struct Radiodata{
    SdrFilePtr r;
    GLUI_EditText *filePath;
    GLUI_EditText *hours;
    GLUI_EditText *minutes;
    GLUI_EditText *seconds;
    FILE *out;
    long frame;
    int checkState;
};

static struct dataDialog{
    Radiodata *rd;
    int numradio;
    int nlast;
    GLUI *glui;
    char text1z[255];
}dds;

static struct dataDialog *dd;

static void control_cb(int control);

static int closeDialog(struct dataDialog *dd);

static int setUpDialog(struct dataDialog *dd);

static int setUpRadio(struct Radiodata *rd,struct dataDialog *dd,int n);

static int getPointers(struct dataDialog *dd);

int PlayIQ()
{
    
    dd=&dds;
    
    if(dd->glui){
        closeDialog(dd);
    }

    setUpDialog(dd);

    return 0;

}

static int closeDialog(struct dataDialog *dd)
{
    if(dd->rd)cFree((char *)dd->rd);
    dd->rd=NULL;
    dd->numradio=0;
    
    dd->glui->close();
    dd->glui=NULL;
    return 0;
}
static int setUpDialog(struct dataDialog *dd)
{
    
    if(getPointers(dd))return 1;

    dd->glui = GLUI_Master.create_glui("Playback I/Q");
    
    for(int n=0;n<dd->numradio;++n){
       setUpRadio(&dd->rd[n],dd,n);
    }

    new GLUI_Button(dd->glui, "Start Playback", 30000, control_cb);
    new GLUI_Button(dd->glui, "Close", 50000, control_cb);

    dd->glui->set_main_gfx_window(glutGetWindow());

    return 0;
}
static int setUpRadio(struct Radiodata *rd,struct dataDialog *dd,int n)
{
    SdrFilePtr r=rd->r;

    GLUI_Panel *panel3 = new GLUI_Panel(dd->glui, r->windowName);
    
    rd->checkState=1;
    
    new GLUI_Checkbox(panel3, "Playback ", &rd->checkState, n, control_cb );
    
    msprintf(dd->text1z,sizeof(dd->text1z),"%d",0);
    
    rd->hours =
    dd->glui->add_edittext_to_panel(panel3, "Hours:", GLUI_EDITTEXT_TEXT, dd->text1z);
    rd->hours->w=200;
    
    rd->minutes =
    dd->glui->add_edittext_to_panel(panel3, "Minutes:", GLUI_EDITTEXT_TEXT, dd->text1z);
    rd->minutes->w=200;
    
    rd->seconds =
    dd->glui->add_edittext_to_panel(panel3, "Seconds:", GLUI_EDITTEXT_TEXT, dd->text1z);
    rd->seconds->w=200;
    
    return 0;
}
static void control_cb(int control)
{
    if(control == 50000)
    {
        if(dd->glui)dd->glui->close();
        dd->glui=NULL;
    }else if(control >= 0 && control <   dd->numradio){
        fprintf(stderr,"flip check box %d\n",control);
    }else if(control >= dd->numradio && control <   2*dd->numradio){
        fprintf(stderr,"press button %d\n",control);
    }else if(control == 30000){
        int ns=0;
        for(int n=0;n<dd->numradio;++n){   // open the files
            struct Radiodata *rd=&dd->rd[n];
            if(rd->checkState){
                float hours=0;
                sscanf(rd->hours->get_text(),"%f", &hours);
                float min=0;
                sscanf(rd->minutes->get_text(),"%f", &min);
                float sec=0;
                sscanf(rd->seconds->get_text(),"%f", &sec);
                
                rd->frame=(long)((3600*hours+60*min+sec)*10+0.5);

                fprintf(stderr,"hours %.1f min %.1f sec %.1f frame %ld\n",hours,min,sec,rd->frame);

                ++ns;
            }
        }
        if(ns == 0){
            fprintf(stderr,"No window found Ready To play\n");
            return;
        }
        for(int n=0;n<dd->numradio;++n){    // actually start recording
            struct Radiodata *rd=&dd->rd[n];
            SdrFilePtr s=rd->r;
            if(rd->checkState){
                s->play.setFrameNumber=rd->frame;
                s->play.controlProcess = -3;
            }
        }
    }
}
EXTERN CWinPtr Root;
static int getPointers(struct dataDialog *dd)
{
    SdrFilePtr r;
    CWinPtr w;
    int numradio;
    
    dd->rd=(Radiodata *)cMalloc((unsigned long)(audio->numsource*sizeof(Radiodata)),8765);
    
    if(!dd->rd)return 1;
    
    zerol((char *)dd->rd,(unsigned long)(audio->numsource*sizeof(Radiodata)));
    
    numradio=0;
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSDR){
            r=(SdrFilePtr)w;
            dd->rd[numradio++].r=r;
        }
        w=w->CNext;
    }
    
    dd->numradio=numradio;
    
    return 0;
}
