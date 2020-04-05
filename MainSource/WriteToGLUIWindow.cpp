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

#include <GL/glui.h>

extern "C" int WriteToGLUIWindow(char *message);


static int gluiID = -1;

static GLUI_TextBox *moo;

static void textbox_cb(GLUI_Control *control) {
    /* printf("Got textbox callback\n"); */
}

static int SetInsertQ(struct QStruct *q);

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
		
		glutAddMenuEntry("Save", 32);
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
