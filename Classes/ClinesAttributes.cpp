/*
 *  ClinesAttributes.cpp
 *  Rx3
 *
 *  Created by Dale Ranta on 1/31/14.
 *  Copyright 2014 Dale Ranta. All rights reserved.
 *
 */

#include "firstFile.h"
#include <stdio.h>

#include "Utilities.h"

#include "ClinesAttributes.h"

#include <cstdio>
#include <cstdlib>

#include <GL/glui.h>


struct LineAttributesData{
	
	GLUI *glui;
	
	struct Scene *scene;
		
	char text1z[255];
	GLUI_EditText *edittext1z;
	
	char text1a[255];
	GLUI_EditText *edittext1a;
	
	char text1b[255];
	GLUI_EditText *edittext1b;
	
	char text1c[255];
	GLUI_EditText *edittext1c;
	
	int usePattern;
	
	GLUI_Scrollbar *color_scroll;
	
	GLUI_Scrollbar *pattern_scroll;
	
	GLUI_Scrollbar *line_scroll;
	
	GLUI_Checkbox *check_box;
	
	int color_Index,pattern_Index,line_Index;
	
  	int sub_window;
	
	struct CLines::LineStruct *dataLocal;
	
};

static struct LineAttributesData laps;

static void control_cb(int control);

static int getDialogData(struct Scene *scene,struct LineAttributesData *lap);

