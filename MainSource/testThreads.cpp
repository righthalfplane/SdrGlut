#include "firstFile.h"
#include <cstdio>
#include <cstdlib>
#include <stdarg.h>
#define EXTERN
#include <SoapySDR/Version.h>
#include <SoapySDR/Modules.h>
#include <SoapySDR/Logger.h>
#include "Utilities.h"
#include "Scene.h"
#include "QuitDialog.h"
#include "BackgroundQ.h"
#include "SceneList.h"
#include "CWindow.h"
#include "DialogFileOpen.h"

#include "audiolib.h"

#include <GL/glui.h>

#include <liquid/liquid.h>

#include "BatchFile.h"

#include "SocketDefs.h"

char ApplicationDirectory[2048];

char WarningBuff[256];

int WindowToDestroy=0;
struct audioInfo *audio;

#include <stdio.h>
int main (int argc, char * argv [])
{ 
    printf("hello\n");
}
int mainClean(void)
{
    return 0;
}

int getPaletteByName(char *palname,unsigned char *pal)
{
    
    if(!palname || !pal)return 1;
    
    getPalette(12,NULL,(char *)pal);
    
    return 1;
}

