#include "sdrReceive.h"

void SetLighting(unsigned int mode);

static void myReshape(int w, int h);

static void displayc(void);

static int DrawString(int x, int y, char *out,int which);

int DrawBox(uRect *box,int offset);

static double GridPlotpow10(int n);

static void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small);

static void getMousel(int button, int state, int x, int y);

int testEM(void);

int DrawLine3(int x1, int y1, int x2, int y2);

static void moveMouse(int x, int y);

extern "C" int DrawLine(int x1, int y1, int x2, int y2);

SdrReceive::SdrReceive(struct Scene *scene,SoapySDR::Kwargs deviceArgs): CWindow(scene)
{
    OpenError=TRUE;
    backGroundEvents=0;
    
    zerol((char *)&rxs,(&rxs.end-&rxs.start)+1);
    rx=&rxs;
    rx->f=100e6;
    rx->fc=100e6;
    rx->samplerate=2e6;
    rx->bw=5000;
    rx->viewWindow=2e6;
    
    OpenError=FALSE;
}
int SdrReceive::drawAxis()
{
    double xmins,xmaxs,Large,Small;
    double xmin,xmax,dx;
    double xmins2,xmaxs2,dxs;
    double fc,samplerate;
    char buff[256];
    
    fc=rx->fc;
    samplerate=rx->samplerate;
    xmin=fc-0.5*samplerate;
    xmax=fc+0.5*samplerate;
    xmins=rx->f-0.5*rx->viewWindow;
    if(xmins < xmin)xmins=xmin;
    xmaxs=rx->f+0.5*rx->viewWindow;
    if(xmaxs > xmaxs)xmins=xmax;
    dx=xmax-xmin;
    dxs=xmaxs-xmins;
    xmins2=xmins;
    xmaxs2=xmaxs;
    GridPlotNeat(&xmins2,&xmaxs2,&Large,&Small);
//    fprintf(stderr,"xmin %f xmax %f xmins %f xmaxs %f Large %g Small %g count %g\n",xmin,xmax,xmins,xmaxs,Large,Small,(xmaxs-xmins)/Large);
    //DrawString(20, (int)scene->yResolution-height/2-15,(char *)"10.0", 3);
    for(double f=xmins2;f<=xmaxs2;f += Large){
        if(f < xmins || f > xmaxs)continue;
        sprintf(buff,"%.3f",f/1e6);
        int ii=(f-xmins)*width/dxs;
        //int iio=(f-xmin)*width/dx;
        int i=ii-4.0*strlen(buff);
        if(i < 0)continue;
        DrawLine3(ii, (int)scene->yResolution-height/2-4, ii, height-28);
//        printf("i %d f %f width %d\n",i,f,width);
        DrawString(i, (int)scene->yResolution-height/2-15,(char *)buff, 3);

    }
    
    DrawLine3(70, (int)scene->yResolution-height/2+20, width, (int)scene->yResolution-height/2+20);
    DrawLine3(70, (int)scene->yResolution-height/2+50, width, (int)scene->yResolution-height/2+50);
    DrawLine3(70, (int)scene->yResolution-height/2+80, width, (int)scene->yResolution-height/2+80);
    DrawLine3(70, (int)scene->yResolution-height/2+110, width, (int)scene->yResolution-height/2+110);

    
    return 0;
}
int DrawLine3(int x1, int y1, int x2, int y2)
{
    
    glPushAttrib(GL_ENABLE_BIT);
    
    glLineStipple(1, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glLineWidth((GLfloat)1.0);
    glBegin(GL_LINES);
    
    glVertex2f((x1),(y1));
    glVertex2f((x2),(y2));
    
    glEnd();
    
    glPopAttrib();
    
    return 0;
}


int SdrReceive::OpenWindows(struct Scene *scene)
{
    struct SceneList *list;
    
    //struct Scene *sceneOpen=scene;
    
    if(!scene)return 1;
    
    list=SceneFindScene(scene);
    if(list == NULL)
    {
        WarningPrint("FMRadio::OpenWindows : Error Could Not Find Scene\n");
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
    glutInitWindowSize(960,340);
    window = glutCreateWindow(scene->WindowTitle);
    list->window=window;
    scene->window=window;
    glutReshapeFunc(myReshape);
    glutMouseFunc(getMousel);
    glutDisplayFunc(displayc);
    glutMotionFunc(moveMouse);

    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glClearColor(1.0,1.0,1.0,0.);
    
    SetLighting(3);
    
    glEnable(GL_LIGHTING);
    
    glShadeModel(GL_SMOOTH);
    
    testEM();

   return 0;
}

int SdrReceive::getMouse(int button, int state, int x, int y)
{
    
    uPoint p;
    p.x=x;
    p.y=height-y;
    
   // printf("getMouse button %d state %d x %d y %d\n",button,state,x,y);
    
    if(uPtInRect(&p,&boxFrequency) ){
        printf("in boxFrequency\n");
    }
    
    
    if(uPtInRect(&p,&xAxis)){
        double dx=(rx->samplerate*0.01);
        if(button == 3){
            rx->viewWindow -= dx;
            if(rx->viewWindow < dx)rx->viewWindow=dx;
        printf("in xAxis 3 viewWindow %g\n",rx->viewWindow);
            glutPostRedisplay();
           return 0;
       }else if( button == 4){
           rx->viewWindow += dx;
           if(rx->viewWindow > rx->samplerate)rx->viewWindow=rx->samplerate;
          printf("in xAxis viewWindow %g\n",rx->viewWindow);
           glutPostRedisplay();
           return 0;
        }
    }
    
    if(button == 3){
        double fl,bw;
        bw=rx->bw*0.5;
        //  if(rx->wShift != 0)bw /= 2;
        if(rx->wShift != 0)bw = 500;
        fl=rx->f-bw;
        if(fl < 0)fl=-fl;
        //printf("3 fl %lld rx->f %lld bw %g\n",(long long)fl,(long long)rx->f,bw);
        rx->f=fl;
        if(fabs(fl-rx->fc) > 0.5*rx->samplerate){
            rx->fc=fl;
        }else{
        }
        glutPostRedisplay();
        return 0;
    }else if(button == 4){
        double fl,bw;
        bw=rx->bw*0.5;
        //if(rx->wShift != 0)bw /= 2;
        if(rx->wShift != 0)bw=500;
        fl=rx->f+bw;
        if(fl < 0)fl=-fl;
        rx->f=fl;
        //printf("4 fl %lld rx->f %lld bw %g\n",(long long)fl,(long long)rx->f,bw);
        if(fabs(fl-rx->fc) > 0.5*rx->samplerate){
            rx->fc=fl;
        }else{
        }
        glutPostRedisplay();
        return 0;
    }else if(button != 0){
        return 0;
    }
    
    
    static double fsave;
    if(state == GLUT_DOWN)
    {
        double fclick;
        if(uPtInRect(&p,&xAxis)){
    
            fclick=rx->fc-0.5*rx->samplerate+x*(double)rx->samplerate/(scene->xResolution);
            
            rx->f=fclick;
            
            fdown=fclick;
            
            fcdown=rx->fc;
            
            fsave=rx->fc;
            
            fcount=0;
        }

    }else{
        //if(fsave != rx->fc) setFrequency2(rx);
    }
    glutPostRedisplay();
    return 0;
}
static void moveMouse(int x, int y)
{
    int window=glutGetWindow();
    SdrReceivePtr s = (SdrReceivePtr)FindWindow2(window);
    if(!s)return;
    
    printf("moveMouse x %d y %d\n",x,y);

    uPoint p;
    p.x=x;
    p.y=s->height-y;

    struct Scene *scene;
    
    scene=s->scene;
    
    if(!scene)return;
    
    if(uPtInRect(&p,&s->xAxis)){

        double fmove=s->fcdown-0.5*s->rx->samplerate+x*(double)s->rx->samplerate/(scene->xResolution);
        
        double diff=fmove-s->fdown;
        
        s->rx->fc=s->fcdown-diff;
        
         glutPostRedisplay();
        
        printf("fc %g\n",s->rx->fc);
    }
}
static void displayc(void)
{
    int window=glutGetWindow();
    SdrReceivePtr s = (SdrReceivePtr)FindWindow2(window);
    if(!s)return;
    
    //fprintf(stderr,"glut %d win1 %d win %d\n",glutGetWindow(),images->window1,list->window);
    
   // s->Display(scene);
    
    //glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glClearColor(0.0, 0.0, 1.0, 0.0);
    
    glRasterPos2i(0,0);
    
    glColor3f(1.0,1.0,1.0);
    
    unsigned long f,fc;
    
    f=s->rx->f;
    
    fc=s->rx->fc;
    
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, s->width, 0, s->height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        glRasterPos2i(0,0);        /* Position at base of window */
        
        char value[256];
        
            msprintf(value,sizeof(value),"F: %010ld Hz CF: %010ld Hz Power: %.0f db",f,fc,0);

        uSetRect(&s->boxFrequency,0,s->height,s->width,28);
        
        DrawBox(&s->boxFrequency,(int)0);

        DrawString(20, (int)s->scene->yResolution-20, value, 0);
        
        uSetRect(&s->xAxis,0,s->height/2,s->width,20);
        
        DrawBox(&s->xAxis,(int)0);
        
 //       DrawString(20, (int)s->scene->yResolution-s->height/2-15, value, 3);
        
        uSetRect(&s->yAxis,0,s->height-28,70,s->height/2-27);
        DrawBox(&s->yAxis,(int)0);
        
        s->drawAxis();

    }
    
    
    glutSwapBuffers();
    
}

static int DrawString(int x, int y, char *out,int which)
{
    int h;
    
    if(!out)return 1;
    
    glPushMatrix();
    glRasterPos2i( x, y);
    while((h=*out++))
    {
        
        if(which == 0){
           glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, h);
        }else if(which == 1){
           glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, h);
        }else if(which == 2){
          glutBitmapCharacter(GLUT_BITMAP_9_BY_15, h);
        }else if(which == 3){
          glutBitmapCharacter(GLUT_BITMAP_8_BY_13, h);
        }
        
    }
    
    glPopMatrix();
    
    return 0;
}

