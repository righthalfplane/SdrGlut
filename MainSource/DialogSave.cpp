/*
 *  DialogSave.cpp
 *  
 *
 *  Created by Dale Ranta on 10/25/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
 
#include "firstFile.h"
#include <cstdio>
#include <cstdlib>

#include "DialogSave.h"
#include "DialogFolder.h"

#include "ulibTypes.h"
#include "SceneList.h"
#include "Utilities.h"

#include <GL/glui.h>

static GLUI_FileBrowser *fb;

static GLUI *glui;

static int type;

static int boxSelection;

static char text1b[255]="saveAudio.raw";
static char text2b[255]="savefile.iq";
static char text3b[255]="saveFluence.fbl";
static char text4b[255]="doc.kml";
static char text5b[255]="aresPreferences.pref";

static GLUI_EditText *edittext1b;


static void control_cb(int control);

static struct Scene *sceneLocal;

static void (*callBack)(struct Scene *scene,char *name);

static int datatype;

static GLUI_Listbox  *box;

static void fillbox(void);

static int listBoxIdMax=-1;


int dialogSave(struct Scene *scene);

extern "C" int goCD(char *name);

int dialogSaveC(struct Scene *scene, void (*callBacki)(struct Scene *scene,char *name), int typei,char *namein)
{
	datatype=typei;
	callBack=callBacki;
    if(namein){
        mstrncpy(text2b,(char *)namein,sizeof(text2b));
    }
	return dialogSave(scene);
}
int dialogSave(struct Scene *scene)
{
	char *tpointer;
		
	if(glui){
		glui->close();
	}
	
	listBoxIdMax=-1;
	
	sceneLocal=scene;
	
	glui = GLUI_Master.create_glui( "Save File" );
	
	GLUI_Panel *obj_panel =  glui->add_panel( "Save File" );

	box=glui->add_listbox_to_panel(obj_panel, "                              ",&boxSelection, 10, control_cb);
	
	box->set_w(480);
	
	fb = new GLUI_FileBrowser(obj_panel, "", false, 1,control_cb);
	fb->set_h(380);
	fb->set_w(580);
	fb->fillbox=fillbox;
	
	if(datatype == 0){
	   tpointer=text1b;
	}else if(datatype == 2){
		tpointer=text3b;
	}else if(datatype == 3){
		tpointer=text4b;
	}else if(datatype == 4){
		tpointer=text5b;
	}else{
	   tpointer=text2b;
	}
	
	edittext1b =
	glui->add_edittext_to_panel(obj_panel, "Name :", GLUI_EDITTEXT_TEXT, tpointer );
	edittext1b->w=280;
	
	glui->add_column(true);
	
	new GLUI_Button(glui, "New", 3, control_cb); 
	new GLUI_Button(glui, "Save", 4, control_cb); 
	new GLUI_Button(glui, "Cancel", 2, control_cb); 
	
	// glui->set_main_gfx_window( glutGetWindow() );
	
	fillbox();
	
	return 0;
}
extern "C" char *strsave(char *s,int tag);
static void fillbox()
{
	char Directory[1024];
	char name[1024];
	char *list[2048];
	char *np,c;
	unsigned int n;
	int nn,ni,k;
	
	
	if(!GetWorking(Directory,sizeof(Directory))){
		WarningPrint("fillbox  Working directory error\n");
		return;
	}
	
	n=0;
	ni=0;
	np=name;
	while((c=Directory[n++]) && n < sizeof(Directory)){
	    if(c == FILE_NAME_SEPERATOR_CHAR){
		  if(np != name){
		     *np=0;
			 list[ni++]=strsave(name,9827);
		  }
		  np=name;
		  continue;
		}
		*np++ = c;
	}
	
	if(np != name){
		*np=0;
		list[ni++]=strsave(name,9828);
	}
	
	for(nn=0;nn<listBoxIdMax;++nn){
		box->delete_item(nn);
	}
	
	k=0;
	for(nn=ni-1;nn>=0;--nn){
	   box->add_item(k++,list[nn]);
	   cFree((char *)list[nn]);
	}
	
	listBoxIdMax=k;

}
static void callMe(struct Scene *scene)
{
	
	if(scene != sceneLocal)return;
	
	if(!fb)return;
	
	fillbox();
	
	fb->fbreaddir(".");
	
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
	if(control == 1)
	{
		file_name = "";
		file_name = fb->get_file();
		WarningPrint("control %d filename %s type %d\n",control,file_name.c_str(),type+1);
		//LoadFiles (sceneLocal,(char *)file_name.c_str(),type+1);
		glui->close();
		//doCommands(1);
		glui=NULL;
		fb=NULL;
	}
	else if(control == 2)
	{
		glui->close();
		glui=NULL;
		fb=NULL;
	}
	else if(control == 3)
	{
		dialogFolder(sceneLocal,callMe);
		
	}
	else if(control == 4)
	{
		/* WarningPrint("Save %s\n",edittext1b->get_text()); */
		glui->close();
		if(callBack)(*callBack)(sceneLocal,(char *)edittext1b->get_text());
		glui=NULL;
		fb=NULL;
	}
	else if(control == 10)
	{
		/* WarningPrint("boxSelection %d\n",boxSelection); */
		
		for(int n=0; n<boxSelection; ++n){
		   goCD((char *)"../");
		}
		
		fillbox();
		
		fb->fbreaddir(".");
	}
}
