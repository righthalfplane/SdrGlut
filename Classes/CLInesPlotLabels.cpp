/*
 *  CLInesPlotLabels.cpp
 *  Rx3
 *
 *  Created by Dale Ranta on 2/15/14.
 *  Copyright 2014 Dale Ranta. All rights reserved.
 *
 */

#include "CLInesPlotLabels.h"
#include "CLinesPlotBox.h"

#include "firstFile.h"

#include <stdio.h>

#include "Utilities.h"
#include "ClinesPlotAttributes.h"

#include <cstdio>
#include <cstdlib>

#include <GL/glui.h>

struct PlotAttributesData{
	
	GLUI *glui;
	
	struct Scene *scene;
	
	struct CLines::LineStruct *lines;
	
	GLUI_EditText *edittext[5];
		
  	int sub_window;
	
};

static struct PlotAttributesData paps;

static void control_cb(int control);

int CLines::PlotLabels(struct Scene *scene, struct LineStruct *lines)
{
	char *names[5]={(char *)"Heading1 :",(char *)"X-Label1 :",(char *)"Heading2 :",(char *)"X-Label2 :",(char *)"Y-Label  :"};
	struct PlotAttributesData *pap;
	
	if(!scene)return 1;
	
	
	pap=&paps;
	
	pap->scene=scene;
	
	pap->lines=lines;
	
	pap->glui = GLUI_Master.create_glui( "Plot Labels" );
	
	
	GLUI_Panel *obj_panel =  pap->glui->add_panel( "Labels" );
	

	for(int n=0;n<5;++n){
		pap->edittext[n] =
		pap->glui->add_edittext_to_panel(obj_panel,names[n], GLUI_EDITTEXT_TEXT, lines->labels.Labels[n] );
		pap->edittext[n]->w=560;
	}
		
	
	new GLUI_Button(pap->glui, "Apply", 9, control_cb); 
	
	new GLUI_Button(pap->glui, "Close", 10, control_cb); 
	
	pap->sub_window=glutGetWindow();
	
	pap->glui->set_main_gfx_window( pap->sub_window );
		
	return 0;
}
static int setDialogData(struct PlotAttributesData *pap)
{
	struct CLines::LineStruct *lines;
	
	lines=pap->lines;
	if(!lines)return 1;
	
	for(int n=0;n<5;++n){
		mstrncpy(&lines->labels.Labels[n][0],(char *)pap->edittext[n]->get_text(),sizeof(lines->labels.Labels[0]));
		if(strlen(&lines->labels.Labels[n][0]) >= 1){
			lines->labels.flag[n]=1;
		}else{
			lines->labels.flag[n]=0;
		}
	}
	
	glutPostRedisplay();
	
	return 0;
	
}
static void control_cb(int control) 
{
	struct PlotAttributesData *pap;
	
	pap=&paps;
	
	CLinesPtr lines=(CLinesPtr)FindScene(pap->scene);
	if(!lines){
		pap->glui->close();
		return;
	}
	
	setDialogData(pap);	
	
	if(control == 9)
	{
		setDialogData(pap);
	}
	else if(control == 10)
	{
		pap->glui->close();
	}
}

