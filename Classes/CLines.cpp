/*
 *  CLines.cpp
 *  Rx3
 *
 *  Created by Dale Ranta on 1/29/14.
 *  Copyright 2014 Dale Ranta. All rights reserved.
 *
 */
#include <string.h>
#include "DialogSave.h"
//#include "SceneBatch.h"
#include "CLines.h"
#include "GridPlot.h"
#include "Contours.h"

#define ControlLineAttributes	4
#define ControlDialogSave		5
#define ControlSaveData			6
#define ControlPlotAttributes	7
#define ControlPlotBox			8
#define ControlPlotLabels		10


static void menu_selectc(int item);

extern "C" int closeScene(struct Scene *scene);

extern "C" int	rMoveTo(double x,double y,DOListPtr d);

extern "C" int rDrawString(char *name,DOListPtr d);

extern "C" int uGetFontInfo(int myIcon,uFontInfo *t);

extern "C" double PixelToLocal(long x,DOListPtr d);

extern "C" int LocalToPixel(double x,DOListPtr d);

static int DefaultFont;

static int DefaultFontSize;

static int DrawStrings(int x, int y, char *out);

int LoadLineTimeHistory(struct Scene *scene,double *xDi,double *yDi,int ysize,long lineReplace);

static int SaveImage(struct Scene *scene);

static void displayc(void);

int DOListDelete(DOListPtr l);

int DODraw(DOListPtr l,long CurrentFrame);

static void moveMouse(int x, int y);

static int DrawBox(uRect *box,int offset);

static void keys2(unsigned char key, int x, int y);

CLinesPtr FindLinesWindow(int window)
{
    CLinesPtr f;
    CWinPtr w;
    
    if(!Root)return NULL;
    
    w=Root;
    while(w){
        if(w->scene->windowType == FileTypeLines){
            f=(CLinesPtr)w;
            if(f->window == window)return f;
        }
        w=w->CNext;
    }
    
    return NULL;
    
}


CLines::CLines(): CWindow(NULL)
{
	scene=NULL;
	
	lines=&lineHolder;
	
	zerol((char *)lines,sizeof(*lines));
    
    //sdr=NULL;
    
    sceneSource=NULL;
    
    Frequency=0;
    
    BandWidth=0;
    
    wShift=0;
    
}

CLines::~CLines()
{
	DOListDelete(&lines->l);
	
	sceneClean(scene);
    	
	// SceneBatchDeleteByScene(scene);
	
}
CLines::CLines(struct Scene *scene): CWindow(scene)
{
	OpenError=TRUE;
	
	lines=&lineHolder;
	
	zerol((char *)lines,sizeof(*lines));
	
	if(!scene)return;
	
	scene->selectionBox.draw=0;
			
	OpenError=FALSE;
	
}
int CLines::plotPutData(struct Scene *scene,double *x,double *y,long count,long replace)
{
	if(!x || !y)return 1;
	
	LoadLineTimeHistory(scene,x,y,(int)count,replace);
	
	return 0;
}

int CLines::Information(struct Scene *scene)
{
	if(!scene)return 1;
	return 0;
}

CLinesPtr CLines::CLinesOpen(struct Scene *scene,int windowParent)
{
	CLinesPtr lines = new CLines(scene);
	
	if(lines && lines->OpenError)
	{
		delete lines;
		lines=NULL;
		goto ErrorOut;
	}else if(!lines){
		goto ErrorOut;
	}
	
	if(lines->OpenWindows(scene,windowParent)){
		delete lines;
		lines=NULL;
		goto ErrorOut;
	}
	
	AddWindowList(lines);
	
ErrorOut:
	return lines;
}

static void menu_selectc(int item)
{
	struct SceneList *list;
	struct Scene *scene;
	list=SceneFindByNumber(glutGetWindow());
	if(!list)return;
	scene=&list->scene;
	
	CLinesPtr images=(CLinesPtr)FindScene(scene);
	
	images->menu_select(scene,item);
	
}

