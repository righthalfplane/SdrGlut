/*
 *  DialogFolder.cpp
 *  
 *
 *  Created by Dale Ranta on 8/14/13.
 *  Copyright 2013 Dale Ranta. All rights reserved.
 *
 */

#include "firstFile.h"
#include <cstdio>
#include <cstdlib>

#include "DialogFolder.h"
#include "ulibTypes.h"
#include "Utilities.h"
#include "SceneList.h"

#include <GL/glui.h>

static GLUI *glui;

static char text1b[255]="New Folder";

static GLUI_EditText *edittext1b;

static void control_cb(int control);

static struct Scene *sceneLocal;

extern "C" int goCD(char *name);

static void (*callBack)(struct Scene *scene);

int dialogFolder(struct Scene *scene,void (*callBacki)(struct Scene *scene))
{
	
	
	if(glui){
		glui->close();
	}
	
	callBack=callBacki;
	
	sceneLocal=scene;
	
	glui = GLUI_Master.create_glui( "New Folder" );
	
	GLUI_Panel *obj_panel =  glui->add_panel( "Folder Name" );
	
	
	edittext1b =
	glui->add_edittext_to_panel(obj_panel, "Name :", GLUI_EDITTEXT_TEXT, text1b );
	edittext1b->w=280;
	glui->deactivate_current_control();
	glui->activate_control( edittext1b, GLUI_ACTIVATE_TAB );
		
	new GLUI_Button(glui, "Create", 3, control_cb); 
	new GLUI_Button(glui, "Cancel", 2, control_cb); 
	
	// glui->set_main_gfx_window( glutGetWindow() );
		
	return 0;
}
static void control_cb(int control) 
{
	std::string file_name;
	
	if(sceneLocal){
		if(SceneSetWindow(sceneLocal)){
			WarningPrint("Window Not Found - Dialog Closed\n");
			glui->close();
			glui=NULL;
			return;
		}	
	}
	if(control == 2)
	{
		glui->close();
		glui=NULL;
	}
	else if(control == 3)
	{
		
		if(MakeNewDirectory((char *)(edittext1b->get_text())))
		{
			WarningPrint("MakeNewDirectory %s Failed\n",edittext1b->get_text());
		}
		
		if(callBack)(*callBack)(sceneLocal);
		
		glui->close();
		glui=NULL;
	}

}