int CLines::LineAttributes(struct Scene *scene,struct LineStruct *lines)
{
	
	if(!scene || !lines)return 1;
	
	
		struct LineAttributesData *lap;
		
		if(!scene)return 1;
		
		lap=&laps;
		
		lap->text1z[0]='0';
		
		lap->text1a[0]='2';
		lap->text1a[1]='5';
		lap->text1a[2]='5';
		
		lap->text1b[2]='0';
		
		lap->scene=scene;
		
		lap->dataLocal=lines;
		
		lap->glui = GLUI_Master.create_glui( "Line Attributes" );
		
		GLUI_Panel *panel3 = new GLUI_Panel( lap->glui, "Line Number" );
		
		lap->edittext1z =
		lap->glui->add_edittext_to_panel(panel3, "", GLUI_EDITTEXT_TEXT, lap->text1z );
		lap->edittext1z->w=160;
		
		lap->line_scroll = 
		new GLUI_Scrollbar( panel3, "Line Number", GLUI_SCROLL_HORIZONTAL,
						   &lap->line_Index, 23,
						   control_cb );
		
		lap->line_scroll->set_int_limits( 0, 5 );
		
		new GLUI_Button(panel3, "Delete Line", 30, control_cb); 
		
		
		GLUI_Panel *panel2 = new GLUI_Panel( lap->glui, "Line Legend" );
		
		lap->edittext1c =
		lap->glui->add_edittext_to_panel(panel2, "", GLUI_EDITTEXT_TEXT, lap->text1c );
		lap->edittext1c->w=260;
		
		new GLUI_Button(panel2, "Set Line Legend", 22, control_cb); 
		
		GLUI_Panel *panel0 = new GLUI_Panel( lap->glui, "Color Selection" );
		
		lap->edittext1a =
		lap->glui->add_edittext_to_panel(panel0, "Color Index :", GLUI_EDITTEXT_TEXT, lap->text1a );
		lap->edittext1a->w=160;
		
		lap->color_scroll = 
		new GLUI_Scrollbar( panel0, "Color Index :", GLUI_SCROLL_HORIZONTAL,
						   &lap->color_Index, 20,
						   control_cb );
		
		lap->color_scroll->set_int_limits( 0, 255 );
		
		new GLUI_Button(panel0, "Set Line Color", 4, control_cb); 
		
		
		GLUI_Panel *panel1 = new GLUI_Panel( lap->glui, "Pattern Selection" );
		
		lap->check_box = new GLUI_Checkbox( panel1, "Use Pattern", &lap->usePattern, 7, control_cb );
		
		lap->edittext1b =
		lap->glui->add_edittext_to_panel(panel1, "Pattern Index :", GLUI_EDITTEXT_TEXT, lap->text1b );
		lap->edittext1b->w=160;
		
		lap->pattern_scroll = 
		new GLUI_Scrollbar( panel1, "Pattern Index :", GLUI_SCROLL_HORIZONTAL,
						   &lap->pattern_Index, 21,
						   control_cb );
		
		lap->pattern_scroll->set_int_limits( 0, 7 );
		
		
		new GLUI_Button(panel1, "Set Line Pattern", 3, control_cb); 
		
		new GLUI_Button(lap->glui, "Close", 5, control_cb); 
		
		lap->sub_window=glutGetWindow();
		
		lap->glui->set_main_gfx_window( lap->sub_window );
		
		getDialogData(scene,lap);
		
		return 0;
}
static int getDialogData(struct Scene *scene,struct LineAttributesData *lap)
{
	uLineDataPtr Line;
	int line, LineCount;
	int Pattern;
	char value[256];
	
	CLinesPtr lines=(CLinesPtr)FindScene(scene);
	
	if(!lines)return 1;
		
	sscanf(lap->edittext1z->get_text(),"%d", &line);
	
	if(lines->GetLineData(lap->dataLocal, &Line, &line, &LineCount, &Pattern)){
	    WarningPrint("Error Getting Point TimeHistory Line Data\n");
	    lap->glui->close();
		return 1;
	}
	
	lap->line_scroll->set_int_limits( 0, LineCount-1);
	
	msprintf(value,sizeof(value),"%d",line);
	lap->edittext1z->set_text(value);
	lap->line_scroll->set_int_val(line);
	
	msprintf(value,sizeof(value),"%d",Line->Attributes.nLineFore);
	lap->edittext1a->set_text(value);
	lap->color_scroll->set_int_val(Line->Attributes.nLineFore);
	
	lap->edittext1c->set_text(Line->save);
	
	lap->check_box->set_int_val(Line->Attributes.DashFlag);
	
	msprintf(value,sizeof(value),"%d",Pattern);
	lap->edittext1b->set_text(value);
	
	lap->pattern_scroll->set_int_val(Pattern);
	
	return 0;
}
static void control_cb(int control) 
{
	struct LineAttributesData *lap;
	int pattern;
	int color;
	int line;
	int flag;
	char value[256];
	
	
	lap=&laps;
	
	CLinesPtr lines=(CLinesPtr)FindScene(lap->scene);
	if(!lines){
		lap->glui->close();
		return;
	}
	
	flag=lap->usePattern;
	
	sscanf(lap->edittext1z->get_text(),"%d", &line);
	
	sscanf(lap->edittext1a->get_text(),"%d", &color);
	
	sscanf(lap->edittext1b->get_text(),"%d", &pattern);
	
	if(control == 1)
	{
		lap->glui->close();
		
	}
	else if(control == 2)
	{
		lap->glui->close();
	}
	else if(control == 3)
	{
		lines->LinePattern(lap->dataLocal, line, pattern, flag);
		
	}
	else if(control == 4)
	{
		lines->LineColor(lap->dataLocal, line, color);
		
	}
	else if(control == 5)
	{
		lap->glui->close();
	}
	else if(control == 20)
	{
		lines->LineColor(lap->dataLocal, line, lap->color_Index);
		msprintf(value,sizeof(value),"%d",lap->color_Index);
		lap->edittext1a->set_text(value);
	}
	else if(control == 21)
	{
		lines->LinePattern(lap->dataLocal, line, lap->pattern_Index, flag);
		msprintf(value,sizeof(value),"%d",lap->pattern_Index);
		lap->edittext1b->set_text(value);
	}
	else if(control == 22)
	{
		lines->LineLabel(lap->dataLocal, line, (char *)lap->edittext1c->get_text());
	}
	else if(control == 23)
	{
		
		msprintf(value,sizeof(value),"%d",lap->line_Index);
		lap->edittext1z->set_text(value);
		
		getDialogData(lap->scene,lap);
	}
	else if(control == 30)
	{
		lines->LineDelete(lap->dataLocal, lap->line_Index);
	}
}
int CLines::GetLineData(struct LineStruct *lines, uLineDataPtr *Line, int *linei, int *LineCount, int *Pattern)
{
	uGridPlotPtr Plot;
	DOListPtr l;
	int line;
	int n;
	
	if(!lines || !Line || !linei || !LineCount || !Pattern)return 1;
	
	
	Plot=lines->Plot;
	if(!Plot)return 1;
	l=&lines->l;
	
	line=*linei;
	
	if(line >= (int)Plot->LineCount)line=(int)Plot->LineCount-1;
	if(line < 0)line=0;
	
	if(Plot->LineCount > 0){
	    *Line=Plot->Lines[line];
		*linei=line;
		*LineCount=(int)Plot->LineCount;
		*Pattern=0;
		for(n=0;n<l->NumberOfPatterns;++n){
			if(!strcmp(Plot->Lines[line]->Attributes.LineDash,l->LinePatterns[n])){
				*Pattern=n;
			}
		}
		return 0;
	}	
	
	
	return 1;
}
int CLines::LinePattern(struct LineStruct *lines, int line, int pattern, int flag)
{
	uGridPlotPtr Plot;
	uLineDataPtr Line;
	DOListPtr l;
	
	
	Plot=lines->Plot;
	if(!Plot)return 1;
	l=&lines->l;
	if(Plot->LineCount > 0 && line >= 0 && line < Plot->LineCount && pattern >= 0 && pattern < l->NumberOfPatterns){
	    Line=Plot->Lines[line];
		mstrncpy(Line->Attributes.LineDash,l->LinePatterns[pattern],16);
		Line->Attributes.DashFlag=flag;
	}	
	
	glutSetWindow(lines->window);
	glutPostRedisplay();
	
	return 0;
}
int CLines::LineColor(struct LineStruct *lines, int line, int color)
{
	uGridPlotPtr Plot;
	uLineDataPtr Line;
	
		
	Plot=lines->Plot;
	if(!Plot)return 1;
	
	if(Plot->LineCount > 0 && line >= 0 && line < Plot->LineCount){
	    Line=Plot->Lines[line];
		Line->Attributes.nLineFore=color;
	}	
	
	glutSetWindow(lines->window);
	glutPostRedisplay();
	
	return 0;
}
int CLines::LineLabel(struct LineStruct *lines, int line, char *label)
{
	uGridPlotPtr Plot;
	uLineDataPtr Line;
	
	
	
	Plot=lines->Plot;
	if(!Plot)return 1;
	
	if(Plot->LineCount > 0 && line >= 0 && line < Plot->LineCount){
	    Line=Plot->Lines[line];
		mstrncpy(Line->save,label,64);
	}	
	
	glutSetWindow(lines->window);
	glutPostRedisplay();
	
	return 0;
}
int CLines::LineDelete(struct LineStruct *lines, int line)
{
	uGridPlotPtr Plot;
	uLineDataPtr Line;
	
	int n,nl;
		
	Plot=lines->Plot;
	if(!Plot)return 1;
	
	if(Plot->LineCount > 0 && line >= 0 && line < Plot->LineCount){
	    Line=Plot->Lines[line];
		nl=0;
		for(n=0;n<Plot->LineCount;++n){
		    if(Plot->Lines[n] != Line){
				Plot->Lines[nl++]=Plot->Lines[n];
			}
		}
		Plot->LineCount=nl;
		(*Line->Kill)((DObjPtr)Line);
	}	
	
	glutSetWindow(lines->window);
	glutPostRedisplay();
	
	return 0;
}

	