static void displayc(void)
{
	struct SceneList *list;
	struct Scene *scene;
    
	list=SceneFindByNumber(glutGetWindow());
	if(!list)return;
	scene=&list->scene;
	
    if(!scene){
        fprintf(stderr,"scene %p\n",scene);
    }
    
	CLinesPtr images=(CLinesPtr)FindScene(scene);
	
	images->display(scene);
}
static void myReshape(int w, int h)
{
	struct SceneList *list;
	struct Scene *scene;
	list=SceneFindByNumber(glutGetWindow());
	if(!list)return;
	scene=&list->scene;
	
	CLinesPtr images=(CLinesPtr)FindScene(scene);
	
	images->reshape(scene,w,h);
}
int CLines::OpenWindows(struct Scene *scene)
{
    return 1;
}
static void getMousel(int button, int state, int x, int y)
{
    struct SceneList *list;
    list=SceneFindByNumber(glutGetWindow());
    if(!list)return;
    
    CLinesPtr myAppl=(CLinesPtr)FindScene(&list->scene);
    
    myAppl->getMouse(button,state, x, y);
    
}

void CLines::getMouse(int button, int state, int x, int y)
{
    
    if(button == 3){
        if(sceneSource){
            if(wShift == 0){
                Frequency -= BandWidth*0.5;
            }else{
                Frequency -= BandWidth*0.25;
            }
            SetFrequencyGlobal(sceneSource,Frequency,BandWidth,M_FREQUENCY);
        }
        if(state == GLUT_DOWN)return;
    }else if(button == 4){
        if(sceneSource){
            if(wShift == 0){
                Frequency += BandWidth*0.5;
            }else{
                Frequency += BandWidth*0.25;
            }
            SetFrequencyGlobal(sceneSource,Frequency,BandWidth,M_FREQUENCY);
        }
        if(state == GLUT_DOWN)return;
    }

    
   	if(state == GLUT_DOWN)
    {
        double dpi=lines->l.dpi;
        if(sceneSource){
            Frequency=lines->Plot->xViewMin+(x-dpi*lines->Plot->box.x)*(lines->Plot->xViewMax-lines->Plot->xViewMin)/(dpi*lines->Plot->box.xsize);
            SetFrequencyGlobal(sceneSource,Frequency,BandWidth,M_FREQUENCY);
        }
        lines->Plot->xAutoMaximum=FALSE;
        lines->Plot->xSetMaximum=lines->Plot->xMaximum;

        lines->Plot->xAutoMinimum=FALSE;
        lines->Plot->xSetMinimum=lines->Plot->xMinimum;
    }else{
        if(sceneSource){
           SetFrequencyGlobal(sceneSource,Frequency,BandWidth,M_FREQUENCY_BANDWIDTH);
        }
        lines->Plot->xAutoMaximum=TRUE;
        lines->Plot->xAutoMinimum=TRUE;
   }
}
static void moveMouse(int x, int y)
{
    struct SceneList *list;
    list=SceneFindByNumber(glutGetWindow());
    if(!list)return;
    
    CLinesPtr myAppl=(CLinesPtr)FindScene(&list->scene);
    if(!myAppl)return;
    struct CLines::LineStruct *lines=myAppl->lines;
    if(!lines)return;
    double dpi=lines->l.dpi;
    
    if(myAppl->sceneSource){
        myAppl->Frequency=lines->Plot->xViewMin+(x-dpi*lines->Plot->box.x)*(lines->Plot->xViewMax-lines->Plot->xViewMin)/(dpi*lines->Plot->box.xsize);
        SetFrequencyGlobal(myAppl->sceneSource,myAppl->Frequency,myAppl->BandWidth,M_FREQUENCY);
    }
}
int CLines::SetFrequency(struct Scene *scene,double f,double bw,int message)
{
    
    CLinesPtr lines=(CLinesPtr)FindScene(scene);
    
    if(!lines)return 1;
    
    if(lines->sceneSource){
        lines->Frequency=f;
        lines->BandWidth=bw;
    }
    return 0;
}
int CLines::OpenWindows(struct Scene *scene,int windowParent)
{
	struct SceneList *list;
	uGridPlotPtr Plot;
	DOListPtr l;
	
	if(!scene)return 1;
	
	list=SceneFindScene(scene);
	if(list == NULL)
	{
		WarningPrint("CLines::OpenWindows : Error Could Not Find Scene\n");
		return 1;
	}
	
	scene->ThreadCount=8;
	
		
	if( scene->stereoType == StereoDoubleBuffered){
	    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO);
	}else if(scene->stereoType == StereoSingleBuffered){
		glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH  | GLUT_STEREO);
	}else{
	    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	}
    
	glutInitWindowSize(800,608);
    
    if(windowParent > -1000){
        window = glutCreateSubWindow(windowParent,0,0,800,304);
    }else{
        window = glutCreateWindow(scene->WindowTitle);
    }
	list->window=window;
	
	glutDisplayFunc(displayc);
	
	glutCreateMenu(menu_selectc);
	glutAddMenuEntry("Line Attributes...", ControlLineAttributes);
	glutAddMenuEntry("Plot Attributes...", ControlPlotAttributes);
	glutAddMenuEntry("Plot Labels...", ControlPlotLabels);
	glutAddMenuEntry("Plot Box...", ControlPlotBox);
	glutAddMenuEntry("Save Image...", ControlDialogSave);
	glutAddMenuEntry("Save Data...", ControlSaveData);
	glutAddMenuEntry("-----------------", -1);
	glutAddMenuEntry("Close", ControlClose);
	glutAddMenuEntry("Quit", ControlQuit);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glClearColor(1.0,1.0,1.0,0.);
	
	glutReshapeFunc(myReshape);
    
    glutMouseFunc(getMousel);
    
    glutMotionFunc(moveMouse);
    
    glutKeyboardFunc(keys2);

	lines->scene=scene;
	
	lines->window=window;
	
	l=&lines->l;
	
	l->xsize=800;
	l->ysize=304;
	l->dpi=72;
	l->raster=RASTER_24;
	
	zerol((char *)&l->Attributes,(long)sizeof(l->Attributes));
	
	l->Attributes.font.txFont=DefaultFont;
	l->Attributes.font.txFace=0;
	l->Attributes.font.txSize=DefaultFontSize;
	
	l->Attributes.LinePattern=0;
	l->Attributes.LineSymbol=0;
	l->Attributes.SolidPattern=19;
	l->Attributes.nLineFore=255;
	l->Attributes.nLineBack=0;
	l->Attributes.nSolidFore=0;
	l->Attributes.nSolidBack=0;
	l->Attributes.xLineWidth=1;
	l->Attributes.yLineWidth=1;
	l->Attributes.DashFlag=FALSE;
	mstrncpy(l->Attributes.LineDash,(char *)"9",16);	 	
	mstrncpy(l->Attributes.AnimationFormat,(char *)"%.0f\0",10);
	
	mstrncpy(l->LinePatterns[0],(char *)"9",16);
	mstrncpy(l->LinePatterns[1],(char *)"0",16);
	mstrncpy(l->LinePatterns[2],(char *)"34",16);
	mstrncpy(l->LinePatterns[3],(char *)"56",16);
	mstrncpy(l->LinePatterns[4],(char *)"78",16);
	mstrncpy(l->LinePatterns[5],(char *)"145541",16);
	mstrncpy(l->LinePatterns[6],(char *)"1222",16);
	mstrncpy(l->LinePatterns[7],(char *)"1112",16);
	l->NumberOfPatterns=8;
	
	l->nLineFore=255;
	
	getPalette(12,(char *)&l->palname,(char *)&l->palette);
	
	l->red=255;
	l->green=255;
	l->blue=255;
	
	Plot=DOGridPlotCreate(l);
	if(!Plot)goto ErrorOut;
	lines->Plot=Plot;
	
