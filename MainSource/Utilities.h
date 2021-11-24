/*
 *  Utilities.h
 *  
 *
 *  Created by Dale Ranta on 2/4/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */

#ifndef __UTILITIES__
#define __UTILITIES__
#include "firstFile.h"
#include <stdio.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include "ulibTypes.h"

#include "SetFrameData.h"
#include "paletteDraw.h"

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
	void msprintf(char *buff, int size, const char *fmt, ...);
	void WarningPrint(const char *fmt, ...);
	
	void *cMalloc(unsigned long r, int tag);
	void *cRealloc(void *p, unsigned long r, int tag);
	int zerol(char *p, unsigned long n);		
	int cFree(char *p);
	int DeleteFile7(char *name);
	double rtime(void);	
	void WarningBatch(char *message);
	extern char WarningBuff[256];	
	long FileLength(FILE *in);
	int mstrncpy(char *out, char *in, long n);
	int getPalette(int n,char *name,char *pal);
	int ResizeArray (void *Array,long Length,long Count,long *Maximun,int tag);
	
	int mstrncat(char *out,char *in,long n);
	int mstrcmp(char *name1,char *name2);	
	int strncatToPath(char *pathname,char *name,int length);
	int uMemcpy(void *out1,void *in1,long length);
	char *strsave(char *s,int tag);
	
	int MoveB(int x,int y);
	int LineB(int x,int y,int nc,struct screenData *s);
	int putString(unsigned char *data,long length,FILE *out);
	char *GetWorking(char *buf, long size);
	int goCD(char *path);
	char *GetExtension(char *name);
	int MakeNewDirectory(char *name);
	int WriteRect24ToRect24(unsigned char *in, uRect *SRect,long xsize, long ysize,
							unsigned char *out,uRect *DRect,long oxsize,long oysize);
	unsigned long int TickCount(void);
    int uSetRect(uRect *r,int x,int y,int xsize,int ysize);
    int uPtInRect(uPoint *p,uRect *r);
    int uInsetRect(uRect *r,int x,int y);


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
