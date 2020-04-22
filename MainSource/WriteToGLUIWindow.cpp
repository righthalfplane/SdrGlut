/*
 *  WriteToGLUIWindow.cpp
 *  FireBall
 *
 *  Created by Dale Ranta on 8/28/12.
 *  Copyright 2012 SAIC. All rights reserved.
 *
 */

#include "firstFile.h"
#include "DialogSave.h"
#include <cstdio>
#include <cstdlib>
#include <stdarg.h>
/* #define EXTERN  */

#include "BackgroundQ.h"
#include "LoadFiles.h"
#include "CWindow.h"

#include <GL/glui.h>

extern "C" int WriteToGLUIWindow(char *message);


static int gluiID = -1;

static GLUI_TextBox *moo;

static void textbox_cb(GLUI_Control *control) {
    /* printf("Got textbox callback\n"); */
}

static int SetInsertQ(struct QStruct *q);


int doFrequnecyFile(char *name);

int doFrequnecyFile(char *path)
{
    char buff[5120],word[20000];
    FILE *inout;
    int n,m,k,itWas,c;

    if(!path)return 1;
    
    if((inout=fopen(path,"rb")) == NULL){
        sprintf(buff,"doFrequnecyFile Cannot open file : %s to read%c\n",path,0);
        fprintf(stderr,"%s",buff);
        return 1;
    }

    itWas = -7777;
    k=0;
    while((m=(int)fread(buff,1,5120,inout)) > 0){
        for(n=0;n<m;++n){
            c=buff[n];
            if(c == '\n' || c == '\r' || (k >= 19998)){
                if((c == '\n') && (itWas == '\r')){
                    continue;
                }
                word[k++]='\n';
                word[k++]='\0';
                WriteToGLUIWindow(word);
                k=0;
            }else{
                word[k++]=buff[n];
            }
            itWas = c;
        }
    }
    if(k != 0){
        word[k++]='\n';
        word[k++]='\0';
        WriteToGLUIWindow(word);
    }

    if(inout)fclose(inout);
    
    return 0;
}
static void SaveIt(struct Scene *scene,char *name)
{
	FILE *out;
	GLUI *glui;
	
	glui = GLUI_Master.find_glui_by_window_id(gluiID);
	
	if(glui){
		if(moo){
			const char *text=moo->get_text();
			if(text){
				out=fopen(name,"w");
				if(out){
					fprintf(out,"%s",text);
					fclose(out);
				}
			}
		}
	}
}
static void menu_select(int item)
{
	GLUI *glui;
	if(item == 32){
			
		dialogSaveC(NULL,SaveIt,3,NULL);
	
		return;
		
	}if(item == 33){
		glui = GLUI_Master.find_glui_by_window_id(gluiID);
		
		if(glui){
			glui->close();
		}
		
		moo=NULL;
		
		gluiID = -1;
    }else if(item == 31){
        char buff[256];
        int n;
        glui = GLUI_Master.find_glui_by_window_id(gluiID);
        
        const char *test=moo->get_text();
        
        int start=moo->sel_start;
        int end=moo->sel_end;
        if(start > end){
            n=start;
            start=end;
            end=n;
        }
        n=0;
        for(int k=start;k<end;++k){
            buff[n++]=test[k];
            if(n > 254)break;
        }
        buff[n++]=0;
        int n1=-1;
        int n2=0;
        for(int k=0;k<n;++k){
            if(n1 == -1 && buff[k] == ','){
                n1=k+1;
            }else if(n1 > -1 && buff[k] == ','){
                buff[k]=0;
                n2=k+1;
                break;
          }
        }

       // fprintf(stderr,"%d %d buff %s\n",moo->sel_start,moo->sel_end,buff);
        
        sendMessageGlobal(&buff[n1],&buff[n2],M_SEND);
    }
}

int WriteToGLUIWindow(char *message)
{
	GLUI *glui;

	if(!message)return 1;
	
	glui = GLUI_Master.find_glui_by_window_id(gluiID);
	
	if(!glui){
		int window;
		
		
		glui = GLUI_Master.create_glui("BatchPrint", 0);
		if(!glui)return 1;
		gluiID=glui->get_glut_window_id();
		glui->set_main_gfx_window(glutGetWindow());
		GLUI_Panel *ep = new GLUI_Panel(glui,"",true);
		moo = new GLUI_TextBox(ep,true,1,textbox_cb);
		moo->set_text(message);
		moo->set_h(400);
		moo->set_w(610);
		
		window=glutGetWindow();
		
		glutSetWindow(gluiID);
		
		glutCreateMenu(menu_select);
		
        glutAddMenuEntry("Set Frequency", 31);
        glutAddMenuEntry("Save", 32);
        glutAddMenuEntry("-------------", 34);
        glutAddMenuEntry("Close", 33);

		glutAttachMenu(GLUT_RIGHT_BUTTON);
		
		glutSetWindow(window);

	}else{
		if(moo)moo->append_text(message);
		/* qBackground(NULL,(int (*)(void *))SetInsertQ); */

	}
  
	return 0;
}
static int SetInsertQ(struct QStruct *q)
{
	GLUI *glui;
	
	glui = GLUI_Master.find_glui_by_window_id(gluiID);

	if(glui)moo->insertion_pt = -1;
	
ErrorOut:
    if(q)q->launched =  -1;
	return 0;
}
