#include "graphics.h"
#include <string.h>
#include <math.h>

#if __has_include(<liquid/liquid.h>)
#include <liquid/liquid.h>
#else
#include <liquid.h>
#endif


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
    glEnable(GL_COLOR_MATERIAL);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    float ratio_w_h = (float)(bottomrigth_x-topleft_x)/(float)(bottomrigth_y-topleft_y);
    gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
}
 
/** Inits the OpenGL viewport for drawing in 2D. */
void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


int colorit(long count,double *level,double value,int *ic)
{
	long ib,it,ns;
	
	if(value <= *level){
		*ic=0;
		return 0;
	}
	it=count-1;
	if(value >= level[it]){
		*ic=it;
		return 0;
	}
	ib=0;
	while(it > ib+1){
		ns=(it+ib)/2;
		if(value > level[ns]){
			ib=ns;
		}else{
			it=ns;
		}
	}
	*ic=ib;
	return 0;
}


int DrawBox(uRect *box,int offset)
{
    if(box->xsize <= 0)return 0;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND); //Enable blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function.
    
    
    glColor4f(0.0, 1.0, 0.0, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(box->x, box->y);
    glVertex2f(box->x, box->y+box->ysize);
    glVertex2f(box->x+box->xsize,box->y+box->ysize);
    glVertex2f(box->x+box->xsize, box->y);
    glEnd();
    
    glDisable(GL_BLEND);
    
    
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

static int DrawSym(double xp,double yp,double hp,char *ib,double angle,int nchar);

static int smove(double x,double y);

static double xlast,ylast;


int DrawLine2(float x1, float y1, float x2, float y2)
{
    glBegin(GL_LINES);
    
    glVertex2f((x1),(y1));
    glVertex2f((x2),(y2));
    
    glEnd();
    
    return 0;
}
int DrawString(int x, int y, char *out)
{
   // int h;
    
    if(!out)return 1;
    
    glPushMatrix();
    glRasterPos2i( x, y);
        
    DrawSym((double)x,(double)y,8.0,out,0.0,(int)strlen(out));
    
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
            iy=10-iy;
            
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
static double GridPlotpow10(int n)
{
	double a;
 
	a=1.;
 
	if(n < 0){
	   n = -n;
	   while(n--)a/=10.;
	} else {
	    while(n--)a*=10.;
	}
	return a;
}

void GridPlotNeat2(double *xmnc,double *xmxc,double *Large,double *Small)
{

    double xmn = *xmnc,xmx = *xmxc;
    static double small2=1.e-6,dnum[4]={1.,2.,5.,10.};
    double delx,temp,t,fac,td,tmn,znd,zndt;
    int i,j,jj,itm;
    
    if(!xmnc || !xmxc || !Large || !Small)return;
    
    jj=0;
	delx=xmx-xmn;
	temp=delx/6.;
	t=0.;
	if(temp != 0.)t=log10(fabs(temp)+small2*fabs(temp));
	i=(int)t;
	if(t < 0.)i=i-1;
	fac=GridPlotpow10(i);
	temp=temp/fac;
	for(j=0;j<4;++j){
	    jj=j;
	    if(dnum[j] >= temp) break;
    }
	td=dnum[jj]*fac;
	tmn=xmn/td;
	if(fabs(tmn) > 3e10){
blastOut:
	    *xmxc = *xmnc+1.0;
	    *xmnc = *xmnc-1.0;
	    temp=(*xmxc - *xmnc)/4.;
	    *Large = temp;
	    *Small = temp*.5;
	    return;
	}
	if(tmn < 0.)tmn=tmn-small2*tmn;
	itm=(int)tmn;
	if(tmn < 0.)itm=itm-1;
	tmn=itm;
	tmn=tmn*td;
	znd=2.;
l50:	znd=znd+1.;
	if(znd > 8.){
	    goto blastOut;
	}
	zndt=znd*td+tmn;
	if(zndt >= xmx) goto l60;
	goto l50;
l60:
	*xmnc=tmn;
	*xmxc=zndt;
	delx=xmx-xmn;
	*Large=td;
	if(jj == 1){
	    *Small = td/4;
	}else{
	    *Small = td/5;
	}
}
void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small)
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
    
    //fprintf(stderr,"step %g stepi %lld\n",step,stepi);
    
    min=(xmn-step)/step;
    xmin=min*step;
    
    max=(xmx+step)/step;
    xmax=max*step;
    
    //fprintf(stderr,"xmin %g xmax %g\n\n",xmin,xmax);
    
    *Large=step;
    
    *Small=step/5;
    
    *xmnc=xmin;
    
    *xmxc=xmax;

}

int doWindow(float *x,long length,int type)
{
    static float *w=NULL;
    static long lengthSave = -1;
    if(lengthSave < length){
        if(w)delete w;
        lengthSave=length;
        w=new float[lengthSave];
//      winout("lengthSave %ld\n",lengthSave);
   	}
   
    int i;
    
    if(!x)return 1;
    
    switch(type){
            
        case FILTER_RECTANGULAR:

            for(i=0; i<length; i++)
                w[i] = 1.0;
            
            break;
            

            
        case FILTER_HANN:
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hann(i, (int)length);
#else
                w[i]=hann(i, (int)length);
#endif
            }
            break;

            
            
        case FILTER_HAMMING:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hamming(i, (int)length);
#else
                w[i]=hamming(i, (int)length);
#endif
            }
            break;
            
        case FILTER_FLATTOP:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_flattop(i, (int)length);
#else
                w[i]=flattop(i, (int)length);
#endif
            }
            break;
            
            
        case FILTER_BLACKMANHARRIS:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris(i, (int)length);
#else
                w[i]=blackmanharris(i, (int)length);
#endif
            }
            break;
            
        case FILTER_BLACKMANHARRIS7:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris7(i, (int)length);
#else
                w[i]=blackmanharris7(i, (int)length);
#endif
            }
            break;
    }
    
    for(i=0; i<length; i++){
        double amp;
        amp=w[i];
        x[2*i]=amp*x[2*i];
        x[2*i+1]=amp*x[2*i+1];
    }
    
    return 0;
    
}

 

int testEM()
{
    double xmnc,xmxc,Large,Small;
    double fc,bw;
    
    fc=10.1e6;
    bw=10e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    printf("xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    printf("xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=1e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    printf("xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    printf("xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.1e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    printf("xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    printf("xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.01e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    printf("xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    printf("xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.001e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    printf("xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    printf("xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    
    return 0;
}


