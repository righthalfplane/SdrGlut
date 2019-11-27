#ifndef __AUDIOLIB_
#define __AUDIOLIB_

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <stdlib.h>
//#include <unistd.h>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif

#define NO_MORE_SPACE 99999

struct audioInfo
{
    ALCdevice *dev;
    ALCcontext *ctx;
    
	ALuint *source;
	int *sourceFlag;
	int numsource;
	
	ALuint *buff;
	int *buffFlag;
	int numbuff;
	
};

extern struct audioInfo *audio;

ALvoid DisplayALError(unsigned char *szText, ALint errorCode);

ALuint getsourceAudio(struct audioInfo *audio);

ALuint getbuffAudio(struct audioInfo *audio);

ALuint freebuffAudio(struct audioInfo *audio,ALuint n);

int freesourceAudio(struct audioInfo *audio,ALuint n);

int checksourceAudio(struct audioInfo *audio,ALuint n);

int startAudio(struct audioInfo *audio,int numsource,int numbuff);
int stopAudio(struct audioInfo *audio);
int updateAudio(struct audioInfo *audio);

#endif