ErrorOut:	
	return 0;
}

static void keys2(unsigned char key, int x, int y)
{
    CLinesPtr l=FindLinesWindow(glutGetWindow());
    
    if(key == 'm'){
        if(l->sceneSource)SetFrequencyGlobal(l->sceneSource,l->Frequency,l->BandWidth,M_MUTE);
    }else if(key == 's'){
        if(l->sceneSource)SetFrequencyGlobal(l->sceneSource,l->Frequency,l->BandWidth,M_SAVE);
    }
   // fprintf(stderr,"Clines keys - key %d key %c lines %p\n",key,key,l);
    
}

void CLines::menu_select(struct Scene *scene,int item)
{
	
	 switch(item)
	 {
	 case ControlClose:
			 closeScene(lines->scene);
			 break;
				 
			 
	 case ControlQuit:
			 dialogQuitC();
			 break;
	 
	 case ControlLineAttributes:
			 LineAttributes(scene, lines);
			 break;
	 
	 case ControlPlotAttributes:
			 PlotAttributes(scene, lines);
			 break;
			 
		 case ControlPlotBox:
			 PlotBox(scene, lines);
			 break;
			 
		 case ControlPlotLabels:
			 PlotLabels(scene, lines);
			 break;
			 
		 case ControlDialogSave:
			 SaveImage(scene);
			 break;
			
		 case ControlSaveData:
			 //dialogSaveC(scene, SaveDatac, 1,NULL);
			 break;
			 
			 
	 /*
	 case ControlDialogSave:
	 if(!data || !scene)return;
	 dialogSaveC(scene, SaveIt, 0,NULL);
	 break;
	 case ControlSaveData:
	 if(!data || !scene)return;
	 dialogSaveC(scene, SaveData, 1,NULL);
	 break;
	 */
	 }
	 
	return;
}
int CLines::SaveData(struct Scene *scene,char *name)
{
	uGridPlotPtr Plot;
	uLineDataPtr Line;
	FILE *out;
	long maxlines;
	long m;
	int n;
	
	
	Plot=lines->Plot;
	if(!Plot)return 1;
	if(Plot->LineCount <= 0)return 1;
	
	/* WarningPrint("SaveData scene %p\n",scene); */
	
	out=fopen(name,"w");
	if(!out){
		WarningPrint("SaveData Could Not Open %s to Write\n",name);
		return 1;
	}

	
	maxlines=0;
	for(n=0;n<Plot->LineCount;++n){
		Line=Plot->Lines[n];
		fprintf(out,"x-%s,y-%s",Line->save,Line->save);
		if(n < Plot->LineCount-1)fprintf(out,",");
		if(Line->dCount > maxlines)maxlines=Line->dCount;
	}
	fprintf(out,"\n");
	
	for(m=0;m<maxlines;++m){
		for(n=0;n<Plot->LineCount;++n){
			Line=Plot->Lines[n];
		    if(m < Line->dCount){
				/* fprintf(out,"%8.6e,%8.6e",Line->xData[m],Line->yData[m]); */
				fprintf(out,"%g,%8.6e",Line->xData[m],Line->yData[m]);
			}else{
				fprintf(out,",");
			}
			if(n < Plot->LineCount-1)fprintf(out,",");
		}
		fprintf(out,"\n");
	}
	
	if(out)fclose(out);
	return 0;
}
static int SaveImage(struct Scene *scene)
{
	if(!scene)return 1;
	
	//dialogSaveC(scene,SaveIt,0,NULL);
	
	return 0;
}
void CLines::reshape(struct Scene *scene,int wscr,int hscr)
{
/*
	lines->w=wscr/4; lines->h=hscr/4;
	wscr=lines->w*4;
	hscr=lines->h*4;
*/
	lines->w=wscr; lines->h=hscr;
    
    //printf("CLines::reshape wscr %d hscr %d\n",wscr,hscr);
	
	glViewport(0,0,(GLsizei)lines->w,(GLsizei)lines->h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	lines->xmin=lines->ymin=0.0; lines->xmax=lines->ymax=1.0;
	
	/*
	if(lines->w<=lines->h)
		lines->ymax=1.0*(GLfloat)lines->h/(GLfloat)lines->w;
	else
		lines->xmax=1.0*(GLfloat)lines->w/(GLfloat)lines->h;
	*/
	if(lines->w<=lines->h){
		lines->ymax=lines->h*(GLfloat)lines->h/(GLfloat)lines->w;
		lines->xmax=lines->w;
	}else{
		lines->xmax=lines->w*(GLfloat)lines->w/(GLfloat)lines->h;
		lines->ymax=lines->h;
	}
	
	
	gluOrtho2D(lines->xmin,lines->xmax,lines->ymin,lines->ymax);
	
    glMatrixMode(GL_MODELVIEW);
    
	
	lines->l.xsize=wscr;
	lines->l.ysize=hscr;
    
    if(sceneSource == NULL){
        lines->Plot->box.x=1.0;
        lines->Plot->box.y=0.5;
        lines->Plot->box.xsize=(lines->w)/72.0-lines->Plot->box.x-1.25;
        lines->Plot->box.ysize=(lines->h)/72.0-lines->Plot->box.y-0.25;
        lines->Plot->mode=0;
    }else{
        lines->Plot->box.x=0.0;
        lines->Plot->box.y=0.0;
        lines->Plot->box.xsize=(lines->w)/72.0;
        lines->Plot->box.ysize=(lines->h)/72.0;
        lines->Plot->mode=1;
    }

}

void CLines::display(struct Scene *scene)
{
    
    if(!scene)return;
    
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, lines->w, 0, lines->h);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		
		glRasterPos2i(0,0);		/* Position at base of window */
		
		/*		if(g.dpp != NULL)glDrawPixels( w, h, GL_RGB, GL_UNSIGNED_BYTE, g.dpp); */
		
		if(DrawIt(scene))return;
		
        uRect box;
        
        if(Frequency > 0){
            
            double dpi=lines->l.dpi;
            int x;
            
            x=0.5+dpi*lines->Plot->box.x+(Frequency-lines->Plot->xViewMin)*(dpi*lines->Plot->box.xsize)/(lines->Plot->xViewMax-lines->Plot->xViewMin);
            
            box.xsize=6;
            if(BandWidth > 0){
                int x2;
                
                x2=0.5+dpi*lines->Plot->box.x+(Frequency-BandWidth-lines->Plot->xViewMin)*(dpi*lines->Plot->box.xsize)/(lines->Plot->xViewMax-lines->Plot->xViewMin);
                box.xsize=(x-x2);

            }
            
            if(box.xsize < 7)box.xsize=7;
            
            box.x=x-box.xsize/2;
            box.y=dpi*lines->Plot->box.ysize;
            box.ysize=box.y;
            if(wShift > 0){
                box.x=x;
                box.xsize /= 2;
            }else if(wShift < 0){
                box.xsize /= 2;
            }
            DrawBox(&box,0);
        }
        
		glutSwapBuffers();
	}
	
	
}
static int DrawBox(uRect *box,int offset)
{
    if(box->xsize <= 0)return 0;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND); //Enable blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function.
    
    
    glColor4f(0.0, 1.0, 0.0, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(box->x, box->y+offset);
    glVertex2f(box->x+box->xsize, box->y+offset);
    glVertex2f(box->x+box->xsize, box->y-box->ysize+offset);
    glVertex2f(box->x, box->y-box->ysize+offset);
    glEnd();
    
    glDisable(GL_BLEND);
    
    //printf("DrawBox %d %d %d %d\n",box->x,box->x+box->xsize,box->y+offset,box->y-box->ysize+offset);
    
    //DrawLine(box->x, box->y+offset, box->x+box->xsize, box->y-box->ysize+offset);
    
    
    return 0;
}
int CLines::DrawIt(struct Scene *scene)
{
		
	if(!scene)return 1;
		
	glColor3f(0.,0.,0.);
	
	DODraw(&lines->l,0L);
	
	DrawLabels(scene);
	
	return 0;
}
int CLines::plotPutLabel(struct Scene *scene,char *label,long n)
{
	if(!scene)return 1;
	
	mstrncpy(&lines->labels.Labels[n][0],(char *)label,sizeof(lines->labels.Labels[0]));
	if(strlen(&lines->labels.Labels[n][0]) >= 1){
		lines->labels.flag[n]=1;
	}else{
		lines->labels.flag[n]=0;
	}
	
	return 0;
}

