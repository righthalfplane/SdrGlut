/*
 *  DialogFolder.h
 *  
 *
 *  Created by Dale Ranta on 8/14/13.
 *  Copyright 2013 Dale Ranta. All rights reserved.
 *
 */
#ifndef __DialogFolder_H__
#define __DialogFolder_H__

#include "Scene.h"


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
	int dialogFolder(struct Scene *scene,void (*callBacki)(struct Scene *scene));
	
#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
