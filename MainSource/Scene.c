/*
 *  Scene.c
 *  
 *
 *  Created by Dale Ranta on 2/4/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */

#include "Scene.h"
#include "Defs.h"
#include "Utilities2.h"
#include "Utilities.h"

int getPalette(int n,char *name,char *pal);

int CheckSceneBuffer(struct Scene *scene)
{
	long length;
	long lengthz;
	long n;
	
	if(!scene)return 1;
	
	length=3*sizeof(char)*scene->xResolution*scene->yResolution;
	
	lengthz=sizeof(float)*sizeof(char)*scene->xResolution*scene->yResolution;
	
	if(scene->dpp)
	{
		if(scene->xResolutiond == scene->xResolution && scene->yResolutiond == scene->yResolution)
		{
			zerol((char *)scene->dpp,length);
			goto Next;
		}
		cFree((char *)scene->dpp);
		scene->dpp=NULL;
	}
	
	scene->dpp=(unsigned char *)cMalloc(length,1011);
	
	if(!scene->dpp)
	{
		return 1;
	}
	
Next:
	
	if(scene->zpp)
	{
		if(scene->xResolutiond == scene->xResolution && scene->yResolutiond == scene->yResolution)
		{
			zerol((char *)scene->zpp,lengthz);
			goto Next2;
		}
		cFree((char *)scene->zpp);
		scene->zpp=NULL;
	}
	
	scene->zpp=(float *)cMalloc(lengthz,1011);
	
	if(!scene->zpp)
	{
		return 1;
	}
	
Next2:	
	
	if(scene->dppRight)
	{
		if(scene->xResolutiond == scene->xResolution && scene->yResolutiond == scene->yResolution)
		{
			zerol((char *)scene->dppRight,length);
			goto Next3;
		}
		cFree((char *)scene->dppRight);
		scene->dppRight=NULL;
	}
	
	scene->dppRight=(unsigned char *)cMalloc(length,1011);
	
	if(!scene->dppRight)
	{
		return 1;
	}
	
Next3:
	
	for(n=0;n<scene->xResolution*scene->yResolution;++n){
	   scene->zpp[n]=1e30;
	}
    
	scene->xResolutiond=(int)scene->xResolution;
	scene->yResolutiond=(int)scene->yResolution;
	
	return 0;
}
int SceneInit(struct Scene *scene)
{
	
	
	if(!scene)return 1;
	
	zerol((char *)scene,sizeof(struct Scene));
	
	DefaultScene(scene);
			
	scene->debug=1;
	
	scene->ThreadCount=4;
		
	scene->scale.showPalette=1;
	scene->scale.showPalette=0;
	
	scene->stereoType=StereoNone;
	
	
	scene->selectionBox.draw=1;
	scene->selectionBox.angle=0;
	
	
	return 0;

}
int sceneClean(struct Scene *scene)
{
	
	if(!scene)return 1;
    
	if(scene->dpp)cFree((char *)scene->dpp);
	scene->dpp=NULL;

	if(scene->dppRight)cFree((char *)scene->dppRight);
	scene->dppRight=NULL;	
	
	if(scene->zpp)cFree((char *)scene->zpp);
	scene->zpp=NULL;
	
	return 0;
	
}
int DefaultScene(struct Scene *scene)
{
	scene->xResolution=800;
	scene->yResolution=608;
	
	getPalette(12,(char *)g.palname,(char *)g.palette);
		
	return 0;
}

