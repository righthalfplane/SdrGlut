/*
 *  ClinesPlotAttributes.cpp
 *  Rx3
 *
 *  Created by Dale Ranta on 1/31/14.
 *  Copyright 2014 Dale Ranta. All rights reserved.
 *
 */


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
	
	GLUI_RadioGroup *group1;
	
	int axistype;
	
	GLUI_RadioGroup *group2;
	
	int scaletype;
	
	char text1a[255];
	GLUI_EditText *edittext1a;
	
	char text1b[255];
	GLUI_EditText *edittext1b;
	
	char text1c[255];
	GLUI_EditText *edittext1c;
	
	char text1d[255];
	GLUI_EditText *edittext1d;
	
	char text1e[255];
	GLUI_EditText *edittext1e;
	
	char text1f[255];
	GLUI_EditText *edittext1f;
	
	GLUI_Checkbox *min_box;
	
	GLUI_Checkbox *max_box;
	
	int autoMin,autoMax;
	
  	int sub_window;
	
};

static struct PlotAttributesData paps;

static void control_cb(int control);

static int getDialogData(struct PlotAttributesData *pap);

int CLines::PlotAttributes(struct Scene *scene, struct LineStruct *lines)
{
	struct PlotAttributesData *pap;
	
	if(!scene)return 1;
	
	
	pap=&paps;
		
	pap->scene=scene;
	
	pap->dataLocal=lines;
	
	pap->glui = GLUI_Master.create_glui( "Plot Attributes" );
	
	
	GLUI_Panel *obj_panel =  pap->glui->add_panel( "Axis" );
	
	pap->group1 =
	pap->glui->add_radiogroup_to_panel(obj_panel,&pap->axistype,1,control_cb);
	
	pap->glui->add_radiobutton_to_group( pap->group1, "x-axis" );
	
	pap->glui->add_radiobutton_to_group( pap->group1, "y-axis" );
	
	obj_panel =  pap->glui->add_panel( "Steps" );
	
	pap->edittext1a =
	pap->glui->add_edittext_to_panel(obj_panel, "Mimimum :", GLUI_EDITTEXT_TEXT, pap->text1a );
	pap->edittext1a->w=160;
	
	pap->edittext1b =
	pap->glui->add_edittext_to_panel(obj_panel, "Maximum :", GLUI_EDITTEXT_TEXT, pap->text1b );
	pap->edittext1b->w=160;
	
	pap->edittext1c =
	pap->glui->add_edittext_to_panel(obj_panel, "Major Step :", GLUI_EDITTEXT_TEXT, pap->text1c );
	pap->edittext1c->w=160;
	
	pap->edittext1d =
	pap->glui->add_edittext_to_panel(obj_panel, "Minor Step :", GLUI_EDITTEXT_TEXT, pap->text1d );
	pap->edittext1d->w=160;
	
	
	obj_panel =  pap->glui->add_panel( "Tick Count Limits" );
	
	pap->edittext1e =
	pap->glui->add_edittext_to_panel(obj_panel, "Major :", GLUI_EDITTEXT_TEXT, pap->text1e );
	pap->edittext1e->w=160;
	
	pap->edittext1f =
	pap->glui->add_edittext_to_panel(obj_panel, "Minor :", GLUI_EDITTEXT_TEXT, pap->text1f );
	pap->edittext1f->w=160;
	
	obj_panel =  pap->glui->add_panel( "Flags" );
	
	pap->group2 =
	pap->glui->add_radiogroup_to_panel(obj_panel,&pap->scaletype,5,control_cb);
	
	pap->glui->add_radiobutton_to_group( pap->group2, "Linear" );
	
	pap->glui->add_radiobutton_to_group( pap->group2, "Log" );
	
	pap->min_box = new GLUI_Checkbox( obj_panel, "Auto Min", &pap->autoMin, 7, control_cb );
	
	pap->max_box = new GLUI_Checkbox( obj_panel, "Auto Max", &pap->autoMax, 8, control_cb );
	
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
	
	if(pap->axistype == 0){
		
		if(Plot->xAutoMinimum){
			msprintf(value,sizeof(value),"%g",Plot->xMinimum);
		}else{
			msprintf(value,sizeof(value),"%g",Plot->xSetMinimum);
		}
	    pap->edittext1a->set_text(value);
		
		if(Plot->xAutoMaximum){
			msprintf(value,sizeof(value),"%g",Plot->xMaximum);
		}else{
			msprintf(value,sizeof(value),"%g",Plot->xSetMaximum);
		}
	    pap->edittext1b->set_text(value);
		
		
		msprintf(value,sizeof(value),"%g",Plot->xMajorStep);
	    pap->edittext1c->set_text(value);
		msprintf(value,sizeof(value),"%g",Plot->xMinorStep);
	    pap->edittext1d->set_text(value);
		
		msprintf(value,sizeof(value),"%d",Plot->xMajorLimit);
	    pap->edittext1e->set_text(value);
		msprintf(value,sizeof(value),"%d",Plot->xMinorLimit);
	    pap->edittext1f->set_text(value);
		
	   	pap->min_box->set_int_val(Plot->xAutoMinimum);
	   	pap->max_box->set_int_val(Plot->xAutoMaximum);
		
		pap->group2->set_int_val(Plot->xLogScale);
		
	}else{
		
		
		if(Plot->yAutoMinimum){
			msprintf(value,sizeof(value),"%g",Plot->yMinimum);
		}else{
			msprintf(value,sizeof(value),"%g",Plot->ySetMinimum);
		}
	    pap->edittext1a->set_text(value);
		
		if(Plot->yAutoMaximum){
			msprintf(value,sizeof(value),"%g",Plot->yMaximum);
		}else{
			msprintf(value,sizeof(value),"%g",Plot->ySetMaximum);
		}
	    pap->edittext1b->set_text(value);
		
		
		msprintf(value,sizeof(value),"%g",Plot->yMajorStep);
	    pap->edittext1c->set_text(value);
		msprintf(value,sizeof(value),"%g",Plot->yMinorStep);
	    pap->edittext1d->set_text(value);
		
		msprintf(value,sizeof(value),"%d",Plot->yMajorLimit);
	    pap->edittext1e->set_text(value);
		msprintf(value,sizeof(value),"%d",Plot->yMinorLimit);
	    pap->edittext1f->set_text(value);
		
	   	pap->min_box->set_int_val(Plot->yAutoMinimum);
	   	pap->max_box->set_int_val(Plot->yAutoMaximum);
		
		pap->group2->set_int_val(Plot->yLogScale);
	}
	
	return 0;
	
}
static int setDialogData(struct PlotAttributesData *pap)
{
	uGridPlotPtr Plot;
		
	Plot=pap->dataLocal->Plot;
	if(!Plot)return 1;
	
	if(pap->axistype == 0){
		
		Plot->xAutoMinimum=pap->autoMin;
		
		Plot->xAutoMaximum=pap->autoMax;	
		
		if(!Plot->xAutoMinimum){
			sscanf(pap->edittext1a->get_text(),"%lg", &Plot->xSetMinimum);
		}
		
		if(!Plot->xAutoMaximum){
			sscanf(pap->edittext1b->get_text(),"%lg", &Plot->xSetMaximum);
		}	
		
	    sscanf(pap->edittext1c->get_text(),"%lg", &Plot->xMajorStep);
		
	    sscanf(pap->edittext1d->get_text(),"%lg", &Plot->xMinorStep);
		
	    sscanf(pap->edittext1e->get_text(),"%d", &Plot->xMajorLimit);
		
	    sscanf(pap->edittext1f->get_text(),"%d", &Plot->xMinorLimit);
		
		
		Plot->xLogScale=pap->scaletype;
		
	}else{
		
		Plot->yAutoMinimum=pap->autoMin;
		
		Plot->yAutoMaximum=pap->autoMax;
		
		if(!Plot->yAutoMinimum){
			sscanf(pap->edittext1a->get_text(),"%lg", &Plot->ySetMinimum);
		}
		
		if(!Plot->yAutoMaximum){
			sscanf(pap->edittext1b->get_text(),"%lg", &Plot->ySetMaximum);
		}	
		
	    sscanf(pap->edittext1c->get_text(),"%lg", &Plot->yMajorStep);
		
	    sscanf(pap->edittext1d->get_text(),"%lg", &Plot->yMinorStep);
		
	    sscanf(pap->edittext1e->get_text(),"%d", &Plot->yMajorLimit);
		
	    sscanf(pap->edittext1f->get_text(),"%d", &Plot->yMinorLimit);
		
		Plot->yLogScale=pap->scaletype;
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



