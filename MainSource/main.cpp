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

char *ProgramVersion=(char *)"SdrGlut-889";

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

static int doHelp1();

int WriteToHelpWindow(char *message);

int WriteToHelpTop();

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
			
    new GLUI_Button(glui, "Radio", 8, control_cb);
    
    
    new GLUI_Button(glui, "Files", 5, control_cb);
    
    new GLUI_Button(glui, "Close", 3, control_cb);

//    new GLUI_Button(glui, "Time", 9, control_cb);
    
    new GLUI_Button(glui, "Help", 2, control_cb);

	new GLUI_Button(glui, "Quit", 4, control_cb);
	
	glui->set_main_gfx_window( glutGetWindow() );
	
	return 0;
}

static void control_cb(int control) 
{
	if(control == 4)
	{
		dialogQuit();
    } else if(control == 2){
        doHelp1();
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

static int doHelp1()
{
    WriteToHelpWindow((char *)"\n                                                    SdrGlut Help\n");
    WriteToHelpWindow((char *)"\n");
    WriteToHelpWindow((char *)"\
SrdGlut uses the GLUT tool kit. This tool kit has one quirk - \nall of the menus are \
hidden under a right click of the mouse button.\n");
    WriteToHelpWindow((char *)"\n                                                    Setting The Frequency\n\n");
    WriteToHelpWindow((char *)"\
The are many ways to set the Frequency.\n\nAfter you click\
 the Radio Button, a window opens where you can set the initial Frequency.\n\n\
In the Power Spectrum and Waterfall window, there are several more ways to set the Frequency.\n\n\
Left click the mouse anywhere in the upper or lower part of the window to set the Frequency.\n\n\
Spin the center Scroll button up or down to change the Frequency in bandwidth size steps.\n");
    WriteToHelpWindow((char *)"\n\
Where the Frequency: is display at the upper left hand side of the window\n\
You can left click in the upper half of the numbers to increase the Frequency\n\
You can left click in the lower half of the numbers to decrease the Frequency\n");
    WriteToHelpWindow((char *)"\n\
In the Specturm, you can left click and hold the mouse down to drag the Frequency.\n\n\
In the Waterfall, you can left click and hold the mouse down to drag the Frequency Range.\n");
    WriteToHelpWindow((char *)"\n\
In the Zoom window, you can left click the mouse to change the Frequency or \n\
you can left click and drag the mouse the change the Frequency Range.\n");
    WriteToHelpWindow((char *)"\n\
In the Specturm, you can right click and select the \"SDR Dialog\".\n\
In the Dialog, Set the \"Frequency\" and the \"Center Frequency\" and hit the \"Set Frequency\".\n\n\
The \"Frequency\" and the \"Center Frequency\" must be with 0.5 *samplerate of one another\n\n\
You can also use a \"Frequency File\" as dicussed in the \"File\" Section\n");

    
    
    WriteToHelpWindow((char *)"\n                                                    The Radio Button\n\n");
    WriteToHelpWindow((char *)"\
The Radio Button is used to select the Software Defined Radio that you \
want to use.\nClick button that has the name of the desired device to select and run it.\n\
It is also used set the initial Sample Rate and Frequency.\n\n");
    WriteToHelpWindow((char *)"\
If a \"SoapySDRUtil --find\" finds your device, but SdrGlut doesn't\n\
you have two version of the SoapySDR tool kit installed.\n\
Solutions to this common problem can be found at the Google Group \"sdrglut-users\" .\n");
    
    WriteToHelpWindow((char *)"\n                                                    The Files Button\n\n");
    WriteToHelpWindow((char *)"\
The File Dialog has three file types.\n\n\
The first is a I/Q file that SdrGlut has created from the menu \"Recording->IQ Recording\" option.\n\
It opens to a Spectrum and Waterfall window that works as usual.\n\n\
The second is Frequency file that was created by or for SdrGlut.\n\
Examples - \"SdrGlut->misc->scanlists\".\n\
Usage - Select a list item then right click and select \"Set Frequency.\"\n\
Usage - Select a item then use Cntl-F go down list or Cntrl-B go up list of Frequencies.\n\
Usage - Select a list then right click and select \"Send Scan Frequencies.\"\n\
To Transfer the Frequency list to a Waterfall window for scanning.\n\
Usage - When you hit the \"s\" key the Frequency information is written to the default Frequency file. \n\n\
The third is a digital filter file - \"SdrGlut->misc->filters->filter-examples\".\n\
The \"writefilter\" command, writes the filter out as a standalone program with testing.\n");
    WriteToHelpTop();
    
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

