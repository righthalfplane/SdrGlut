/*
 *  CLinesPlotBox.cpp
 *  Rx3
 *
 *  Created by Dale Ranta on 2/13/14.
 *  Copyright 2014 Dale Ranta. All rights reserved.
 *
 */

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
	
	struct CLines::LineStruct *dataLocal;
	
	char text1a[255];
	GLUI_EditText *edittext1a;
	
	char text1b[255];
	GLUI_EditText *edittext1b;
	
	char text1c[255];
	GLUI_EditText *edittext1c;
	
	char text1d[255];
	GLUI_EditText *edittext1d;
	
  	int sub_window;
	
};

static struct PlotAttributesData paps;

static void control_cb(int control);

static int getDialogData(struct PlotAttributesData *pap);

int CLines::PlotBox(struct Scene *scene, struct LineStruct *lines)
{
	struct PlotAttributesData *pap;
	
	if(!scene)return 1;
	
	
	pap=&paps;
	
	pap->scene=scene;
	
	pap->dataLocal=lines;
	
	pap->glui = GLUI_Master.create_glui( "Plot Box" );
	
	
	GLUI_Panel *obj_panel =  pap->glui->add_panel( "Box" );
		
	pap->edittext1a =
	pap->glui->add_edittext_to_panel(obj_panel, "x :", GLUI_EDITTEXT_TEXT, pap->text1a );
	pap->edittext1a->w=160;
	
	pap->edittext1b =
	pap->glui->add_edittext_to_panel(obj_panel, "y :", GLUI_EDITTEXT_TEXT, pap->text1b );
	pap->edittext1b->w=160;
	
	pap->edittext1c =
	pap->glui->add_edittext_to_panel(obj_panel, "xsize :", GLUI_EDITTEXT_TEXT, pap->text1c );
	pap->edittext1c->w=160;
	
	pap->edittext1d =
	pap->glui->add_edittext_to_panel(obj_panel, "ysize :", GLUI_EDITTEXT_TEXT, pap->text1d );
	pap->edittext1d->w=160;
	
		
	new GLUI_Button(pap->glui, "Apply", 9, control_cb); 
	
	new GLUI_Button(pap->glui, "Close", 10, control_cb); 
	
	pap->sub_window=glutGetWindow();
	
	pap->glui->set_main_gfx_window( pap->sub_window );
	
	getDialogData(pap);
	
	return 0;
}
static int getDialogData(struct PlotAttributesData *pap)
{
	
	uGridPlotPtr Plot;
	char value[256];
	
	
	Plot=pap->dataLocal->Plot;
	if(!Plot)return 1;

	msprintf(value,sizeof(value),"%g",Plot->box.x);
	pap->edittext1a->set_text(value);
	msprintf(value,sizeof(value),"%g",Plot->box.y);
	pap->edittext1b->set_text(value);
	
	msprintf(value,sizeof(value),"%g",Plot->box.xsize);
	pap->edittext1c->set_text(value);
	msprintf(value,sizeof(value),"%g",Plot->box.ysize);
	pap->edittext1d->set_text(value);
	
	return 0;
	
}
static int setDialogData(struct PlotAttributesData *pap)
{
	uGridPlotPtr Plot;
	
	Plot=pap->dataLocal->Plot;
	if(!Plot)return 1;
	
	sscanf(pap->edittext1a->get_text(),"%lg", &Plot->box.x);
	
	sscanf(pap->edittext1b->get_text(),"%lg", &Plot->box.y);
	
	sscanf(pap->edittext1c->get_text(),"%lg", &Plot->box.xsize);
	
	sscanf(pap->edittext1d->get_text(),"%lg", &Plot->box.ysize);
		
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
	
	if(control == 1)
	{
		getDialogData(pap);
	}
	else if(control == 9)
	{
		setDialogData(pap);
	}
	else if(control == 10)
	{
		pap->glui->close();
	}
}