int CLines::DrawLabels(struct Scene *scene)
{
	if(!scene)return 1;
	
	double LineHeight,cWidth;
	uFontInfo Info;
	
	uGetFontInfo(0,&Info);
	
	LineHeight=PixelToLocal(Info.LineHeight,&lines->l);
	
	cWidth=PixelToLocal(Info.CWidth,&lines->l);
	
	lines->labels.x[0]=lines->Plot->box.x+0.5*lines->Plot->box.xsize-0.5*cWidth*strlen(lines->labels.Labels[0]);
	lines->labels.y[0]=lines->Plot->box.y+lines->Plot->box.ysize+LineHeight;
	
	
	lines->labels.x[1]=lines->Plot->box.x+0.5*lines->Plot->box.xsize-0.5*cWidth*strlen(lines->labels.Labels[1]);
	lines->labels.y[1]=lines->Plot->box.y-3*LineHeight;
	
	lines->labels.x[2]=lines->Plot->box.x+0.5*lines->Plot->box.xsize-0.5*cWidth*strlen(lines->labels.Labels[2]);
	lines->labels.y[2]=lines->Plot->box.y+lines->Plot->box.ysize+2*LineHeight;
	
	lines->labels.x[3]=lines->Plot->box.x+0.5*lines->Plot->box.xsize-0.5*cWidth*strlen(lines->labels.Labels[3]);
	lines->labels.y[3]=lines->Plot->box.y-4*LineHeight;
	
	
	for(int n=0;n<4;++n){
		if(lines->labels.flag[n]){
			rMoveTo(lines->labels.x[n],lines->labels.y[n],&lines->l);
			rDrawString(lines->labels.Labels[n],&lines->l);
		}
	}
	
		
	if(lines->labels.flag[4]){
		int x,y;
		
		lines->labels.x[4]=lines->Plot->box.x-cWidth*10;
		lines->labels.y[4]=lines->Plot->box.y+0.5*lines->Plot->box.ysize-0.5*cWidth*strlen(lines->labels.Labels[4]);
		
		x=LocalToPixel(lines->labels.x[4],&lines->l);
		y=LocalToPixel(lines->labels.y[4],&lines->l);
		/*printf("%f %f %d %d \n",lines->labels.x[4],lines->labels.y[4],x,y);*/
		DrawStrings(x, y,(char *)lines->labels.Labels[4]);
	}
	
	return 0;
}
static int DrawStrings(int x, int y, char *out)
{
	int h;
	
	if(!out)return 1;
	
	glPushMatrix();
	
	
	glTranslatef(x, y, 0);
	
	glRotatef(90.,0.0,0.0,1.0);
	
	glScalef(0.1, 0.1, 1.0);
	
	while((h=*out++))
	{
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, h);

	}
	
    glPopMatrix();
	
	return 0;
}

