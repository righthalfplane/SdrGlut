/*
 *  SceneList.h
 *  
 *
 *  Created by Dale Ranta on 4/2/12.
 *  Copyright 2012 Dale Ranta. All rights reserved.
 *
 */

#ifndef __SceneList__
#define __SceneList__

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */

#include "Scene.h"

struct SceneList{

	struct Scene scene;
  
	char name[256];
	
  	int window;
	
    void *listPointer;
    
    char *fileListName;
    
	struct SceneList *next;
};

struct SceneList *SceneNext(void);
struct SceneList *SceneFind(char *name);
int SceneDelete(char *name);
int SceneClose(void);
int SceneDeleteByNumber(int sub_window);
int SceneDeleteByScene(struct Scene *scene);
struct SceneList *SceneFindByNumber(int sub_window);
struct SceneList *SceneFindScene(struct Scene *scene);
struct Scene *FindSceneByNumber(int sub_window);
int SceneStartList(void);
struct SceneList *SceneNextList(void);
int SceneSetWindow(struct Scene *scene);
int SceneSetName(struct Scene *scene,char *name);
#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */


#endif
