/*
 *  QuitDialog.cpp
 *  
 *
 *  Created by Dale Ranta on 10/31/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */

#include "firstFile.h"
#include "QuitDialog.h"

#include <cstdio>
#include <cstdlib>

#include "DialogSave.h"
#include "ulibTypes.h"
#include "audiolib.h"

#include <GL/glui.h>

static void control_cb(int control);

static GLUI *glui;

int dialogQuit(void);

extern "C" void checkall(void);

int mainClean(void);

int dialogQuitC(void)
{
	return dialogQuit();
}
int dialogQuit(void)
{

	if(glui){
		glui->close();
	}
	
	glui = GLUI_Master.create_glui( "Quit ?" );
	
	glui->add_statictext( "Do You Really Want To Quit ?");
	
	new GLUI_Button(glui, "Yes", 1, control_cb); 
	
	new GLUI_Button(glui, "No", 2, control_cb); 
	
	
	return 0;
}
static void control_cb(int control) 
{
	if(control == 1)
	{
		//FileManagerExit(1);
		mainClean();
		checkall();
        stopAudio(audio);
        glui=NULL;
		exit(0);
	}
	else if(control == 2)
	{
		glui->close();
		glui=NULL;
	}
}
