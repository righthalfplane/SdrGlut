/*
 *  BackgroundQ.c
 *  
 *
 *  Created by Dale Ranta on 12/19/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
#include "firstFile.h"
#include <stdio.h>
#include "BackgroundQ.h"
#include "mThread.h"

static struct QStruct q[200];

static int qSize=sizeof(q)/sizeof(struct QStruct);

static int qTop=0;

static int qBackGo(void *data,int (*sageThread)(void *data));

int qBackgroundClear(void)
{
	qTop=0;
	return 0;
}
int qBackgroundWait(void *data,int (*sageThread)(void *data))
{
	int ret;
	
	ret=qBackground(data,sageThread);

	if(ret == 0){
		q[qTop-1].wait=1;
	}

	return ret;
}

int qBackground(void *data,int (*sageThread)(void *data))
{
	if(!sageThread)return 1;
	
	if(qTop < qSize){
		q[qTop].data=data;
		q[qTop].sageThread=sageThread;
		q[qTop].launched=0;
		q[qTop].wait=0;
	}else{
		return 1;
	}
	
	++qTop;
	
	return 0;
}

int qCheck(void)
{
	int n;
	
	if(qTop <= 0)return 0;
	if(q[0].launched == 0){
		return qBackGo(q[0].data,q[0].sageThread);
	}else if(q[0].launched == -1){
		for(n=0;n<qTop-1;++n){
		    q[n]=q[n+1];
		}
		--qTop;
		return 3;
	}
	return 2;
}
static int qBackGo(void *data,int (*sageThread)(void *data))
{
	q[0].launched =  1;
	
	if(q[0].wait){
	    return sageThread(&q[0]);
	}else{
	   return launchThread(&q[0],sageThread);
	}
}
