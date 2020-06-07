
#ifndef  _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <conio.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#include <shlobj.h>
#include <objbase.h>

#include <stdarg.h>
#include "mThread.h"

char WarningBuff[256];

int zerol(char *s,unsigned long n);

int WarningBatch(char *WarningBuff)
{
	fprintf(stdout,"%s",WarningBuff);
    return  0;    
}

int Sleep2(int ms)
{
	Sleep(ms);

	return 0;
}

void *cMalloc(unsigned long length,int add)
{
	return malloc(length);
}

int cFree(char *ptr)
{
	free(ptr);
	return 0;
}



struct SendData{
    int (*sageThread)(void *data);
	void *data;
};

unsigned __stdcall  ThreadFunc( LPVOID lpParam ) 
{ 
	struct SendData *data=(struct SendData *)lpParam;

	if(data->sageThread)(*data->sageThread)(data->data);

	_endthreadex( 0 );

	return 0; 
} 



int launchThread(void *data,int (*sageThreadi)(void *data))
{
	unsigned int dwThreadId;
	HANDLE hThread; 
	struct SendData sdata;

	if(!sageThreadi)return 1;
	sdata.sageThread=sageThreadi;
	sdata.data=data;

    hThread = (HANDLE)_beginthreadex( 
        NULL,                       
        0,                          
        &ThreadFunc,  
        &sdata,					
        0,                         
        &dwThreadId);               

	if (hThread == NULL){
		WarningBatch("CreateThread Failed\n");
	}else{
	    Sleep2(50);
		CloseHandle( hThread );
	}

	return 0;
}
int zerol(char *s,unsigned long n)
{
	unsigned long count;

	if(s == NULL)return 1;
	count=0;
	while(count++ < n)*s++ = 0;
	return 0;
}
