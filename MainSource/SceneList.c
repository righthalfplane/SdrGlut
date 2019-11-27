/*
 *  SceneList.c
 *  
 *
 *  Created by Dale Ranta on 4/2/12.
 *  Copyright 2012 Dale Ranta. All rights reserved.
 *
 */

#include "SceneList.h"
#include "Utilities.h"

static struct SceneList *listRoot;

static struct SceneList *start;

int SceneSetWindow(struct Scene *scene)
{
	struct SceneList *list;
	
	if(!scene)return 1;
	
	list=SceneFindScene(scene);
	if(list){
		if(list->window >= 0){
			glutSetWindow(list->window);
			return 0;
		}
	}
	
	return 1;
}

int SceneSetName(struct Scene *scene,char *name)
{
	struct SceneList *list;
	
	if(!scene)return 1;
	
	list=SceneFindScene(scene);
	if(list){
		mstrncpy(list->name,name,sizeof(list->name));
		return 0;
	}
	
	return 1;
}

int SceneStartList(void)
{
	start=listRoot;
	return 0;
}

struct SceneList *SceneNextList(void)
{
	struct SceneList *ret;
	
	ret=start;
	if(start){
		start=start->next;
	}
	return ret;
}
int SceneClose()
{
     while(listRoot)
	 {
	     SceneDelete(listRoot->name);
	 }
	 
	 return 0;
}

struct SceneList *SceneFind(char *name)
{
	struct SceneList *next;
	
	if(!name)return NULL;
	
	
	next=listRoot;
	while(next){
		if(!mstrcmp(next->name, name))break;
	   next=next->next;
	}

	return next;
}
int SceneDelete(char *name)
{
	struct SceneList *next,*last;
	
	last=NULL;
	next=listRoot;
	while(next){
	   if(!mstrcmp(next->name, name)){
	       break;
	   }
	   last=next;
	   next=next->next;
	}
	
	if(next){
	    if(last){
		   last->next=next->next;
		}else{
		   listRoot=next->next;
		}
		cFree((char *)next);
	}

	return 0;
}

int SceneDeleteByScene(struct Scene *scene)
{
	struct SceneList *next,*last;
	
	last=NULL;
	next=listRoot;
	while(next){
		if(&next->scene == scene){
			break;
		}
		last=next;
		next=next->next;
	}
	
	if(next){
	    if(last){
			last->next=next->next;
		}else{
			listRoot=next->next;
		}
		cFree((char *)next);
	}
	
	return 0;
}

int SceneDeleteByNumber(int window)
{
	struct SceneList *next,*last;
	
	last=NULL;
	next=listRoot;
	while(next){
	   if(next->window == window){
	       break;
	   }
	   last=next;
	   next=next->next;
	}
	
	if(next){
	    if(last){
		   last->next=next->next;
		}else{
		   listRoot=next->next;
		}
		cFree((char *)next);
	}

	return 0;
}
struct SceneList *SceneFindByNumber(int window)
{
	struct SceneList *next;
	
	next=listRoot;
	while(next){
		if(next->window == window)break;
		next=next->next;
	}
	
	return next;
}
struct Scene *FindSceneByNumber(int window)
{
    struct SceneList *next;
    
    next=listRoot;
    while(next){
        if(next->window == window)break;
        next=next->next;
    }
    
    return &next->scene;

}
struct SceneList *SceneFindScene(struct Scene *scene)
{
	struct SceneList *next;
	
	next=listRoot;
	while(next){
		if(&next->scene == scene)break;
		next=next->next;
	}
	
	return next;
}

struct SceneList *SceneNext()
{
	struct SceneList *next;
	
	next=NULL;
	if(listRoot == NULL){
	   next=(struct SceneList *)cMalloc(sizeof(struct SceneList),9287);
	   if(!next)return NULL;
	   zerol((char *)next,sizeof(struct SceneList));
	   listRoot=next;
	}else{
		next=listRoot;
		while(next->next){
		   next=next->next;
		}
		next->next=(struct SceneList *)cMalloc(sizeof(struct SceneList),9288);
	    if(!next->next)return NULL;
		zerol((char *)next->next,sizeof(struct SceneList));
		next=next->next;
	}
	
	return next;
}
