/*
 *  Scene.h
 *  
 *
 *  Created by Dale Ranta on 2/4/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */

#ifndef __SCENE_H__
#define __SCENE_H__
#include <math.h>
#include "ulibTypes.h"
#include "sceneDisplay.h"
#include "BackgroundQ.h"

#ifdef __APPLE__
#include <GLUT/glut.h>	/* GLUT OpenGL includes */
#else
#include <GL/glut.h>	/* GLUT OpenGL includes */
#endif

//#include "KdTree.h"
#include "Defs.h"
//#include "Matrix.h"



#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */

#define uint32 uint3232
    
struct P{
    double x;
    double y;
    double z;
};

struct Pf{
	float x;
	float y;
	float z;
};
	
struct Color{
    float r;
    float g;
    float b;
};
    
    
struct Colors{
	struct Pf c1;
	struct Pf c2;
	struct Pf c3;
};

struct ScaleInfo{
	int updateTemperatureScale;
	int dataIsDefined;
	int showPalette;
	double dmin,dmax;
	int logscale;
	double a,b;
	double sPmax;
	double sPmin;
};

struct GlobalData{
    struct FileInfo2 *FilesList[1000];
    int FileOwner[1000];
    long FileCount;
    unsigned char *buffin,*buffout;
    long buffsize;
	unsigned char palette[256*3];
	unsigned char palname[256];
};
	
struct SelectionBox{
	int clipToBox;
	int draw;
	double xc,yc,zc;
	double dx,dy,dz;
	double angle;
	struct P pc;
	struct P bounds[6];
	double north,south,east,west;
	int locationSet;
	int update;
};
	
		
struct Scene{
    struct SelectionBox selectionBox;

	struct ScaleInfo scale;
    
    long xResolution;
    long yResolution;
	long debug;
    
    long ThreadCount;
	
	unsigned char *dpp;
	unsigned char *dppRight;
	float *zpp;

	int xResolutiond;
	int yResolutiond;

    char WindowTitle[512];
    
    char FileName[512];
    
    char FilePathIQ[2048];
	
	double workTimer;
	
	int workFlip;

    int (*sceneDisplay)(struct Scene *scene);
	
	int (*sceneDisplay2)(struct Scene *scene);
	
	int (*UpdateTemperaturesQ)(struct QStruct *q);
	
	int (*CompilePtr)(struct Scene *scene,int flag);
    
	int stereoType;
		
    int windowType;
};

	int DefaultScene(struct Scene *scene);
	int CheckSceneBuffer(struct Scene *scene);
	int SetWindowDefault(void);
	int SceneInit(struct Scene *scene);
	int RecipeExe(struct Scene *scene);
	int sceneClean(struct Scene *scene);
	int sceneDisplay(struct Scene *scene);
	
#ifndef EXTERN
#define EXTERN extern
#endif

	EXTERN struct GlobalData g;
	
#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */


#endif

