/*
 *  sceneDisplay.c
 *  
 *
 *  Created by Dale Ranta on 6/19/13.
 *  Copyright 2013 Dale Ranta. All rights reserved.
 *
 */
#include <math.h>
#include <stdlib.h>

#include "sceneDisplay.h"
#include "LoadFiles.h"
//#include "ShowGLSL.h"
//#include "ViewFactorTests.h"
#ifdef __APPLE__
#include <GLUT/glut.h>	// GLUT OpenGL includes
#include <OpenGL/glext.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>	// GLUT OpenGL includes
#include <stdarg.h>
#endif

#include <stdarg.h>

#include <string.h>

#define DTOR            0.0174532925

GLuint textureId;

int txsize=400;
int tysize=400;

GLuint fBuffer;

GLuint rBuffer;

GLuint vboId = 0;                   // ID of VBO for vertex arrays


int w=800,h=400;

GLuint iBuff[3];


static int DrawSym(double xp,double yp,double hp,char *ib,double angle,int nchar);

static int smove(double x,double y);

static double xlast,ylast;

int sceneDisplay(struct Scene *scene)
{

	return 0;
}

int DrawLine(int x1, int y1, int x2, int y2)
{
    glBegin(GL_LINES);
    
    glVertex2f((x1),(y1));
    glVertex2f((x2),(y2));
    
    glEnd();
    
    return 0;
}
int DrawLine2(float x1, float y1, float x2, float y2)
{
    glBegin(GL_LINES);
    
    glVertex2f((x1),(y1));
    glVertex2f((x2),(y2));
    
    glEnd();
    
    return 0;
}
/*
void glutBitmapCharacter(void *in,int h)
{
    char c[2];
    int pos[4];
 
   // glPushMatrix();

    c[0]=h;
 
    for(int i=0;i<4;++i)pos[i]=0;

    glGetIntegerv(GL_CURRENT_RASTER_POSITION, pos);

    glRasterPos2i( (int)pos[0], (int)pos[1]);
    
    
   smove(pos[0],pos[1]);
    
    fprintf(stderr,"1 %c pos[0] %d pos[1] %d xlast %g ylast %g\n",c[0],pos[0],pos[1],xlast,ylast);

   DrawSym(999.,999.,10.0,c,0.0,1);
    
    glRasterPos2i( (int)xlast, (int)ylast);

    glGetIntegerv(GL_CURRENT_RASTER_POSITION, pos);
    
    fprintf(stderr,"2 %c pos[0] %d pos[1] %d xlast %g ylast %g\n",c[0],pos[0],pos[1],xlast,ylast);
//    glPopMatrix();

    return;
}
*/
int DrawString(int x, int y, char *out)
{
   // int h;
    
    if(!out)return 1;
    
    glPushMatrix();
    glRasterPos2i( x, y);
    
/*
    float pos[4];
    
    for(int i=0;i<4;++i)pos[i]=0;
    
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
    
    fprintf(stderr,"%s pos[0] %g pos[1] %g pos[2] %g pos[3] %g\n",out,pos[0],pos[1],pos[2],pos[3]);
*/
    
    /*
    while((h=*out++))
    {
        // glutBitmapCharacter(GLUT_BITMAP_9_BY_15, h);
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, h);
        
    }
     */
    
    DrawSym((double)x,(double)y,10.0,out,0.0,(int)strlen(out));
    
    glPopMatrix();
    
    return 0;
}

static int swhere(double *x,double *y)
{
    *x=xlast;
    *y=ylast;
    return 0;
}
static int smove(double x,double y)
{
    xlast=x;
    ylast=y;
    
    return 0;
}
static int sdraw(double x,double y)
{
    
    DrawLine2(xlast, ylast, x, y);

    xlast=x;
    ylast=y;
    
    return 0;
}

