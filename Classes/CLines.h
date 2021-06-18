/*
 *  CLines.h
 *  Rx3
 *
 *  Created by Dale Ranta on 1/29/14.
 *  Copyright 2014 Dale Ranta. All rights reserved.
 *
 */

#ifndef __CLINES_H__
#define __CLINES_H__
#include "CWindow.h"
#include "GridPlot.h"
#include "Message.h"

class CLines;

//class CSdrFile;

class CLines: public CWindow{
public:
    
  //  CSdrFile *sdr;
    
    
	CLines();
    
	CLines(struct Scene *scene);
    
	virtual ~CLines();

	virtual int OpenWindows(struct Scene *scene);
    
    virtual int SetFrequency(struct Scene *scene,double f,double bw,int messageType);

    int OpenWindows(struct Scene *scene,int parentWindow);
    
    void getMouse(int button, int state, int x, int y);
	
	void menu_select(struct Scene *scene,int item);
	
	void display(struct Scene *scene);
	
	int DrawIt(struct Scene *scene);
		
	void reshape(struct Scene *scene,int w,int h);
	
	int Information(struct Scene *scene);
	
	int plotPutData(struct Scene *scene,double *x,double *y,long count,long replace);
	
	int plotPutLabel(struct Scene *scene,char *label,long item);
	
    int DrawLabels(struct Scene *scene);
    
    int SaveData(struct Scene *scene,char *path);
    
    static CLines *CLinesOpen(struct Scene *scene,int parentWindow);

    struct Scene *sceneSource;

	struct uPlotLabelsStruct{
		char Labels[5][256];
		char flag[5];
		double x[5];
		double y[5];
	};
	
	typedef struct uPlotLabelsStruct uPLotLabels;
	
	struct LineStruct{ 
		int window;
		int w,h;
		double xmin,xmax,ymin,ymax;
		struct Scene *scene;
		struct DOList l;
		uGridPlotPtr Plot;
		uPLotLabels labels;
	};

    int PlotLabels(struct Scene *scene,struct LineStruct *lines);
    
    int GetLineData(struct LineStruct *lines, uLineDataPtr *Line, int *linei, int *LineCount, int *Pattern);
    
    int LinePattern(struct LineStruct *lines, int line, int pattern, int flag);
    
    int LineColor(struct LineStruct *lines, int line, int color);
    
    int LineLabel(struct LineStruct *lines, int line, char *label);
    
    int LineDelete(struct LineStruct *lines, int line);
    
    int PlotAttributes(struct Scene *scene,struct LineStruct *lines);
    
    int PlotBox(struct Scene *scene,struct LineStruct *lines);

    int LineAttributes(struct Scene *scene,struct LineStruct *lines);
    
	struct LineStruct lineHolder;
	
	struct LineStruct *lines;
	
    double Frequency;

    double BandWidth;
    
    int wShift;

};

typedef CLines *CLinesPtr;

#endif
