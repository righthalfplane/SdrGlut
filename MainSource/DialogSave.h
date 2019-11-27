/*
 *  DialogSave.h
 *  FireBall
 *
 *  Created by Dale Ranta on 10/25/11.
 *  Copyright 2011 SAIC. All rights reserved.
 *
 */
#ifndef __DialogSave_H__
#define __DialogSave_H__

#include "Scene.h"
#include "GridPlot.h"


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */

int dialogSaveC(struct Scene *scene, void (*callBack)(struct Scene *scene,char *name),int type,char *namein);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
