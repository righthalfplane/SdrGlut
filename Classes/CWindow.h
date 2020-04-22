#ifndef __CWINDOW__
#define __CWINDOW__

#include "firstFile.h"
#include "Scene.h"
#include "Utilities.h"
#include "SceneList.h"
#include "LoadFiles.h"
//#include "WriteJpeg.h"
#include "QuitDialog.h"

#define M_FREQUENCY             0
#define M_FREQUENCY_BANDWIDTH   1
#define M_MUTE                  2
#define M_SAVE                  3
#define M_SEND                  4

class CWindow{
public:	
	CWindow(struct Scene *scene);
	virtual int BackGroundEvents(struct Scene *scene);
    virtual int UpdateTemperatureColors(struct Scene *scene);
    virtual int SetFrequency(struct Scene *scene,double f,double bw ,int messageType);
	virtual int OpenWindows(struct Scene *scene);
	virtual int ReDrawScene(struct Scene *scene);
	virtual int FindPoint(struct Scene *scene,int x,int y);
	virtual int PrintTemperatures(struct Scene *scene);
	virtual int LoadFile (struct Scene *scene,char *name, int fileType);
	virtual int Keys(struct Scene *scene,unsigned int key, int x, int y);
	virtual int SelectionBoxUpdate(struct Scene *scene,int flag);

    virtual int sendMessage(char *m1,char *m2,int type);

	int New();
	virtual ~CWindow();
    char windowName[256];
	int OpenError;
    int window;
	struct Scene *scene;
	class CWindow *CNext;
};

typedef CWindow *CWinPtr;

EXTERN CWinPtr Root,myAppl;

CWinPtr FindScene(struct Scene *scene);

CWinPtr RemoveCWindow(CWinPtr i);

void AddWindowList(CWinPtr w);

extern "C" int InvalRectMyWindow(struct Scene *scene);

extern "C" int BackGroundEvents(struct Scene *scene);

extern "C" int SetFrequencyGlobal(struct Scene *scene,double f,double bw, int messageType);

#define KEY_OFFSET 1000

#define ControlQuit                    9
#define ControlClose               24


#define NoCommand	0
#define NewScene	1
#define QuitProgram	2

#endif
