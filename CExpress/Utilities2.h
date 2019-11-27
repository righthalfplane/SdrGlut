/*
 *  Utilities.h
 *  
 *
 *  Created by Dale Ranta on 2/4/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */

#ifndef __UTILITIES2__
#define __UTILITIES2__
#include "firstFile.h"
#include <stdio.h>
#include <sys/timeb.h>
/* #include <unistd.h> */
#include <stdlib.h>
#include "ulibTypes.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


#define max(x1,x2)    (((x1) > (x2)) ? (x1) : (x2))
#define min(x1,x2)    (((x1) < (x2)) ? (x1) : (x2))

#include "SetFrameData.h"
#include "paletteDraw.h"

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
	long FileLength(FILE *in);	
	int GetTime2(long *Seconds,long *milliseconds);
	void *cMalloc(unsigned long r,int tag);
	int zerol(char *p,unsigned long n);	
	void WarningBatch(char *message);
	double rtime(void);
	int cFree(char *p);
	extern char WarningBuff[256];
	int mstrncpy(char *out,char *in,long n);
	int strncatToPath(char *pathname,char *name,int length);
	int mstrncat(char *out,char *in,long n);
	int FixPathName(char *path);
	int goCD(char *path);
	int mstrcmp(char *name1,char *name2);
	char *strsave(char *name,int tag);
	int uMemcpy(void *out1,void *in1,long length);
	void *cRealloc(void *p,unsigned long r,int tag);
	int MoveB(int x,int y);
	int LineB(int x,int y,int nc,struct screenData *s);
	int swhere(double *x,double *y);
	int sdraw(double x,double y,struct screenData *s);
	int smove(double x,double y);
	int symsft(double xp,double yp,double hp,char *ib,double angle,int nchar,struct screenData *s);
	int LineB2(int x1,int y1,int x2,int y2,int nc,struct screenData *s);
	int kpoint(int x,int y,int nc,struct screenData *s);
	int DrawString2(int x, int y, char *message,double xsize,struct screenData *s);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