int LoadLineTimeHistory(struct Scene *scene,double *xDi,double *yDi,int ysize,long lineReplace)
{
	uGridPlotPtr Plot;
	uLineDataPtr Line;
	double *xD,*yD;
	DOListPtr l;
	int ret;
	
	CLinesPtr lines=(CLinesPtr)FindScene(scene);
	
	if(!lines || !xDi || !yDi)return 1;
	
	Plot=lines->lines->Plot;
	if(!Plot)return 1;
	l=&lines->lines->l;
	
	ret=1;
		
	xD=(double *)cMalloc(sizeof(double)*ysize,9234);
	yD=(double *)cMalloc(sizeof(double)*ysize,9234);
	
	if(!xD || !yD)goto ErrorOut;
	
	for(int n=0;n<ysize;++n){
		xD[n]=xDi[n];
		yD[n]=yDi[n];
	}
	
	if(Plot->LineCount > 0 && lineReplace >= 0 && lineReplace < Plot->LineCount){
	    Line=Plot->Lines[lineReplace];
		if(Line->xData)cFree((char *)Line->xData);
		if(Line->yData)cFree((char *)Line->yData);
		
		Line->xData=xD;
		Line->yData=yD;
		
		xD=NULL;
		yD=NULL;
		
		Line->dCount=ysize;
	}else{
	    char name[64];
		Line=DOLineDataCreate(NULL,&l->Attributes);
		if(!Line)return 1;
		
		Line->xData=xD;
		Line->yData=yD;
		
		xD=NULL;
		yD=NULL;
		
		Line->dCount=ysize;
		
		msprintf(name,sizeof(name),"Line-%ld",Plot->LineCount);
		mstrncpy(Line->save,name,64);
		
		if(ResizeArray(&Plot->Lines,sizeof(struct uLineData *),Plot->LineCount,&Plot->LineMax,2018))goto ErrorOut;
		Plot->Lines[Plot->LineCount++]=Line;
		Line->l=l;
	}
	
	ret=0;
	
ErrorOut:
	glutSetWindow(lines->lines->window);
	glutPostRedisplay();
	if(xD)cFree((char *)xD);
	if(yD)cFree((char *)yD);
	return ret;
}
extern "C" int LinesPlot(struct cdatal *d,void *lines);

