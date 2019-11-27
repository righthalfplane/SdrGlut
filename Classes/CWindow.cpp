#include "CWindow.h"
#include "Matrix.h"
#include <stdarg.h>

int ReDrawScene(struct Scene *scene);

void SetLighting(unsigned int mode);

extern char *ProgramVersion;

extern "C" int closeScene(struct Scene *scene);

extern "C" int ReDrawSceneC(struct Scene *scene);

static int mMenuSelect(struct Scene *scene,int item);



CWindow::~CWindow()
{
	
}
CWindow::CWindow(struct Scene *scene)
{
	this->scene=scene;
	this->OpenError=FALSE;
	this->CNext=NULL;
}

int InvalRectMyWindow(struct Scene *scene)
{
    struct SceneList *list;
    int save_window;
    
    if(!scene)return 1;
    
    save_window=glutGetWindow();
    
    list=SceneFindScene(scene);
    if(list == NULL)
    {
        return 1;
    }
    
    if(list->window >= 0){
        glutSetWindow(list->window);
        glutPostRedisplay();
    }
    
    if(save_window && (save_window!=glutGetWindow()))glutSetWindow(save_window);
    
    return 0;
}

int CWindow::SetFrequency(struct Scene *scene,double f,double bw,int messageType)
{
    if(!scene)return 1;
    return 0;
}
int SetFrequencyGlobal(struct Scene *scene,double f,double bw,int messageType)
{
    if(!scene)return 1;
    
    CWinPtr myAppl=FindScene(scene);
    if(myAppl){
        return myAppl->SetFrequency(scene,f,bw,messageType);
    }
    return 0;
}


int CWindow::BackGroundEvents(struct Scene *scene)
{
    if(!scene)return 1;

	return 0;
}
int BackGroundEvents(struct Scene *scene)
{
	if(!scene)return 1;
	
	CWinPtr myAppl=FindScene(scene);
	if(myAppl){
		return myAppl->BackGroundEvents(scene);
	}
	
	
	return 0;
}

int CWindow::SelectionBoxUpdate(struct Scene *scene,int flag)
{
	return 0;
}
int CWindow::UpdateTemperatureColors(struct Scene *scene)
{
	return 0;
}
int CWindow::ReDrawScene(struct Scene *scene)
{
	return 0;
}
int CWindow::PrintTemperatures(struct Scene *scene)
{
	return 0;
}
int CWindow::FindPoint(struct Scene *scene,int x,int y)
{
	return 0;
}

int CWindow::LoadFile(struct Scene *scene,char *name,int fileType)
{
	return 0;
}
void display(void)
{	
	struct SceneList *list;
	struct Scene *scene;
	list=SceneFindByNumber(glutGetWindow());
	if(!list)return;
	scene=&list->scene;
	
	sceneDisplay(scene);
	
}


int ReDrawSceneC(struct Scene *scene)
{
	return ReDrawScene(scene);
}
int ReDrawScene(struct Scene *scene)
{
	
	
	CWinPtr myAppl=FindScene(scene);
	if(myAppl){
		myAppl->ReDrawScene(scene);
	}
	
	return 0;
}

int CWindow::Keys(struct Scene *scene,unsigned int key, int x, int y)
{
	return 0;
}
void keyss(int key, int x, int y)
{
}
void keys(unsigned char key, int x, int y)
{
    fprintf(stderr,"Cwindow keys - key %d key %c \n",key,key);
}
void myReshape(int w, int h)
{
	
}
void getMouse(int button, int state, int x, int y)
{
}
void moveMouse(int x, int y)
{
}

void palette_select(int item)
{
	struct SceneList *list;
	struct Scene *scene;
	list=SceneFindByNumber(glutGetWindow());
	if(!list)return;
	scene=&list->scene;

    getPalette(item,(char *)g.palname,(char *)g.palette);
	UpdateTemperatureColors(scene);
	glutPostRedisplay();
	
}
void menu_select(int item)
{
	struct SceneList *list;
	list=SceneFindByNumber(glutGetWindow());
	if(list){
	    mMenuSelect(&list->scene,item);
	}
}
int closeScene(struct Scene *scene)
{
	struct SceneList *list;
	
	if(!scene)return 1;
	
	CWinPtr myAppl=FindScene(scene);
	
	if(myAppl){
		RemoveCWindow(myAppl);
	}
	
	list=SceneFindScene(scene);
	if(list){
		int window11;
		window11=list->window;
		SceneDeleteByScene(scene);
		if(window11 >= 0)glutDestroyWindow(window11);
	}

	return 0;
}

static int mMenuSelect(struct Scene *scene,int item)
{
	if(!scene)return 1;

	return 0;
}


int CWindow::OpenWindows(struct Scene *scene)
{
	if(!scene)return 1;
		return 0;
}

void SetLighting(unsigned int mode)
{
	/*
	 GLfloat position[4] = {7.0,-7.0,-12.0,0.0};
	 */
	
	GLfloat position[4] = {700.0,700.0,1200.0,0.0};
	GLfloat ambient[4]  = {0.2,0.2,0.2,1.0};
	GLfloat diffuse[4]  = {1.0,1.0,1.0,1.0};
	GLfloat specular[4] = {1.0,1.0,1.0,1.0};
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {90.0};
	
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	
	switch (mode) {
		case 0:
			break;
		case 1:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			break;
		case 2:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			break;
		case 3:
			//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			//glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			break;
		case 4:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			break;
	}
	
	glLightfv(GL_LIGHT0,GL_POSITION,position);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glEnable(GL_LIGHT0);
}
int UpdateTemperatureColors(struct Scene *scene)
{
	if(!scene)return 1;
	
	CWinPtr myAppl=FindScene(scene);
	if(myAppl){
		return myAppl->UpdateTemperatureColors(scene);
	}
	return 1;
}

CWinPtr FindScene(struct Scene *scene)
{
	CWinPtr w;
	if(!scene || !Root)return NULL;
	
	w=Root;
	while(w){
		if(w->scene == scene)return w;
		w=w->CNext;
	}

	return NULL;
}

int CWindow::New()
{
	CWinPtr w = new CWindow(NULL);
	
	if(w == NULL){
	    WarningBatch((char *)"Out of Memory");
	    return 1;
	}
	
	if(w->OpenError != FALSE){
	    delete w;
	    return 0;
	}
	
	myAppl=w;
	
	AddWindowList(myAppl);
	
	return 0;
}
void AddWindowList(CWinPtr w)
{
	CWinPtr p,o;
	
	o=NULL;
	p=Root;
	
	while(p){		/*	Search for end of List	*/
	    o=p;
	    p=p->CNext;
	}
	p=w;
	if(o){
	    o->CNext=p;
	} else {
	    Root=p;
	}
}
CWinPtr RemoveCWindow(CWinPtr i)
{
	CWinPtr p,o;
	
	p=Root;
	o=NULL;
	
	while(p){		/*	Search for Icon	*/
	    if(p == i)break;
	    o=p;
	    p=p->CNext;
	}
	
	if(!p)return NULL;
	
	if(o)o->CNext=p->CNext; /*	Connect link	*/
	
	if(p == Root){
	    if((o=p->CNext) != NULL){
			Root=o;
	    } else {
			o=NULL;
	    	Root=NULL;
	    }
	}
	
	delete p;
	
	return o;
}
