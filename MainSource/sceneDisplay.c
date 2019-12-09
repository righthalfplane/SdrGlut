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

#define DTOR            0.0174532925

GLuint textureId;

int txsize=400;
int tysize=400;

GLuint fBuffer;

GLuint rBuffer;

GLuint vboId = 0;                   // ID of VBO for vertex arrays


int w=800,h=400;

GLuint iBuff[3];


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


int DrawString(int x, int y, char *out)
{
    int h;
    
    if(!out)return 1;
    
    glPushMatrix();
    glRasterPos2i( x, y);
    while((h=*out++))
    {
        /* glutBitmapCharacter(GLUT_BITMAP_9_BY_15, h); */
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, h);
        
    }
    
    glPopMatrix();
    
    return 0;
}

