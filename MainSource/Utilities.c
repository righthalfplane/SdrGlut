/*
 *  Utilities.c
 *  
 *
 *  Created by Dale Ranta on 2/4/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
#include "firstFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <string.h>
#include "Utilities.h"

#ifndef WIN32
#include <unistd.h>
#endif

static int GetTime(long *Seconds,long *milliseconds);

#define MaxIncrement 200

unsigned long int TickCount(void)
{
	unsigned long int t;
	double sec;

	sec=rtime()*60;
	
	t=(unsigned long int)(sec);
	
	
	return t;
}


char *GetExtension(char *name)
{
	char *lp;
	
	if(!name)return NULL;
	
	lp=strrchr(name,'.');
	
	if(lp)++lp;
	
	return lp;
}

int mstrncat(char *out,char *in,long n)
{
	long len;
	
	if(!out || !in || (n <= 0))return 1;
		
	len=(long)strlen(out);
	
	n -= len;
	
	if(n <= 0)return 1;
	
	out += len;
	
	while(n-- > 0){
	    if(*in == 0){
	       *out = 0;
	       break;
	    }else{
	       *out++ = *in++;
	    }
	}
	
	return 0;
}
char *strsave(char *s,int tag)
{
	long length;
	char *p;
	
	if(!s)return NULL;
	
	length=(long)strlen(s)+1;
	
	if((p=(char *)cMalloc(length+1,tag)) != NULL)
		mstrncpy(p,s,length);
	return(p);
}
int mstrncpy(char *out,char *in,long n)
{
	if(!out || !in || (n <= 0))return 1;
	
	while(n-- > 0){
	    if(*in == 0){
			*out = 0;
			break;
	    }else{
			*out++ = *in++;
	    }
	}
	
	return 0;
}


int ResizeArray(void *Array,long Length,long Count,long *Maximun,int tag)
{
	char ***array;
	char **array1;
	char **array2;
	long Max;
	long n;
	
	if(!Array || !Maximun || (Length <= 0))return 1;
	
	Max = *Maximun;
	
	if(Count+1 <= Max)return 0;
	
	array=(char ***)Array;
	
	if(Count > 0){
		array1 = *array;
		if(!array1)return 1;
		array2=(char **)cMalloc((MaxIncrement+Max)*Length,tag);
		for(n=0;n<Max;++n){		    
            memcpy(&array2[n],&array1[n],Length);
		}
	    zerol((char *)&array2[Max],MaxIncrement*Length);
	    cFree((char *)array1);
	    *array=array2;
		*Maximun=MaxIncrement+Max;
	}else{
	    *array=(char **)cMalloc(MaxIncrement*Length,tag);
	    if(!*array)return 1;
	    zerol((char *)*array,MaxIncrement*Length);
	    *Maximun=MaxIncrement;
	}
	return 0;
}
long FileLength(FILE *in)
{
	long length;
	
	if(!in)goto ErrorOut;
	
	if(fseek(in,0L,2)){
		goto ErrorOut;
	}
	
	if((length=ftell(in)) == -1L){
		goto ErrorOut;
	}
	
	if(fseek(in,0L,0)){
		goto ErrorOut;
	}
	return length;
ErrorOut:
	return -1L;
}

int zerol(char *p,unsigned long n)
{
	if(!p || !n)return 1;
	
	while(n-- > 0)*p++ = 0;
	
	return 0;
}

double rtime(void)
{
	long milliseconds;
	long Seconds;
	double ret;
	
	
	GetTime(&Seconds,&milliseconds);
	
	ret=(double)Seconds+(double)milliseconds/1000.;
	
	return ret;

}
static int GetTime(long *Seconds,long *milliseconds)
{
	struct timeb t;
	
	if(!Seconds || !milliseconds)return 1;
	

	ftime(&t);

	*Seconds=(long)t.time;
	*milliseconds=t.millitm;
	
	return 0;
}
void WarningBatch(char *message)
{
	int n;
	
	while(*message){
		n = *message++;
		if(n == '\r' || n == '\n'){
			printf("\n");
			fflush(stdout);
		}else{
			putc(n,stdout);
		}
	}
}

#ifdef WIN32
#define vsnprintf _vsnprintf
#define chdir _chdir
#define getcwd _getcwd
#define unlink _unlink
#endif
extern int WriteToGLUIWindow(char *message);
void WarningPrint(const char *fmt, ...)
{
	char buff[4096];
	va_list arg;
	int ret;
	
    va_start(arg, fmt);
    ret = vsnprintf((char *)buff, sizeof(buff)-1, fmt, arg);
	if(ret){

	}
    va_end(arg);
	
	/* if(!WriteToGLUIWindow(buff))return; */
	
	WarningBatch((char *)buff);

}
void msprintf(char *buff, int size, const char *fmt, ...)
{
	va_list arg;
	int ret;
	
    va_start(arg, fmt);
    ret = vsnprintf((char *)buff, size-1, fmt, arg);
	if(ret){

	}
    va_end(arg);

}
char *GetWorking(char *buf, long size)
{
	return getcwd(buf,size);
}
int MakeNewDirectory(char *name)
{
	if(!name)return 1;

#ifdef WIN32 
	if(_mkdir(name))return 1;
#else
	if(mkdir(name,0755))return 1;
#endif

	return goCD(name);
}
int DeleteFile7(char *name)
{
	return unlink(name);
}
int goCD(char *name)
{
	return chdir(name);
}
int WriteRect24ToRect24(unsigned char *in, uRect *SRect,long xsize, long ysize,
						unsigned char *out,uRect *DRect,long oxsize,long oysize)
{
	double xScale,yScale;
	register unsigned char *outp,*inp;
	register int ic;
	long oxsize3;
	long xsize3;
	int i,j,jj,ii;
	int jd,id;
	int is,js;
	int ie,je;
	
	
	if(!in || !out || !SRect || !DRect )return 1;
	
	oxsize3=3*oxsize;
	xsize3=3*xsize;
	
	if((SRect->xsize == DRect->xsize) && (SRect->ysize == DRect->ysize)){
	    js=SRect->y;
	    if(js < 0)js = 0;
	    
	    je=SRect->y+SRect->ysize;
	    if(je >= ysize)je=(int)ysize;
	    
	    is=SRect->x;
	    if(is < 0)is = 0;
	    
	    ie=SRect->x+SRect->xsize;
	    if(ie >= xsize)ie=(int)xsize;
		
	    jj=DRect->y;
	    if(jj < 0){
	        js -= jj;
	        jj=0;
	    }
	    
	    if(jj+je-js > oysize){
	        je=(int)(js+oysize-jj);
	    }
		
		
	    ii=DRect->x;
	    if(ii < 0){
	        is -= ii;
	        ii=0;
	    }
		
	    if(ii+ie-is > oxsize){
	        ie=(int)(is+oxsize-ii);
	    }
	    
	    ie -= is;
	    
	    if(ie < 0)ie = 0;
	    
	    for(j=js;j<je;++j){
	    	outp=out+3*ii+jj++ *oxsize3;
	    	inp=in+is*3+j*xsize3;	    	
	    	ic=ie+1;
	    	while(--ic){	    	
	    		*outp++ = *inp++;
	    		*outp++ = *inp++;
	    		*outp++ = *inp++;
	    	}
	    }
	}else{
	    xScale=(double)SRect->xsize/(double)DRect->xsize;
	    yScale=(double)SRect->ysize/(double)DRect->ysize;
	    if(xScale > 1.0 && yScale > 1.0){
	    	xScale=(double)DRect->xsize/(double)SRect->xsize;
	    	yScale=(double)DRect->ysize/(double)SRect->ysize;
		    for(j=0;j<SRect->ysize;++j){
		        js=j+SRect->y;
		    	if(js < 0 || js >= ysize)continue;	 
		    	jd=DRect->y+(int)(j*yScale);
		    	if(jd < 0 || jd >= oysize)continue;
		    	for(i=0;i<SRect->xsize;++i){
		        	is=i+SRect->x;
		    		if(is < 0 || is >= xsize)continue;	 
		    		id=DRect->x+(int)(i*xScale);
		    		if(id < 0 || id >= oxsize)continue;
		    		/*
					 if(in[is*3+js*xsize3] == 255 && in[is*3+1+js*xsize3] == 255 &&
					 in[is*3+2+js*xsize3] == 255){
					 continue;
					 }
					 */
		    		out[id*3+jd*oxsize3]=in[is*3+js*xsize3];
		    		out[id*3+1+jd*oxsize3]=in[is*3+1+js*xsize3];
		    		out[id*3+2+jd*oxsize3]=in[is*3+2+js*xsize3];
		    		
		    	}
		    }
	    }else{
		    for(j=0;j<DRect->ysize;++j){
		        jd=j+DRect->y;
		    	if(jd < 0 || jd >= oysize)continue;
		    	js=SRect->y+(int)(j*yScale);
		    	if(js < 0 || js >= ysize)continue;	 
		    	for(i=0;i<DRect->xsize;++i){
		        	id=i+DRect->x;
		    		if(id < 0 || id >= oxsize)continue;
		    		is=SRect->x+(int)(i*xScale);
		    		if(is < 0 || is >= xsize)continue;	 
		    		out[id*3+jd*oxsize3]=in[is*3+js*xsize3];
		    		out[id*3+1+jd*oxsize3]=in[is*3+1+js*xsize3];
		    		out[id*3+2+jd*oxsize3]=in[is*3+2+js*xsize3];
		    	}
		    }
	    }
	}
	
	return 0;
}
