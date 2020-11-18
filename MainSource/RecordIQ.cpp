//
//  RecordIQ.cpp
//  SdrGlut
//
//  Created by Dale Ranta on 3/12/19.
//  Copyright © 2019 Dale Ranta. All rights reserved.
//

#include <stdio.h>

#include "Radio.h"

#include "audiolib.h"

#include "Utilities.h"

#include "RecordIQ.h"

#include <GL/glui.h>
struct Radiodata{
    RadioPtr r;
    GLUI_EditText *filePath;
    FILE *out;
    int checkState;
};

static struct dataDialog{
    Radiodata *rd;
    int numradio;
    int nlast;
    GLUI *glui;
}dds;

static struct dataDialog *dd;

static void control_cb(int control);

static int closeDialog(struct dataDialog *dd);

static int setUpDialog(struct dataDialog *dd);

static int setUpRadio(struct Radiodata *rd,struct dataDialog *dd,int n);

static int getPointers(struct dataDialog *dd);

static void IQSave(struct Scene *scene,char *name);

int RecordIQ()
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

    dd->glui = GLUI_Master.create_glui("Record I/Q");
    
    for(int n=0;n<dd->numradio;++n){
        //fprintf(stderr,"name %s n %d r %p\n",r->windowName,n,r);
        setUpRadio(&dd->rd[n],dd,n);
    }

    new GLUI_Button(dd->glui, "Start Recording", 30000, control_cb);
    new GLUI_Button(dd->glui, "Stop Recording", 40000, control_cb);
    new GLUI_Button(dd->glui, "Close", 50000, control_cb);

    dd->glui->set_main_gfx_window(glutGetWindow());

    return 0;
}
static int setUpRadio(struct Radiodata *rd,struct dataDialog *dd,int n)
{
    RadioPtr r=rd->r;

    GLUI_Panel *panel3 = new GLUI_Panel(dd->glui, r->windowName);
    
    rd->checkState=1;
    
    new GLUI_Checkbox(panel3, "Record ", &rd->checkState, n, control_cb );
    
    rd->filePath =
    dd->glui->add_edittext_to_panel(panel3, "Path:", GLUI_EDITTEXT_TEXT, &r->scene->FilePathIQ);
    rd->filePath->w=200;

    new GLUI_Button(panel3, "Set Record I/Q File", n+dd->numradio, control_cb);

    return 0;
}
static void control_cb(int control)
{
    if(control == 50000)
    {
        for(int n=0;n<dd->numradio;++n){
            RadioPtr sdr=dd->rd[n].r;
            sdr->scene->FilePathIQ[0]=0;
            strncatToPath(sdr->scene->FilePathIQ,(char *)(dd->rd[n].filePath->get_text()),sizeof(sdr->scene->FilePathIQ));
        }
        if(dd->glui)dd->glui->close();
        dd->glui=NULL;
    }else if(control >= 0 && control <   dd->numradio){
        //fprintf(stderr,"flip check box %d\n",control);
    }else if(control >= dd->numradio && control <   2*dd->numradio){
        unsigned long freq,samp;
        char name[512];
        int n=control-dd->numradio;
        RadioPtr sdr=dd->rd[n].r;
        fprintf(stderr,"press button %d\n",n);
        freq=(unsigned long)(sdr->rx->fc);
        samp=(unsigned long)(sdr->rx->samplerate);
        sprintf(name,"%s_IQ_%lu_%lu_fc.raw",sdr->windowName,freq,samp);
        dd->nlast=n;
        dialogSaveC(sdr->scene, IQSave, 1, name);
    }else if(control == 30000){
        
        int ns=0;
        for(int n=0;n<dd->numradio;++n){   // open the files
            RadioPtr sdr=dd->rd[n].r;
            sdr->scene->FilePathIQ[0]=0;
            strncatToPath(sdr->scene->FilePathIQ,(char *)(dd->rd[n].filePath->get_text()),sizeof(sdr->scene->FilePathIQ));
            dd->rd[n].out=NULL;
            if(dd->rd[n].checkState && sdr->scene->FilePathIQ[0]){
                if(dd->rd[n].out){
                    fclose(dd->rd[n].out);
                }
                dd->rd[n].out=fopen(sdr->scene->FilePathIQ,"wb");
                if(dd->rd[n].out== NULL){
                    fprintf(stderr,"Error Opening %s to Write\n",sdr->scene->FilePathIQ);
                }else{
                    ++ns;
                    fprintf(stderr,"Open Record %d out %p\n",n,dd->rd[n].out);
                }
            }
        }
        if(ns == 0){
            fprintf(stderr,"No window found Ready To record\n");
            return;
        }
        for(int n=0;n<dd->numradio;++n){    // actually start recording
            RadioPtr sdr=dd->rd[n].r;
            if(dd->rd[n].out){
                sdr->rx->iqOutput=dd->rd[n].out;
                fprintf(stderr,"Start Record %d out %p\n",n,dd->rd[n].out);
            }
        }
    }else if(control == 40000){
        for(int n=0;n<dd->numradio;++n){   // stop recording
            RadioPtr sdr=dd->rd[n].r;
            if(dd->rd[n].out){
                sdr->rx->iqOutput=NULL;
            }
        }
        Sleep2(100);
        for(int n=0;n<dd->numradio;++n){    // close the files
            if(dd->rd[n].out){
                fclose(dd->rd[n].out);
                dd->rd[n].out=NULL;
            }
        }
    }
}
void IQSave(struct Scene *scene,char *name)
{
    
    if(GetWorking(scene->FilePathIQ,sizeof(scene->FilePathIQ))){
        strncatToPath(scene->FilePathIQ,name,sizeof(scene->FilePathIQ));
    }
    //fprintf(stderr,"FilePathIQ %s\n",scene->FilePathIQ);
    dd->rd[dd->nlast].filePath->set_text(scene->FilePathIQ);

}
EXTERN CWinPtr Root;
static int getPointers(struct dataDialog *dd)
{
    RadioPtr r;
    CWinPtr w;
    int numradio;
    
    dd->rd=(Radiodata *)cMalloc((unsigned long)(audio->numsource*sizeof(Radiodata)),8765);
    
    if(!dd->rd)return 1;
    
    zerol((char *)dd->rd,(unsigned long)(audio->numsource*sizeof(Radiodata)));
    
    numradio=0;
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeSdrRadio){
            r=(RadioPtr)w;
            dd->rd[numradio++].r=r;
        }
        w=w->CNext;
    }
    
    dd->numradio=numradio;
    
    return 0;
}
