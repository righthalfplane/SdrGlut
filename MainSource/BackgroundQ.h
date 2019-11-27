/*
 *  BackgroundQ.h
 *  
 *
 *  Created by Dale Ranta on 12/19/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
#ifndef __BackgroundQ__
#define __BackgroundQ__

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */


struct QStruct{
    int (*sageThread)(void *data);
	void *data;
	int launched;
	int wait;
};


int qBackground(void *data,int (*sageThread)(void *data));

int qBackgroundWait(void *data,int (*sageThread)(void *data));

int qCheck(void);

int qBackgroundClear(void);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
