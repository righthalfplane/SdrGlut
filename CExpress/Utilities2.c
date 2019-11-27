/*
 *  Utilities.c
 *  
 *
 *  Created by Dale Ranta on 2/4/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
#include "firstFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include "FileList.h"
#include "paletteDraw.h"
#include <string.h>
#include <ctype.h>
#include "Utilities2.h"
#include "Utilities.h"

#define FLOAT_NOT_SET ((double)(-1.23e-21))

#define POINT_INCREMENT 10


typedef struct uPatternStruct{
    unsigned char pat[8];
} uPattern;

static uPattern  Patterns[] = {
	{{0xff ,0xff, 0xff, 0xff, 0xff ,0xff, 0xff, 0xff}},				
	{{0xdd ,0xff, 0x77, 0xff, 0xdd ,0xff, 0x77, 0xff}},				
	{{0xdd ,0x77, 0xdd, 0x77, 0xdd ,0x77, 0xdd, 0x77}},				
	{{0xaa ,0x55, 0xaa, 0x55, 0xaa ,0x55, 0xaa, 0x55}},				
	{{0x55 ,0xff, 0x55, 0xff, 0x55 ,0xff, 0x55, 0xff}},				
	{{0xaa ,0xaa, 0xaa, 0xaa, 0xaa ,0xaa, 0xaa, 0xaa}},				   
	{{0xee ,0xdd, 0xbb, 0x77, 0xee ,0xdd, 0xbb, 0x77}},				
	{{0x88 ,0x88, 0x88, 0x88, 0x88 ,0x88, 0x88, 0x88}},				   
	{{0xb1 ,0x30, 0x03, 0x1b, 0xd8 ,0xc0, 0x0c, 0x8d}},				   
	{{0x80 ,0x10, 0x02, 0x20, 0x01 ,0x08, 0x40, 0x04}},				
	{{0xff ,0x88, 0x88, 0x88, 0xff ,0x88, 0x88, 0x88}},				
	{{0xff ,0x80, 0x80, 0x80, 0xff ,0x08, 0x08, 0x08}},				
	{{0x80 ,0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00}},				   
	{{0x80 ,0x40, 0x20, 0x00, 0x02 ,0x04, 0x08, 0x00}},				 
	{{0x82 ,0x44, 0x39, 0x44, 0x82 ,0x01, 0x01, 0x01}},				   
	{{0xf8 ,0x74, 0x22, 0x47, 0x8f ,0x17, 0x22, 0x71}},				
	{{0x55 ,0xa0, 0x40, 0x40, 0x55 ,0x0a, 0x04, 0x04}},				
	{{0x20 ,0x50, 0x88, 0x88, 0x88 ,0x88, 0x05, 0x02}},				
	{{0xbf ,0x00, 0xbf, 0xbf, 0xb0 ,0xb0, 0xb0, 0xb0}},				
	{{0x00 ,0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00}},				
	{{0x80 ,0x00, 0x08, 0x00, 0x80 ,0x00, 0x08, 0x00}},				   
	{{0x88 ,0x00, 0x22, 0x00, 0x88 ,0x00, 0x22, 0x00}},				
	{{0x88 ,0x22, 0x88, 0x22, 0x88 ,0x22, 0x88, 0x22}},				
	{{0xaa ,0x00, 0xaa, 0x00, 0xaa ,0x00, 0xaa, 0x00}},				
	{{0xff ,0x00, 0xff, 0x00, 0xff ,0x00, 0xff, 0x00}},				   
	{{0x11 ,0x22, 0x44, 0x88, 0x11 ,0x22, 0x44, 0x88}},				
	{{0xff ,0x00, 0x00, 0x00, 0xff ,0x00, 0x00, 0x00}},				   
	{{0x01 ,0x02, 0x04, 0x08, 0x10 ,0x20, 0x40, 0x80}},				
	{{0xaa ,0x00, 0x80, 0x00, 0x88 ,0x00, 0x80, 0x00}},				
	{{0xff ,0x80, 0x80, 0x80, 0x80 ,0x80, 0x80, 0x80}},				   
	{{0x08 ,0x1c, 0x22, 0xc1, 0x80 ,0x01, 0x02, 0x04}},				   
	{{0x88 ,0x14, 0x22, 0x41, 0x88 ,0x00, 0xaa, 0x00}},				
	{{0x40 ,0xa0, 0x00, 0x00, 0x04 ,0x0a, 0x00, 0x00}},				
	{{0x03 ,0x84, 0x48, 0x30, 0x0c ,0x02, 0x01, 0x01}},				
	{{0x80 ,0x80, 0x41, 0x3e, 0x08 ,0x08, 0x14, 0xe3}},				   
	{{0x10 ,0x20, 0x54, 0xaa, 0xff ,0x02, 0x04, 0x08}},				
	{{0x77 ,0x89, 0x8f, 0x8f, 0x77 ,0x98, 0xf8, 0xf8}},				
	{{0x00 ,0x08, 0x14, 0x2a, 0x55 ,0x2a, 0x14, 0x08}},				
};



int GetTime(long *Seconds,long *milliseconds);

int domkdir(char *nb);


struct dataStructHold{
    long count;
    long countMax;
    char **message;
};

static struct dataStructHold BatchHold;

static int checkBatchHold(struct dataStructHold *plane);

int kpoint(int x,int y,int nc,struct screenData *s);

int kpointPattern(int x,int y,struct screenData *s);

int findit(long count,double *level,double value,long *ic);

int findit(long count,double *level,double value,long *ic)
{
        long ib,it,ns;

        if(!level || !ic)return 1;

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
            if(value >= level[ns]){
                ib=ns;
            }else{
                it=ns;
            }
        }
        *ic=ib;
        return 0;

}

int WarningBatchHoldDump(void)
{
	struct dataStructHold *b;
	long n;

	b=&BatchHold;
	
	if(!b->message || (b->count <= 0))return 1;
	
	
	for(n=0;n<b->count;++n){
		if(b->message[n]){
		    WarningBatch(b->message[n]);
		    cFree((char *)b->message[n]);
		    b->message[n]=NULL;
		}
	}
	
	cFree((char *)b->message);
	
	b->message=NULL;
	b->countMax=0;
	b->count=0;
		
	return 0;
	
}
int WarningBatchHold(char *buff)
{
	struct dataStructHold *b;
	
	if(!buff)return 1;
	
	b=&BatchHold;
	
	if(checkBatchHold(b))return 1;
	
	b->message[b->count]=strsave(buff,1974);
	
	if(!b->message[b->count])return 1;
	
	++b->count;
	
	return 0;
}
static int checkBatchHold(struct dataStructHold *plane)
{
    long countMax;
	char **message;
	if(!plane)return 1;
	
	if(plane->count+1 < plane->countMax)return 0;
	
	countMax=plane->countMax+POINT_INCREMENT;
	
	message=NULL;
	
	if(plane->message){
	    message=(char **)cRealloc((char *)plane->message,countMax*sizeof(char **),7761);
	    if(!message){
	        goto ErrorOut;
	    }
	    zerol((char *)&message[plane->countMax],POINT_INCREMENT*sizeof(char **));
	}else{
	    message=(char **)cMalloc(countMax*sizeof(char **),7452);
	    if(!message){
	        goto ErrorOut;
	    }
	    zerol((char *)message,countMax*sizeof(char **));
	}
	
	plane->countMax=countMax;
	plane->message=message;
	
	return 0;
ErrorOut:
    if(message)cFree((char *)message);
	return 1;
	
}	

char *DefaultPathString(void)
{
    char working[2048];
   
    if(!GetWorking(working,2048))return NULL;

        return strsave(working,1982);
}
int mstrcmp(char *out,char *in)
{
    char out2[256],in2[256];
    int c,n;
    
	if(!out || !in)return 0;
	
	mstrncpy(out2,out,255);	
	for(n=0;n<255;++n){
	    c=out2[n];
	    if(!c)break;
	    out2[n]=toupper(c);
	}
	
	mstrncpy(in2,in,255);
	for(n=0;n<255;++n){
	    c=in2[n];
	    if(!c)break;
	    in2[n]=toupper(c);
	}
	
	return strcmp(out2,in2);
}

int strncatToPath(char *pathname,char *name,int length)
{
        int nn;

        if(!pathname || !name)return 1;

        nn=(int)strlen(pathname);
        if(nn > 0){
            if(pathname[nn-1] != *FILE_NAME_SEPERATOR){
                pathname[nn] = *FILE_NAME_SEPERATOR;
                if(nn+1 < length)pathname[nn+1] = 0;
            }
        }
        mstrncat(pathname,name,length);
        return 0;
}

int isDirectory(char *name)
{
        struct stat buf;

        if(!name)return 0;

    if(stat(name,&buf)){
        return 0;
    }

    if(buf.st_mode & S_IFDIR){
        return 1;
    }

        return 0;
}
int uMemcpy(void *out1,void *in1,long length)
{
        unsigned char *out,*in;

        if(!out1 || !in1 || (length <= 0))return 1;

        out=(unsigned char *)out1;
        in=(unsigned char *)in1;

        while(length--)*out++ = *in++;


        return 0;

}
int FloatToImage(double *d,long length,struct paletteDraw *pd,unsigned char *bp)
{
        double dmin;
        double dmax;
        double mmax;
        double dx;
        double a,b;
        double r;
        long n;
        int dolog;
        int v;

        if(!d || !pd)return 1;


        dmin=pd->sPmin;
        dmax=pd->sPmax;


        dolog = 0;

        if(pd->sType == 0){

            dmin=pd->dmin;
			dmax=pd->dmax;
	
			dx=dmax-dmin;
	
			mmax=max(fabs(dmin),fabs(dmax));
	
			if(dx <= 0.0 || dx < 1e-5*mmax){
				dmax=dmin+1.;
				--dmin;
			}
        }else if(pd->sType == 1){
            dolog = 1;
        }

        if(!bp){
            bp=(unsigned char *)d;
        }

        if(dolog){
            if(dmin <= 0.0)return 1;

            a=(254.0-2.0)/(log10(dmax)-log10(dmin));
            b=2.0-a*log10(dmin);

                for(n=0;n<length;++n){
                    r=d[n];
                    if(r == FLOAT_NOT_SET){
                        *bp++ = 0;
                        continue;
                    }else if(r < dmin){
                       r = dmin;
                    }else if(r > dmax){
                       r = dmax;
                    }

                    r=log10(r);
                    
                    v=(int)(a*r+b);
                    
                    if(v < 2){
                        v=2;
                    }else if(v > 254){
                        v=254;
                    }
                    
                    *bp++ = (unsigned char)(v);
                }

        }else{
            a=(254.0-2.0)/(dmax-dmin);
            b=2.0-a*dmin;
                for(n=0;n<length;++n){
                    r=d[n];
                    if(r == FLOAT_NOT_SET){
                        *bp++ = 0;
                        continue;
                    }

                    v=(int)(a*r+b);

                    if(v < 2){
                        v=2;
                    }else if(v > 254){
                        v=254;
                    }

                    *bp++ = (unsigned char)(v);
                }
        }

        pd->dmin=dmin;
        pd->dmax=dmax;

        return 0;
}
int FloatToColor(double *d,struct paletteDraw *pd,unsigned char *bp)
{
	double dmin;
	double dmax;
	double mmax;
	double dx;
	double a,b;
	double r;
	int dolog;
	int v;
	
	if(!d || !pd || !bp)return 1;
	
	
	dmin=pd->sPmin;
	dmax=pd->sPmax;
	

	dolog = 0;
	    
	if(pd->sType == 0){
	    		    
   		dmin=pd->dmin;
    	dmax=pd->dmax;
    	
    	dx=dmax-dmin;
    	
    	mmax=max(fabs(dmin),fabs(dmax));
	    	    
	    if(dx <= 0.0 || dx < 1e-5*mmax){
	        dmax=dmin+1.;
	        --dmin;
	    }
	}else if(pd->sType == 1){
	    dolog = 1;
	}
			
	if(dolog){
	    if(dmin <= 0.0)return 1;
	    
		a=(254.0-2.0)/(log10(dmax)-log10(dmin));
		b=2.0-a*log10(dmin);

		r = *d;
		if(r == FLOAT_NOT_SET){
		    *bp = 0;
		}else if(r < dmin){
		   r = dmin;
		}else if(r > dmax){
		   r = dmax;
		}

		r=log10(r);
		*bp = (unsigned char)(a*r+b);
	
	}else{
	    a=(254.0-2.0)/(dmax-dmin);
	    b=2.0-a*dmin;
		    r = *d;
		    if(r == FLOAT_NOT_SET){
		        *bp = 0;
		    }
		    		    
		    v=(int)(a*r+b);
		    		    
		    if(v < 2){
		        v=2;
		    }else if(v > 254){
		        v=254;
		    }
		      
		    *bp = (unsigned char)(v);
	}
	
	
	return 0;
}
int setFloat(double *d,long length)
{
	if(!d)return 1;
	if(length <= 0)return 1;
	while(length-- > 0)*d++ = FLOAT_NOT_SET;
	return 0;
}
static int xBuff,yBuff;   
int WhereB(int *x,int *y)
{
	*x=xBuff;
	*y=yBuff;
	return 0;
}
int MoveB(int x,int y)
{   
        xBuff=x;
        yBuff=y;
        return 0;
}   
int LineB2(int x1,int y1,int x2,int y2,int nc,struct screenData *s)
{
        xBuff=x1;
        yBuff=y1;
		return LineB(x2,y2,nc,s);
}
int LineB(int x,int y,int nc,struct screenData *s)
{
        int ix0,iy0,ix1,iy1;
        int idx,irl,idy,iup,idxy;
        int ix,iy,ixs,iys,nxy;

        if(!s)return 1;
        ix0=xBuff;
        iy0=yBuff;
        ix1=x;
        iy1=y;

        if(ix1 < ix0){
            idx = ix0-ix1;
            irl = -1;
        }else{
            idx = ix1-ix0;
            irl = 1;
        }

        if(iy1 < iy0){
            idy = iy0-iy1;
            iup = -1;
        }else{
            idy = iy1-iy0;
            iup = 1;
        }
        idxy=idx+idy;
        if(idxy <= 0){
            kpoint(ix0,iy0,nc,s);
            goto Out;
        }
        ix=ix0;
        iy=iy0;
        ixs=0;
        iys=0;
        for(nxy=1;nxy<=idxy;++nxy){
            kpoint(ix,iy,nc,s);
            ixs=ixs+idx;
            if(ixs >= idxy){
                ixs=ixs-idxy;
                ix=ix+irl;
            }
            iys=iys+idy;
            if(iys < idxy)continue;
            iys=iys-idxy;
            iy=iy+iup;
        }
Out:
        kpoint(ix1,iy1,nc,s);
        xBuff=x;
        yBuff=y;
        return 0;
}
int kpoint(int x,int y,int nc,struct screenData *s)
{   
	long n,yr;

	if(!s || !s->buffer)return 1;

	if(s->xLineWidth > 1 || s->xLineWidth > 1){
	
		return kpointPattern(x,y,s);

    }else{
        if(x < 0 || x >= s->viewRect.xsize)return 0;
        if(y < 0 || y >= s->viewRect.ysize)return 0;
		if(s->noFlip){
			yr=y;
		}else{
			yr=s->viewRect.ysize-1-y;
		}
        if(s->ScreenBits == RASTER_8){
            s->buffer[x+yr*s->viewRect.xsize]=nc;
        }else if(s->ScreenBits == RASTER_24){
            unsigned char *pal;
            pal=s->palette;
            n=x*3+yr*s->viewRect.xsize*3;
            s->buffer[n]=pal[nc*3];
            s->buffer[1+n]=pal[nc*3+1];
            s->buffer[2+n]=pal[nc*3+2];
        }
    }
    return 0;
}
uPattern *PatternPattern(int n)
{
	
	if(n < 0 || n > (int)(sizeof(Patterns)/sizeof(uPattern))){
	    n=1;
	}
	return &Patterns[n];
}                       
int kpointPattern(int x,int y,struct screenData *s)
{   
	static uPattern Pat={{0x11,0x44,0x11,0x44,0x11,0x44,0x11,0x44}};
	uPattern *pat;
	unsigned char p;
	int n,yr;
	int xn,yn;
	int i,j;
	int xs,ys;
	int nc;

	if(!s || !s->buffer)return 1;

		pat=PatternPattern(s->LinePattern);
		if(!pat){
		   pat=&Pat;
		}
        
    	xs=x;
    	ys=y;
        xn=s->xLineWidth;
        if(xn <= 0)xn=1;
        yn=s->yLineWidth;
        if(yn <= 0)yn=1;
        for(j=0;j<yn;++j){
            y=ys+j;
    		p=pat->pat[y % 8];
            for(i=0;i<xn;++i){
            	x=xs+i;
            	if((p << (x % 8)) & 128){
            	   nc=s->nLineFore;
            	}else{
            	   nc=s->nLineBack;
            	}
		        if(x < 0 || x >= s->viewRect.xsize)continue;
		        if(y < 0 || y >= s->viewRect.ysize)continue;
		        yr=s->viewRect.ysize-1-y;
		        if(s->ScreenBits == RASTER_8){
		            s->buffer[x+yr*s->viewRect.xsize]=nc;
		        }else if(s->ScreenBits == RASTER_24){
		            unsigned char *pal;
		            pal=s->palette;
		            n=x*3+yr*s->viewRect.xsize*3;
		            s->buffer[n]=pal[nc*3];
		            s->buffer[1+n]=pal[nc*3+1];
		            s->buffer[2+n]=pal[nc*3+2];
		        }	            
            	            	            
            }
        }
    return 0;
}

int DrawString2(int x,int y,char *ib,double xsize,struct screenData *s)
{
	return symsft((double)x/72.0,(double)y/72.0,xsize,ib,0.0,(int)strlen(ib),s);
}


static double xlast,ylast;
int swhere(double *x,double *y)
{
	*x=xlast;
	*y=ylast;
	return 0;
}
int smove(double x,double y)
{
	xlast=x;
	ylast=y;
	
	MoveB((int)( x*72+0.5),(int)( y*72+0.5));
	return 0;
}
int sdraw(double x,double y,struct screenData *s)
{

	LineB((int) (x*72+0.5),(int)( y*72+0.5),255,s);
		
	xlast=x;
	ylast=y;
	
	return 0;
}
int  symsft(double xp,double yp,double hp,char *ib,double angle,int nchar,struct screenData *s)
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
		if(nchar != -1)sdraw(xzero,yzero,s);
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
			   sdraw(xxx,yyy,s);
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