static int DrawSym(double xp,double yp,double hp,char *ib,double angle,int nchar)
{
    static int icp[]={
        1,7,17,21,26,31,36,41,45,49,55,64,74,79,81,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,
        0,0,0,0,0,0,0,0,
        83,87,91,101,107,110,112,117,119,128,134,143,159,163,
        172,183,188,204,215,0,0,215,220,0,0,220,229,243,251,258,
        265,271,283,291,299,305,313,316,321,325,334,341,353,363,
        373,378,384,387,392,397,403,412
    };
    
    static int ishape[]={
        47,17,11,71,77,47,47,37,15,13,31,51,73,75,57,47,47,11,71,47,
        47,41,00,14,74,17,71,00,11,77,47,14,41,74,47,47,14,74,47,41,
        11,77,17,71,17,77,11,71,17,44,77,00,44,41,17,71,00,11,77,17,
        11,71,77,47,41,00,14,74,77,17,11,71,74,77,11,71,17,77,47,41,
        00,00,27,16,12,21,47,56,52,41,25,43,00,33,35,00,45,23,00,24,
        44,24,44,00,35,33,32,41,30,24,44,31,41,42,32,31,11,57,21,12,
        16,27,47,56,52,41,21,26,37,31,00,21,41,15,16,27,47,56,55,12,
        11,51,15,16,27,47,56,55,44,53,52,41,21,12,13,00,34,44,41,47,
        14,54,57,17,14,44,53,52,41,21,12,56,47,27,16,12,21,41,52,53,
        44,14,16,17,57,56,31,24,15,16,27,47,56,55,44,53,52,41,21,12,
        
        13,24,44,54,24,15,16,27,47,56,52,41,21,12,35,55,00,33,53,11,
        16,27,47,56,51,00,14,54,17,11,41,52,53,44,44,55,56,47,17,00,
        14,44,56,47,27,16,12,21,41,52,52,56,47,17,11,41,52,57,17,11,
        51,00,44,14,57,17,11,00,44,14,54,52,41,21,12,16,27,47,56,00,
        44,64,11,17,00,14,54,00,57,51,17,57,00,37,31,00,11,51,57,52,
        41,21,12,13,17,11,00,14,51,00,14,57,17,11,51,11,17,35,57,51,
        11,17,51,57,21,12,16,27,47,56,52,41,21,11,17,47,56,55,44,14,
        21,12,16,27,47,56,52,41,21,00,42,51,11,17,47,56,55,44,14,00,
        44,51,56,47,27,16,15,53,52,41,21,12,17,57,00,37,31,17,12,21,
        41,52,57,17,31,57,17,11,34,51,57,17,51,00,11,57,17,34,57,00,
        34,31,17,57,56,12,11,51,00,24,44
    };
    double xzero,yzero,h;
    double sina,cosa,a;
    double dxzero,dyzero;
    double xx,yy,xxx,yyy;
    int nloop,ioff,iloop;
    int c,lc,ic,iloc,iend;
    int penup,i,iss,ix,iy;
    
    if(nchar == 0)return 0;
    swhere(&xzero,&yzero);
    if(xp != 999.)xzero=xp;
    if(yp != 999.)yzero=yp;
    h=hp/7.;
    ioff=1;
    nloop=nchar;
    if(nchar <= 0){
        nloop=1;
        if(nchar == -1)smove(xzero,yzero);
        if(nchar != -1)sdraw(xzero,yzero);
        ioff=4;
    }
    a=.017453293*angle;
    sina=sin(a);
    cosa=cos(a);
    dxzero=hp*cosa;
    dyzero=hp*sina;
    for(iloop=0;iloop<nloop;++iloop){
        c=ib[iloop];
        lc=c;
        if(c >= 'a'&& c <= 'z'){
            lc=lc-32;
        }
        ic=lc;
        if(ic < 0 || ic > 90)goto L110;
        iloc=icp[ic];
        if(iloc <= 0)goto L110;
        iend=icp[ic+1]-1;
        penup=TRUE;
        
        for(i=iloc;i<=iend;++i){
            iss=ishape[i-1];
            if(iss == 0){
                penup=TRUE;
                continue;
            }
            ix=(iss/10);
            iy=iss-10*ix;
            
            xx=(ix-ioff)*h;
            yy=(iy-ioff)*h;
            
            xxx=xzero+xx*cosa-yy*sina;
            yyy=yzero+xx*sina+yy*cosa;
            
            if(penup){
                smove(xxx,yyy);
            }else{
                sdraw(xxx,yyy);
            }
            penup=FALSE;
        }
        
    L110:
        if(nchar < 0)continue;
        xzero=xzero+dxzero;
        yzero=yzero+dyzero;
        
    }
    
    return 0;
}