// extern "C" int initScene(struct Scene *scene);

static int initScene(struct Scene *scene)
{
    
    if(!scene)return 1;
    
    SceneInit(scene);
    scene->windowType=FileTypeLines;

//    scene->FileType = FileTypeNone;
    
    scene->scale.showPalette=1;
    
    scene->scale.updateTemperatureScale=1;
    
    scene->scale.logscale=1;
    
    
    return 0;
}

int LinesPlot(struct cdatal *d,void *linesi)
{
    struct SceneList *list;
    struct Scene *scene;
    CLinesPtr lines;
    
    if(!d)return 1;
    
    if(!linesi){
        list=SceneNext();
        if(list == NULL)
        {
            WarningPrint("LinesPlot : Error Allocation Scene Memory File\n");
            goto ErrorOut;
        }
        scene=&list->scene;
        zerol((char *)scene,sizeof(struct Scene));
        
        initScene(scene);
        lines = CLines::CLinesOpen(scene,-12345);
        list->listPointer=lines;
        list->fileListName=NULL;
    }else{
        lines=(CLinesPtr)linesi;
    }
    
    for(long n=0;n<d->count;++n){
        lines->plotPutData(lines->scene,d->cList[n].xdata,d->cList[n].ydata,d->cList[n].nndata,-1L);
    }
    
    
ErrorOut:
    
    return 0;
}


