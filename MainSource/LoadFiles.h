/*
 *  LoadFiles.h
 *  mShowOpenGL
 *
 *  Created by Dale Ranta on 2/4/11.
 *  Copyright 2011 SAIC. All rights reserved.
 *
 */

#ifndef __LOADFILES_H__
#define __LOADFILES_H__

#include "Scene.h"

#define FileTypeNone				0
#define FileTypeTemperatureSTL		1
#define FileTypeFlight				2
#define FileTypeGlut				3
#define FileTypeKdTree				4
#define FileTypeVolumetric			5
#define FileTypeTemperature			6
#define FileTypeGLSL				7
#define FileTypeKdTreeSTL		    8
#define FileTypeKdTemperatures		9
#define FileTypeAres			   10
#define FileTypeSDR                11
#define FileTypeImages			   12

#define FileTypeClusters		   14
#define FileTypeSphereToDisk	   15
#define FileTypeSTLCompiled        16
#define FileTypeSTLImmediate       17
#define FileTypeFireBall		   18
#define FileTypeSdrRadio           19
#define FileTypeFMRadio            20
#define FileTypeLines              21

#define DrawTypeNone 0
#define DrawTypePixel 1
#define DrawTypeGlut 2



#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
	int LoadFiles (struct Scene *scene,char *name,int FileType);
	int LoadKdTemperaturesSTL (struct Scene *scene,char *name);
	int doCommands(int command);
	int mDisplay(struct Scene *scene);
	int rDisplay(struct Scene *scene);
	int ClearFileData (struct Scene *scene);
	int ReLoadFile(struct Scene *scene,int flag);
	int GetTemperatures (struct Scene *scene,double *min, double *max);
	int UpdateTemperatureColors(struct Scene *scene);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
