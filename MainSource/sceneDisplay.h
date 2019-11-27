/*
 *  sceneDisplay.h
 *  
 *
 *  Created by Dale Ranta on 6/19/13.
 *  Copyright 2013 Dale Ranta. All rights reserved.
 *
 */
#ifndef __sceneDisplay_h__   
#define __sceneDisplay_h__   

#include "Scene.h"

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
#define ImmediateMode     0
#define CompiliedMode     1
#define VertexArrayMode   2
#define VboMode			  3
#define VboMode2		  4
	
#define StereoNone				1
#define StereoDoubleBuffered	2
#define StereoSingleBuffered	4
#define StereoSplitScreen		8
	
#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
