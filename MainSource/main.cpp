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

int printInfo(void);

char ApplicationDirectory[2048];

char WarningBuff[256];

static GLUI *glui2;
static GLUI *glui;

int dialogStart(void);

static void control_cb(int control);

static void control_cb2(int control);

char *ProgramVersion=(char *)"SdrGlut-820";

extern "C" struct Scene *sceneRoot(void);

static int mTimer(void);

static int doWorking(struct Scene *scene,int check,int main_window);

extern "C" int BackGroundEvents(struct Scene *scene);

int ReDrawScene(struct Scene *scene);

static int doAbout(void);

int WindowToDestroy=0;

extern "C" int glutMain(int argc, char *argv[]);

int doRadioOpenRA(void);

static int argcs;

static char **argvs;

struct audioInfo *audio;

struct audioInfo audioStruct;

int mainClean(void)
{
	struct SceneList *list;
	struct Scene *scene;	
	
	SceneStartList();
	
	while((list=SceneNextList()) != NULL){
		
		scene=&list->scene;
		
		sceneClean(scene);
	}
	
	SceneClose();
	
	return 0;
	
}

int SetWindowDefault(void)
{
	/* glutSetWindow(main_window); */
	return 0;
}

struct Scene *sceneRoot()
{
	return NULL;
}


static void timer(int value)
{
	if(WindowToDestroy)
	{
		int iwin=glutGetWindow();
		glutDestroyWindow(WindowToDestroy);
		if(iwin && (iwin != WindowToDestroy)){
			glutSetWindow(iwin);
		}else{
			glutSetWindow(1);
		}
		WindowToDestroy=0;
		glutTimerFunc(50,timer,50);
		return;
	}

	mTimer();
	
	glutTimerFunc(50,timer,50);

}
static int mTimer()
{	
	struct SceneList *list;
	struct Scene *scene;	
	int save_window;
	int main_window;
	int check;
	
	save_window=glutGetWindow();
	
	SceneStartList();
	
	check=qCheck();
	
	while((list=SceneNextList()) != NULL){
        
		scene=&list->scene;

		main_window=list->window;
		
		if(main_window)glutSetWindow(main_window);
		
		BackGroundEvents(scene);
		
		doWorking(scene,check,main_window);
		
		if(check == 3){
			ReDrawScene(scene);
			if(main_window)glutSetWindow(main_window);
			glutPostRedisplay();
		}
	}
	
	if(save_window && (save_window!=glutGetWindow()))glutSetWindow(save_window);
		
	return 0;
}
static int doWorking(struct Scene *scene,int check,int main_window)
{
	char title[512];
	
	if(!scene || !main_window)return 1;
	
	if(check == 3){
		glutSetWindow(main_window);
		glutSetWindowTitle(scene->WindowTitle);
		scene->workTimer=0;
		scene->workFlip=0;
	} else if(check == 2){
	    if(scene->workTimer== 0.0){
			scene->workTimer=rtime();
	    }else{
			if(rtime() > scene->workTimer + 1.0){
				scene->workTimer=0.0;
				scene->workFlip = !scene->workFlip;
				if(scene->workFlip){
					mstrncpy(title,(char *)"(working) ",sizeof(title));
				}else{
					title[0]=0;
				}
				mstrncat(title,scene->WindowTitle,sizeof(title));
				glutSetWindow(main_window);
				glutSetWindowTitle(title);
			}
		}
	}
	
	return 0;
}

void display22(void)
{	
	;
}


int dialogStart(void)
{
	
	if(glui){
		glui->close();
	}
	
	glui = GLUI_Master.create_glui( ProgramVersion);
		
	new GLUI_Button(glui, "About", 1, control_cb); 
			
    new GLUI_Button(glui, "Close", 3, control_cb);
    
    new GLUI_Button(glui, "Files", 5, control_cb);

    new GLUI_Button(glui, "Radio", 8, control_cb);
    
//    new GLUI_Button(glui, "Time", 9, control_cb);

	new GLUI_Button(glui, "Quit", 4, control_cb);
	
	glui->set_main_gfx_window( glutGetWindow() );
	
	return 0;
}
static void control_cb(int control) 
{
	if(control == 4)
	{
		dialogQuit();
	} else if(control == 3){
		if(glui)glui->close();
		glui=NULL;
    }else if(control == 5){
        dialogFunctionPtr=NULL;
        dialogFileOpen((struct Scene *)NULL);
    }else if(control == 8){
        doRadioOpenRA();
//    }else if(control == 9){
//       int dialogTime(void);
//        dialogTime();
    } else if(control == 1)
	{
		
		/*
		  double stereo;
		 stereo=10.03;
		 glGetDoublev(GL_STEREO,&stereo);
		 WarningPrint("stereo %g\n",stereo);
		 */
		
		doAbout();
	}
    
    
	
}
static int doAbout(void)
{
	if(glui2){
		glui2->close();
	}
	
	glui2 = GLUI_Master.create_glui(ProgramVersion);

	if(!glui2)return 1;
	
	GLUI_StaticText *p = new GLUI_StaticText(glui2, ProgramVersion); 
	p->set_alignment(GLUI_ALIGN_CENTER);
	
	new GLUI_StaticText(glui2, ""); 
	p = new GLUI_StaticText(glui2, "Created by Dale Ranta"); 
	p->set_alignment(GLUI_ALIGN_CENTER);
		
	new GLUI_StaticText(glui2, ""); 
	p = new GLUI_StaticText(glui2, "Copyright 2011-2021 Dale Ranta. All rights reserved."); 
	p->set_alignment(GLUI_ALIGN_CENTER);
	
	new GLUI_Button(glui2, "Close", 3, control_cb2); 
	
	glui2->set_main_gfx_window( glutGetWindow() );

	return 0;
}
static void control_cb2(int control) 
{
	if(control == 3)
	{
		if(glui2)glui2->close();
		
		glui2=NULL;
	}
}
int main (int argc, char *argv[]) {
    
	SoapyNetSDR_SocketInit socket_init;

    fprintf(stderr,"LIQUID_VERSION_NUMBER %d\n",LIQUID_VERSION_NUMBER);
    
    SoapySDR_setLogLevel(SOAPY_SDR_WARNING);
    //SoapySDR_setLogLevel(SOAPY_SDR_TRACE);

    // signal(SIGPIPE, SIG_IGN);
    
    for(int n=1;n<argc;++n){
        if(processFile(argv[n]))break;
    }
    if(argc > 1)exit(0);
    
    printInfo();
    
    if(startAudio(&audioStruct,6,18)){
        fprintf(stderr,"startAudio Failed\n");
        exit(1);
    }
    
    audio=&audioStruct;
    
    argcs=argc;
    argvs=argv;
    
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH /* | GLUT_STEREO */);
    glutInitWindowSize(800,608);
    
    if(!GetWorking(ApplicationDirectory,sizeof(ApplicationDirectory))){
        WarningPrint("scan_dir  Working directory error %s\n",ApplicationDirectory);
    }
    
    
    dialogStart();
    
    glutTimerFunc(50,timer,50);
    
    glutMainLoop();
    
    return 0;
}
int getPaletteByName(char *palname,unsigned char *pal)
{
    
    if(!palname || !pal)return 1;
    
    getPalette(12,NULL,(char *)pal);
    
    return 1;
}