static void myReshape(int wscr, int hscr)
{
    double xmin,ymin,xmax,ymax;
    int window=glutGetWindow();
    
    SdrReceivePtr s = (SdrReceivePtr)FindWindow2(window);
    if(!s)return;
    
    s->width=wscr; s->height=hscr;
    glViewport(0,0,(GLsizei)s->width,(GLsizei)s->height);

    xmin=ymin=0.0; xmax=ymax=1.0;
    if(s->width<=s->height){
        ymax=s->height*(GLfloat)s->height/(GLfloat)s->width;
        xmax=s->width;
    }else{
        xmax=s->width*(GLfloat)s->width/(GLfloat)s->height;
        ymax=s->height;
    }
    
    gluOrtho2D(xmin,xmax,ymin,ymax);
    glMatrixMode(GL_MODELVIEW);
    
    //glLoadIdentity();
    
    s->scene->xResolution=s->width;
    
    s->scene->yResolution=s->height;
}
static void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small)
{
    
    double xmn = *xmnc,xmx = *xmxc;
    double delx,step;
    int nn;
    long long stepi,min,max;
    double xmin,xmax;
    
    if(!xmnc || !xmxc || !Large || !Small)return;
    
    delx=xmx-xmn;
    
    step=delx/8;
    
    stepi=(long long)step;
    
    nn=0;
    while(stepi >= 10){
        stepi /= 10;
        ++nn;
    }
    
    while(nn-- > 0){
        stepi *= 10;
    }
    
    step=stepi;
    
    //printf("step %g stepi %lld\n",step,stepi);
    
    min=(xmn-step)/step;
    xmin=min*step;
    
    max=(xmx+step)/step;
    xmax=max*step;
    
    //printf("xmin %g xmax %g\n\n",xmin,xmax);
    
    *Large=step;
    
    *Small=step/5;
    
    *xmnc=xmin;
    
    *xmxc=xmax;

}

static void getMousel(int button, int state, int x, int y)
{
    int window=glutGetWindow();
    SdrReceivePtr s = (SdrReceivePtr)FindWindow2(window);
    if(!s)return;
    s->getMouse(button,state,x,y);
}
int testEM()
{
    double xmnc,xmxc,Large,Small;
    double fc,bw;
    
    fc=10.1e6;
    bw=10e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=1e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.1e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.01e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.001e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    
    return 0;
}

SdrReceive::~SdrReceive()
{
	;
}
