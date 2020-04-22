/*
 *  DialogFileOpen.cpp
 *  
 *
 *  Created by Dale Ranta on 3/3/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
#include "firstFile.h"
#include <cstdio>
#include <cstdlib>

#include "DialogFileOpen.h"
#include "LoadFiles.h"
#include "ulibTypes.h"
#include "Utilities.h"
#include "BatchFile.h"

#include <GL/glui.h>

static GLUI_FileBrowser *fb;

static GLUI *glui;

static int type;

static int boxSelection;

static void control_cb(int control);

static struct Scene *sceneLocal;

extern int doCommands(int command);

static GLUI_Listbox  *box;

static void fillbox(void);

static int listBoxIdMax=-1;

extern "C" int SaveFluencePath(struct Scene *scene,char *path);

int doSdrFileOpen(char *name);

int doFrequnecyFile(char *name);

int dialogFileOpen(struct Scene *scene)
{


	if(glui){
		glui->close();
	}
	
	listBoxIdMax=-1;
	
	sceneLocal=scene;
	
	glui = GLUI_Master.create_glui( "Open File" );
	
	GLUI_Panel *obj_panel =  glui->add_panel( "Open Files" );
	
	box=glui->add_listbox_to_panel(obj_panel, "                              ",&boxSelection, 10, control_cb);
	
	box->set_w(480);

	fb = new GLUI_FileBrowser(obj_panel, "", false, 1,control_cb);
	fb->set_h(280);
	fb->set_w(280);
	
	fb->fillbox=fillbox;
	
	glui->add_column(true);
	
	type=0; /* For the distribution vesion */
	

	obj_panel =  glui->add_panel( "Options" );
	    	
	GLUI_RadioGroup *group1 =
	glui->add_radiogroup_to_panel(obj_panel,&type,4,control_cb);
    
    glui->add_radiobutton_to_group( group1, "FileType SDR File" );
    
    glui->add_radiobutton_to_group( group1, "FileType Frequency File" );

	GLUI_Button *b=new GLUI_Button(glui, "Open", 3, control_cb); 
	b->set_w(120);
		
	b=new GLUI_Button(glui, "Cancel",  2, control_cb); 
	b->set_w(120);
	
	fillbox();
	
	glui->set_main_gfx_window(glutGetWindow());
	
	return 0;
}
static void control_cb(int control) 
{
	std::string file_name;
	
	if(control == 1)
	{
		file_name = "";
		file_name = fb->get_file();
		// LoadFiles (sceneLocal,(char *)file_name.c_str(),type+1);
		glui->close();
		glui=NULL;
	}
	else if(control == 2)
	{
		glui->close();
		glui=NULL;
	}
	else if(control == 3)
	{
		file_name = "";
		int this_item;
		const char *selected;
		
		
		this_item = fb->list->get_current_item();
		if (this_item > 0) { /* file or directory selected */
			selected = fb->list->get_item_ptr( this_item )->text.c_str();
			file_name = selected;
		}
		//LoadFiles (sceneLocal,(char *)file_name.c_str(),type+1);
        
        // fprintf(stderr,"Name %s %p len %lu\n",(char *)file_name.c_str(),file_name.c_str(),strlen((char *)file_name.c_str()));
        
        
        if(type == 0){
		  int glu = glutGetWindow();

          doSdrFileOpen((char *)file_name.c_str());

		  glutSetWindow(glu);
        }else if(type == 1){
            doFrequnecyFile((char *)file_name.c_str());
        }
        glui->close();
		glui=NULL;
	}
	else if(control == 10)
	{
		
		for(int n=0; n<boxSelection; ++n){
		   goCD((char *)"../");
		}
		
		fillbox();
		
		fb->fbreaddir(".");
	}
}
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
