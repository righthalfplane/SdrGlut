#ifndef __GRAPHICSH__
#define __GRAPHICSH__
#include <vector>

//#define GLEW_IN
#ifdef GLEW_IN
#include <GL/glew.h>
#endif

#ifdef __APPLE__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

typedef struct uRectStruct {
    int x;
    int y;
    int xsize;
    int ysize;
} uRect;

#define FILTER_RECTANGULAR     0
#define FILTER_HANN            1
#define FILTER_HAMMING         2
#define FILTER_FLATTOP         3
#define FILTER_BLACKMANHARRIS  4
#define FILTER_BLACKMANHARRIS7 5


int doWindow(float *x,long length,int type);
int DrawLine(int x1, int y1, int x2, int y2);
int DrawLine3(int x1, int y1, int x2, int y2);
int DrawString(int x, int y, char *out);
void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small);
void GridPlotNeat2(double *xmnc,double *xmxc,double *Large,double *Small);
int getPalette(int n,char *name,double *pal);
int getPaletteNames(std::vector<std::string>&names);	
int DrawBox(uRect *box,int offset);
int colorit(long count,double *level,double value,int *ic);
void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);

#endif
