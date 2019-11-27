/*
 *  GridPlot.c
 *  
 *
 *  Created by Dale Ranta on 10/18/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
#include "firstFile.h"
#include <stdio.h>
#include <string.h>
#include "Scene.h"
#include "GridPlot.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#define max(a,b) 	((a)>(b)?(a):(b))
#define min(a,b) 	((a)<(b)?(a):(b))
#define sign(a,b) 	((b)>=(0)?(fabs(a)):(-fabs(a)))


static struct DObject *GridPlotCreate(char *name,rRect *box,struct uAttributes *Attributes);
int uGridPlotInit(uGridPlotPtr b,struct uAttributes *Attributes);
int GridPlotSet(uGridPlotPtr Object);
void ObjectKill(struct DObject *Object);


static void GridPlotAttribute(DObjPtr o,struct uAttributes *Attributes,int Flag);
int uPutAttributes(struct uAttributes *Aout,struct uAttributes *Ain,int Flag);

static void GridPlotKill(struct DObject *Object);

void ObjectKill(struct DObject *Object);
struct DObject *ObjectCreate(char *name,int type,rRect *box,long size);
int ObjectSet(DObjPtr Object);
/* static int GridPlotSetFrame(struct DObject *Object,long CurrentFrame); */

static int GridPlotDraw(struct DObject *Object,DOListPtr d);

static int GridPlotRange(struct uGridPlot *Plot);

int GridPlotScale(struct uGridPlot *b);

static double GridPlotpow10(int n);
static void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small);

int GridPlotPlotGrid(struct uGridPlot *b,DOListPtr d);

static void GridPlotxGridDraw(struct uGridPlot *b,DOListPtr d,double y,double xMajorIn,double xMajorOut,double xMinorIn,double xMinorOut,double mult,rRect *r);
static void GridPlotyGridDraw(struct uGridPlot *b,DOListPtr d,double x,double yMajorIn,double yMajorOut,double yMinorIn,double yMinorOut,double mult,rRect *r);
static int GridPlotLineGrid(struct uGridPlot *b,DOListPtr d);
int CPlotPlotNumbers(struct uGridPlot *b,DOListPtr d);
static int CPlotPlotLegends(struct uGridPlot *b,DOListPtr d);
static int GridPlotSetSelection(struct uGridPlot *b,DOListPtr d);
static int CPlotyLog(struct uGridPlot *b,rRect *r,DOListPtr d);
static int CPlotyLin(struct uGridPlot *b,rRect *r,DOListPtr d);
static int CPlotxLog(struct uGridPlot *b,rRect *r,DOListPtr d);
static int CPlotxLin(struct uGridPlot *b,rRect *r,DOListPtr d);
int uGetFontInfo(int myIcon,uFontInfo *t);
int LocalToPixel(double x,DOListPtr d);
double PixelToLocal(long x,DOListPtr d);
int uCStringWidth(char *name,int myIcon);
int rDrawString(char *name,DOListPtr d);
static int CPlotrPoint(struct uGridPlot *b,double *x,double *y,double *xr,double *yr);
static int PlotrPoint(struct uGridPlot *b,double *x,double *y,double *ix,double *iy);
static void CPlotDashedLine(struct uGridPlot *b,double *xx,double *yy,int nn,char *pat,DOListPtr d,struct uLineData *Line);
static int clip2(struct uGridPlot *b,double *x1i,double *y1i,double *x2i,double *y2i,int *i1i);
int	rMoveTo(double x,double y,DOListPtr d);
int rLineTo(double x,double y,DOListPtr d);
int rLinePat(uRect *r,char *pat,DOListPtr d);
int rDrawImageRect(iRect *r,DOListPtr d);
int riLineTo(int ix,int iy,DOListPtr d);
int	riMoveTo(int ix,int iy,DOListPtr d);
static void LineDataKill(struct DObject *Object);
static DObjPtr LineDataCreate(char *name,struct linedata *li,struct uAttributes *Attributes);
int GridPlotLineSet(uLineDataPtr Line);

int DrawLine(int x1, int y1, int x2, int y2);

int DrawString(int x, int y, char *out);

static int xOld,yOld;

DObjPtr DOListFindName(DOListPtr l,char *name,int type);
int DOListADD(DOListPtr l,struct DObject *Object);
int DOErase(DOListPtr l);

uLineDataPtr DOLineDataCreate(struct linedata *li,struct uAttributes *Attributes)
{
	static long ObjectCount;
	uLineDataPtr Line;
	char name[256];
	int ret;
	
	if(!Attributes)return NULL;
	
	ret=1;

	msprintf(name,sizeof(name),"Line%03ld%c",ObjectCount++,0);
		
	Line=(uLineDataPtr)LineDataCreate(name,li,Attributes);
	if(!Line)goto ErrorOut;
	
	ret=0;
ErrorOut:	    
	if(ret){
	    if(Line){
			LineDataKill((DObjPtr)Line);
	        Line=NULL;
	    }
	}
	
	return Line;
}

static DObjPtr LineDataCreate(char *name,struct linedata *li,struct uAttributes *Attributes)
{
	struct uLineData *Line;
	DObjPtr Object;
	rRect box;
	
	
	if(!name || !Attributes)return NULL;
	
	Object=NULL;
/*
	if(li && li->sourceName[0]){
		if(!FindWindowByName(li->sourceName)){
		    if(!FileManagerGet(li->sourceName)){
				msprintf(WarningBuff,sizeof(WarningBuff),"LineDataCreate data source '%s' not found\n",li->sourceName);
				WarningPrint(WarningBuff);		
		    }
		}
	}
*/
	
	zerol((char *)&box,sizeof(box));	
	
    Object=ObjectCreate(name,LineDataType,&box,sizeof(struct uLineData));
    if(!Object)goto ErrorOut;
    
    Line=(struct uLineData *)Object;
    
    Line->Attributes= *Attributes;    
/*
	if(li && li->sourceName[0]){
	    if(li->pioName[0]){
	        if(li->pioIndex > 0){
		        msprintf(Line->save,sizeof(Line->save),"%s(%d)",li->pioName,(int)li->pioIndex);
	        }else{
		        msprintf(Line->save,sizeof(Line->save),"%s",li->pioName);
		    }
		}else{
		    msprintf(Line->save,sizeof(Line->save),"Range (%g,%g)",li->rymin,li->rymax);
		}
		Line->li= *li;
    }
*/

	if(li)Line->li= *li;
    Line->xShift=0;
    Line->yShift=0;
    Line->xScale=1.0;
    Line->yScale=1.0;
       
    GridPlotLineSet(Line);
        
ErrorOut:	
	return Object;
}
int GridPlotLineSet(uLineDataPtr Line)
{
	if(!Line)return 1;

    Line->Kill=LineDataKill;
/*
    Line->GetFrameCount=LineDataFrameCount;
    Line->SetFrame=LineDataSetFrame;    
    Line->ReadWrite=uLineWrite;    
*/
	return 0;
}


static void LineDataKill(struct DObject *Object)
{
	struct uLineData *Line=(struct uLineData *)Object;
	
	if(!Line)return;
	
		
	if(Line->xData)cFree((char *)Line->xData);
	Line->xData=NULL;

	if(Line->yData)cFree((char *)Line->yData);
	Line->yData=NULL;

	Line->dCount=0;
		
	ObjectKill(Object);
	
}
uGridPlotPtr DOGridPlotCreate(DOListPtr l)
{
	uGridPlotPtr Plot;
	static rRect box={1.75,1.30,5.75,4.30};
	char name[256];
	int ret;
		
	if(!l)return NULL;
	
	ret=1;
	
	msprintf(name,sizeof(name),"Grid%03ld%c",l->ObjectIDCount++,0);
	
	Plot=(uGridPlotPtr)GridPlotCreate(name,&box,&l->Attributes);
	if(!Plot)goto ErrorOut;
	
	if(DOListADD(l,(DObjPtr)Plot))goto ErrorOut;

	ret=0;
ErrorOut:	    
	if(ret){
	    if(Plot){
			GridPlotKill((DObjPtr)Plot);
	        Plot=NULL;
	    }
	}
	return Plot;
}
int DODraw(DOListPtr l,long CurrentFrame)
{
	int ret;
	long n;
	
	if(!l)return 1;
	/* if(!l->data)return 1; */
	
	ret=1;
	
	DOErase(l);
	

	l->CurrentFrame=CurrentFrame;
	
	for(n=0;n<l->oCount;++n){
		DObjPtr Current;
	    if((Current = l->oCurrent[n]) == NULL)continue;
        if(Current->SetFrame){
            if((*Current->SetFrame)(Current,CurrentFrame))continue;
        }
	}

	for(n=0;n<l->oCount;++n){
		DObjPtr Current;
	    if((Current = l->oCurrent[n]) == NULL)continue;
        if(Current->Draw){
            if((*Current->Draw)(Current,l))goto ErrorOut;
        }
	}
	
	
	ret = 0;
	
ErrorOut:

	return ret;
}
int DOErase(DOListPtr l)
{
	long n;
	
	if(!l)return 1;
	if(!l->data)return 1;
	
	if(l->raster == RASTER_24){
	    for(n=0;n<l->xsize*l->ysize;++n){
	       l->data[3*n]=l->red;
	       l->data[3*n+1]=l->green;
	       l->data[3*n+2]=l->blue;
	    }
	}else{
		zerol((char *)l->data,l->length);
	}
	
	return 0;
}
int DOListADD(DOListPtr l,struct DObject *Object)
{
	long oMaximun;
	int ret;
	
	if(!l || !Object)return 1;
	
	ret = 1;
	if(DOListFindName(l,Object->name,AnyType)){
		WarningPrint("DOListADD create name  '%s' already used\n",Object->name);
	}

	oMaximun=l->oMaximun;
	if(ResizeArray(&l->oCurrent,sizeof(DObjPtr),l->oCount,&l->oMaximun,2006))goto ErrorOut;
	if(ResizeArray(&l->oOld,sizeof(DObjPtr),l->oCount,&oMaximun,2007))goto ErrorOut;
	l->oCurrent[l->oCount]=Object;
	Object->l=l;
	l->oOld[l->oCount]=NULL;
	++l->oCount;
		
	ret = 0;
ErrorOut:
	return ret;
}
DObjPtr DOListFindName(DOListPtr l,char *name,int type)
{
	DObjPtr Found;
	long n;
    
	if(!l || !name)return NULL;
	
	Found=NULL;
		
	for(n=0;n<l->oCount;++n){
		DObjPtr Current;
	    if((Current = l->oCurrent[n]) == NULL)continue;
	    if(!strcmp(Current->name,name)){
	        Found=Current;
	        break;
	    }
	}
	
	if(type == AnyType)return Found;	
	
	if(Found && (Found->type == type)){
	    return Found;
	}
	
	return NULL;
}
int DOListDelete(DOListPtr l)
{
	DObjPtr Current;
	long n;
	
	if(!l)return 1;
	
	if(l->oCurrent){
	    for(n=0;n<l->oCount;++n){
	        if((Current = l->oCurrent[n]) == NULL)continue;
	        if(Current->Kill){
	            (*Current->Kill)(Current);
	        }
	    }
	    cFree((char *)l->oCurrent);
	    l->oCurrent=NULL;
	}
	if(l->oOld){
	    for(n=0;n<l->oCount;++n){
	        if((Current = l->oOld[n]) == NULL)continue;
	        if(Current->Kill){
	            (*Current->Kill)(Current);
	        }
	    }
	    cFree((char *)l->oOld);
	    l->oOld=NULL;
	}

	if(l->data)cFree((char *)l->data);
	l->data=NULL;
	
	if(l->Boxes)cFree((char *)l->Boxes);
	l->Boxes=NULL;
	
	if(l->Stretch)cFree((char *)l->Stretch);
	l->Stretch=NULL;
	
	if(l->itemList)cFree((char *)l->itemList);
	l->itemList=NULL;
	
	if(l->lAttributes)cFree((char *)l->lAttributes);
	l->lAttributes=NULL;
	
	
	zerol((char *)l,sizeof(*l));
	
	return 0;
	
}

static int GridPlotDraw(struct DObject *Object,DOListPtr d)
{
	struct uGridPlot *Plot;
	
	if(!Object || !d)return 1;

	Plot=(struct uGridPlot *)Object;

	if(GridPlotRange(Plot))return 1;
	
	if(GridPlotScale(Plot))return 1;
		
	if(GridPlotPlotGrid(Plot,d))return 1;

	if(GridPlotLineGrid(Plot,d))return 1;
	
	if(CPlotPlotNumbers(Plot,d))return 1;
	
	if(CPlotPlotLegends(Plot,d))return 1;
	
	if(GridPlotSetSelection(Plot,d))return 1;
	
	return 0;
	
}
int rDrawImageRect(iRect *r,DOListPtr d)
{
	//uRect SRect;
	int n;
	
	if(!r || !d)return 1;
	
	//SRect=r->box;
	
	//SRect.x=0;
	//SRect.y=0;
	
	for(n=0;n<r->box.xsize*r->box.ysize;++n){
	    if(r->data[n] != 0 && d->nLineFore != 0)r->data[n]=d->nLineFore;
	}
	
	/*
	
	WriteRect8ToRect24(r->data,&SRect,r->box.xsize,r->box.ysize,
                       d->data,&r->box,d->xsize,d->ysize,
                       (unsigned char *)d->palette);	  
	*/
	
	return 0;
}
int	riMoveTo(int ix,int iy,DOListPtr d)
{
	
	if(!d)return 1;
	
	xOld=ix;
	yOld=iy;

/*
	if(d->PrintFlag){
	   uMoveTo(ix+d->PrintRect.x,iy+d->PrintRect.y);
	}else{
	   uMoveU(ix,iy,&d->s);
	}
*/
	return 0;
}
int riLineTo(int ix,int iy,DOListPtr d)
{
	
	if(!d)return 1;
	
	DrawLine(xOld, yOld, ix, iy);

/*
	if(d->PrintFlag){
		uLineTo(ix+d->PrintRect.x,iy+d->PrintRect.y,d->myIcon);
	}else{
		uLineU(ix,iy,d->nLineFore,&d->s);
	}
*/
	return 0;
}

int rLinePat(uRect *r,char *pat,DOListPtr d)
{
	int rast[16],ion[16];
	int ix1,ix2,iy;
	int k,i,nCount,np,nLoc,nPart,nFlag;
	
	if(!r || !pat || !d)return 1;
	
	nPart=0;
	for(i=0;i<16;++i){
	  if(pat[i] == '0' || pat[i] == 0)break;	
	  ++nPart;
	  np=pat[i]-'0';
	  rast[i]=np;
	  ion[i]=FALSE;
	  if(np & 1)ion[i]=TRUE;
	}
	
	if(nPart <=0 )return 1;
	
	iy=r->y+(r->ysize)/2;
	ix1=r->x+2;
	ix2=r->x+r->xsize-4;
	nLoc=0;
	nCount=rast[0];
	nFlag=ion[0];
	/* uMoveU(ix1,iy,&d->s); */
	riMoveTo(ix1,iy,d);
	
	for(k=ix1;k<=ix2;++k){
		if(nFlag){
			/* uLineU(k,iy,d->nLineFore,&d->s); */
			riLineTo(k,iy,d);
		}
		if(--nCount <= 0){
			if(++nLoc >= nPart)nLoc=0;
			nCount=rast[nLoc];
			/* if(!nFlag && ion[nLoc])uMoveU(k,iy,&d->s); */
			if(!nFlag && ion[nLoc])riMoveTo(k,iy,d);
			nFlag=ion[nLoc];
		}

	}
	return 0;
}

static void CPlotDashedLine(struct uGridPlot *b,double *xx,double *yy,int nn,char *pat,DOListPtr d,struct uLineData *Line)
{
	double dxy,dist,xl,yl,xs,ys,xp,yp,xt,yt; 
	double x,y;
	double rast[16];
	int ion[16];
	int  flag,back,n,i,np;
	int ns;
	int i1;
      
      
	  if(!b || !xx || !yy || !pat || !d || !Line)return;
      n=0;
      for(i=0;i<16;++i){
		  if(pat[i] == '0' || pat[i] == 0)break;	
		  ++n;
		  np=pat[i]-'0';
		  rast[i]=(double)np/(double)d->dpi;
		  ion[i]=FALSE;
		  if(np & 1)ion[i]=TRUE;
      }
/*
c
c        start pattern
c
*/
	if(n <= 0)return;


	ns=0;
	flag=ion[ns];
	dxy=rast[ns];
	back=FALSE;
	
	x=Line->xShift+Line->xScale*xx[0];
	y=Line->yShift+Line->yScale*yy[0];

	if(!PlotrPoint(b,&x,&y,&xs,&ys)){
	    rMoveTo(xs,ys,d);
	}
	xl=Line->xShift+Line->xScale*xx[0];
	yl=Line->yShift+Line->yScale*yy[0];
	for(np=1;np<nn;++np){
		xt=Line->xShift+Line->xScale*xx[np];
		yt=Line->yShift+Line->yScale*yy[np];	
		if(clip2(b,&xl,&yl,&xt,&yt,&i1))goto l115;
		if(i1 == 0)goto l60;
				
		if(CPlotrPoint(b,&xl,&yl,&xp,&yp)){
		    goto l115;
		}
		rMoveTo(xp,yp,d);
		xs=xp;
		ys=yp;
l60:	
		back=FALSE;
		if(CPlotrPoint(b,&xt,&yt,&xp,&yp)){
		    goto l115;
		}
		dist=sqrt((xp-xs)*(xp-xs)+(yp-ys)*(yp-ys));
		if(dist >= dxy){
			xp=xs+(xp-xs)*dxy/dist;
			yp=ys+(yp-ys)*dxy/dist;
			back=TRUE;
		}
		if(flag){
			rLineTo(xp,yp,d);
		}else{
			rMoveTo(xp,yp,d);
		}
		xs=xp;
		ys=yp;
		dxy=dxy-dist;
		if(back){
			ns=ns+1;
			if(ns >= n)ns=0;
			dxy=rast[ns];
			flag=ion[ns];
			goto l60;
		}
l115:
		xl=Line->xShift+Line->xScale*xx[np];
		yl=Line->yShift+Line->yScale*yy[np];
	}
}
static int clip2(struct uGridPlot *b,double *x1i,double *y1i,double *x2i,double *y2i,int *i1i)
{
	double sxn,syn,sxx,syx;
	double x1, y1, x2, y2;
	int i1,i2,iflip;
	double s[3],dnom,t;
	double xt,yt,sn,sx;
	
	if(!b || !x1i || !y1i || !x2i || !y2i || !i1i)return 1;
	
	x1 = *x1i;
	x2 = *x2i;
	y1 = *y1i;
	y2 = *y2i;

	i1=0;
	i2=0;
	
      if(x1 < b->xViewMin || x1 > b->xViewMax)i1=1;
      if(x2 < b->xViewMin || x2 > b->xViewMax)i2=1;
      if(y1 < b->yViewMin || y1 > b->yViewMax)i1=1;
      if(y2 < b->yViewMin || y2 > b->yViewMax)i2=1;
      
      *i1i=i1;
      
      if(i1 == 0 && i2 == 0){
          return 0;
      }
      sxn=-1.;
      syn=-1.;
      sxx=-1.;
      syx=-1.;
      iflip=0;
      dnom=x2-x1;
      if(fabs(dnom) <= b->dxLim)goto l20;
      sxn=(b->xViewMin-x1)/dnom;
      sxx=(b->xViewMax-x1)/dnom;
 l20: dnom=y2-y1;
      if(fabs(dnom) <= b->dyLim)goto l30;
      syn=(b->yViewMin-y1)/dnom;
      syx=(b->yViewMax-y1)/dnom;
 l30: if(sxn < 0.0 || sxn > 1.)goto l40;
      t=sxn*y2+(1.-sxn)*y1;
      if(t < b->yViewMin || t > b->yViewMax) goto l40;
      iflip=iflip+1;
      s[iflip]=sxn;
 l40: if(sxx < 0.0 || sxx > 1.)goto l50;
      t=sxx*y2+(1.-sxx)*y1;
      if(t < b->yViewMin || t > b->yViewMax) goto l50;
      iflip=iflip+1;
      s[iflip]=sxx;
      if(iflip > 1) goto l70;
 l50: if(syn < 0.0 || syn > 1.)goto l60;
      t=syn*x2+(1.-syn)*x1;
      if(t < b->xViewMin || t > b->xViewMax) goto l60;
      iflip=iflip+1;
      s[iflip]=syn;
      if(iflip > 1) goto l70;
 l60: if(syx < 0.0 || syx > 1.)goto l70;
      t=syx*x2+(1.-syx)*x1;
      if(t < b->xViewMin || t > b->xViewMax) goto l70;
      iflip=iflip+1;
      s[iflip]=syx;
 l70: 
      if(iflip == 0) return 1;
      if(iflip > 1)goto l90;
      if(i1 == 0) goto l80;
      *x1i=s[1]*x2+(1.-s[1])*x1;
      *y1i=s[1]*y2+(1.-s[1])*y1;
      return 0;
 l80: 
      *x2i=s[1]*x2+(1.-s[1])*x1;
      *y2i=s[1]*y2+(1.-s[1])*y1;
      return 0;
l90: 
      xt=x1;
      yt=y1;
      sn=min(s[1],s[2]);
      sx=max(s[1],s[2]);
      *x1i=sn*x2+(1.-sn)*xt;
      *y1i=sn*y2+(1.-sn)*yt;
      *x2i=sx*x2+(1.-sx)*xt;
      *y2i=sx*y2+(1.-sx)*yt;
      return 0;
}

int uSetAttributes(DOListPtr l,struct uAttributes *a,int Flag)
{
	
	if(!l || !a)return 1;

	if(Flag & sForeColor){
	    l->s.nSolidFore=a->nSolidFore;
	    /* uSetForeByIndex(a->nSolidFore,l->myIcon); */
	}
	if(Flag & sBackColor){
	    l->s.nSolidBack=a->nSolidBack;
	    /* uSetBackByIndex(a->nSolidBack,l->myIcon); */
	}
	if(Flag & sPattern){
	    l->s.SolidPattern=a->SolidPattern;
	}
	if(Flag & lForeColor){
		l->nLineFore=a->nLineFore;
	    l->s.nLineFore=a->nLineFore;
	    /* uSetForeByIndex(a->nLineFore,l->myIcon); */
	}
	if(Flag & lBackColor){
		l->nLineBack=a->nLineBack;
	    l->s.nLineBack=a->nLineBack;
	    /* uSetBackByIndex(a->nLineBack,l->myIcon); */
	}
	
	if(Flag & lPattern){
	    l->s.LinePattern=a->LinePattern;
	}
	
	if(Flag & lWidth){
	    l->s.xLineWidth=(int)a->xLineWidth;
	    l->s.yLineWidth=(int)a->yLineWidth;
	}
	
	if(Flag & tFont){
		/* uSetTextFace(l->myIcon,a->font.txFace); */
		
		/* uSetTextFont(l->myIcon,a->font.txFont); */
		
		/* uSetFontSize(l->myIcon,a->font.txSize); */
	}

	return 0;
}

static int PlotrPoint(struct uGridPlot *b,double *x,double *y,double *ix,double *iy)
{

	int ret;
	
	if(!b || !ix || !iy || !x ||  !y)return 0;
	
	ret=0;
	
	if(b->xLogScale){
	    if(*x < b->xViewMin*(1-sign(b->Small,b->xViewMin))){
	        *ix = (log10(b->xViewMin)-b->xViewMinLog)*b->xScaleLog+b->xOff;
			ret=1;
	    }else if(*x > b->xViewMax*(1+sign(b->Small,b->xViewMax))){
	        *ix = (log10(b->xViewMax)-b->xViewMinLog)*b->xScaleLog+b->xOff;
			ret=1;
	    }else{
	        *ix = (log10(*x)-b->xViewMinLog)*b->xScaleLog+b->xOff;
	    }
	}else{
	    if(*x < b->xViewMin*(1-sign(b->Small,b->xViewMin))){
	    	*ix = b->xOff;
			ret=1;
	    }else if(*x > b->xViewMax*(1+sign(b->Small,b->xViewMax))){
	    	*ix = (b->xViewMax-b->xViewMin)*b->xScale+b->xOff;
			ret=1;
	    }else{
	    	*ix = (*x-b->xViewMin)*b->xScale+b->xOff;
	    }
	}
	if(b->yLogScale){
	    if(*y < b->yViewMin*(1-sign(b->Small,b->yViewMin))){
	        *iy = (log10(b->yViewMin)-b->yViewMinLog)*b->yScaleLog+b->yOff;
			ret=1;
	    }else if(*y > b->yViewMax*(1+sign(b->Small,b->yViewMax))){
	        *iy = (log10(b->yViewMax)-b->yViewMinLog)*b->yScaleLog+b->yOff;
			ret=1;
	    }else{
	        *iy = (log10(*y)-b->yViewMinLog)*b->yScaleLog+b->yOff;
	    }
	}else{
	    if(*y < b->yViewMin*(1-sign(b->Small,b->yViewMin))){
	    	*iy = b->yOff;
			ret=1;
	    }else if(*y > b->yViewMax*(1+sign(b->Small,b->yViewMax))){
	    	*iy = (b->yViewMax-b->yViewMin)*b->yScale+b->yOff;
			ret=1;
	    }else{
	    	*iy = (*y-b->yViewMin)*b->yScale+b->yOff;
	    }
	}
	return ret;
}

static int CPlotrPoint(struct uGridPlot *b,double *x,double *y,double *xr,double *yr)
{

	if(!b || !x || !y || !xr || !yr)return 0;
	
	if(b->xLogScale){
	    if(*x < b->xViewMin){
	        return 1;
	    }else{
	        *xr = (log10(*x)-b->xViewMinLog)*b->xScaleLog+b->xOff;
	    }
	}else{
	    *xr = (*x-b->xViewMin)*b->xScale+b->xOff;
	}
	if(b->yLogScale){
	    if(*y < b->yViewMin){
	        return 1;
	    }else{
	        *yr = (log10(*y)-b->yViewMinLog)*b->yScaleLog+b->yOff;
	    }
	}else{
	    *yr = (*y-b->yViewMin)*b->yScale+b->yOff;
	}
	return 0;
}

int rDrawString(char *name,DOListPtr d)
{
	int kc;
	
	if(!name || !d)return 1;
	
	kc=d->nLineFore;
	
	glColor3d(g.palette[kc*3]/255.0,g.palette[kc*3+1]/255.0,g.palette[kc*3+2]/255.0);
	
	DrawString(xOld, yOld, name);
	
/*	
	if(d->PrintFlag){
		uDrawString(name,d->myIcon);		    
	}else{
		uDrawStringU(name,d->nLineFore,0,&d->s);		    
	}
*/
	return 0;
	
}

int uCStringWidth(char *name,int myIcon)
{

/*
    XFontStruct *f;
    int width;
    
	if(!myIcon || !name || !xg.mydisplay)return 1;
	
	if(!myIcon->info){
        myIcon->info=XQueryFont(xg.mydisplay,XGContextFromGC(myIcon->gc));
    }
    f=myIcon->info;
    if(!f)return 1;
    width=XTextWidth(f,name,(int)strlen(name));
	return width;
*/
	return 8*(int)strlen(name);
}

int uGetFontInfo(int myIcon,uFontInfo *t)
{

/*
    XFontStruct *f;
    XCharStruct *c;
	    
	if(!myIcon || !t || !xg.mydisplay)return 1;
	
    f=XQueryFont(xg.mydisplay,XGContextFromGC(myIcon->gc));
    if(!f)return 1;
    c=&(f->max_bounds);
    t->CWidth = c->width;	
    t->LineHeight=f->ascent+f->descent;
    t->Descent=f->descent;	    
	XFreeFontInfo(NULL, f, 1);
*/

    t->CWidth = 8;	
    t->LineHeight=13;
    t->Descent=0;	    


	return 0;
}


int rLineTo(double x,double y,DOListPtr d)
{

    int ix,iy,kc;
	
	if(!d)return 1;
	
	ix=(int)(x*(double)d->dpi + 0.5);
	iy=(int)(y*(double)d->dpi + 0.5);
	
	kc=d->nLineFore;
	
	glColor3d(g.palette[kc*3]/255.0,g.palette[kc*3+1]/255.0,g.palette[kc*3+2]/255.0);
	
	DrawLine(xOld, yOld, ix, iy);
	
	xOld=ix;
	yOld=iy;
	

/*
	if(d->PrintFlag){
		uLineTo(ix+d->PrintRect.x,iy+d->PrintRect.y,d->myIcon);
	}else{
		uLineU(ix,iy,d->nLineFore,&d->s);
	}
*/
	return 0;
}
int	rMoveTo(double x,double y,DOListPtr d)
{

	int ix,iy;
	
	if(!d)return 1;

	ix=(int)(x*(double)d->dpi + 0.5);
	iy=(int)(y*(double)d->dpi + 0.5);
	
	xOld=ix;
	yOld=iy;
/*
	if(d->PrintFlag){
	   uMoveTo(ix+d->PrintRect.x,iy+d->PrintRect.y);
	}else{
	   uMoveU(ix,iy,&d->s);
	}
*/
	return 0;
}

double PixelToLocal(long x,DOListPtr d)
{
	double xx;
	
	if(d && d->dpi){
	    xx=(double)x/(double)d->dpi;
	}else{
	    xx=0;
	}
	
	return xx;
}
int LocalToPixel(double x,DOListPtr d)
{
	int ix;
	
	
	if(d){
		ix=(int)(x*(double)d->dpi + 0.5);
	}else{
		ix=0;
	}
	
	return ix;
}

static int CPlotxLin(struct uGridPlot *b,rRect *r,DOListPtr d)
{
	double LineHeight,width,cWidth;
	double ix,iyy;
	char Label[256];
	uFontInfo Info;
	double x,End,xSmall;
	int n;
	
	if(!b || !r || !d)return 1;
				
	
	xSmall=1.0e-5*(b->xMaximum-b->xMinimum);
	End=b->xViewMax+xSmall;
	
	uGetFontInfo(d->myIcon,&Info);
	
	LineHeight=PixelToLocal(Info.LineHeight,d);
	
	cWidth=PixelToLocal(Info.CWidth,d);

	iyy = r->y+r->ysize-LineHeight-0.5*cWidth;
	
	if(((b->xViewMax-b->xViewMin)/b->xMajorStep) > b->xMajorLimit){
	    b->xMajorStep=(b->xViewMax-b->xViewMin)/(double)b->xMajorLimit;
	}
	
	b->xCount=0;
	for(x=b->xViewMin;x <= End;x += b->xMajorStep){
	    b->xCount++;
	}
/*
	if(b->xBoxes)cFree((char *)b->xBoxes);
	b->xBoxes=cMalloc(sizeof(rRect)*(1+b->xCount),8742);
*/
	n=0;
    
    double Start=b->xMajorStep*((long long)(b->xViewMin/b->xMajorStep))-b->xMajorStep;
    End=b->xMajorStep*((long long)(b->xViewMax/b->xMajorStep))+b->xMajorStep;
    
    //fprintf(stderr,"Start %f End %f Step %f\n",Start,End,b->xMajorStep);
    
    for(x=Start;x <= End;x += b->xMajorStep){
        if(x < (b->xViewMin-xSmall) || x > (b->xViewMax+xSmall))continue;
    		ix=(x-b->xViewMin)*b->xScale+b->xOff;
		msprintf(Label,sizeof(Label),"%.2f%c",x/1e6,0);

		width=PixelToLocal(uCStringWidth(Label,d->myIcon),d);

		ix -= 0.5*width;

		rMoveTo(ix+b->xBoxOffset.x,iyy+b->xBoxOffset.y,d);
		rDrawString(Label,d);
        /*
		if(b->xBoxes && (n < b->xCount)){
		    b->xBoxes[n].x=ix+b->xBoxOffset.x;
		    b->xBoxes[n].y=iyy-LineHeight+b->xBoxOffset.y;
		    b->xBoxes[n].xsize=width+cWidth;
		    b->xBoxes[n].ysize=LineHeight;
		    
			rBoxDraw(&b->xBoxes[n],d);
			
			++n;
		}
        */
	}
	
	b->xCount=n;
	
	return 0;
}
static int CPlotxLog(struct uGridPlot *b,rRect *r,DOListPtr d)
{
	double LineHeight,width,cWidth;
	double iyy,ix,iy;
	int iStart,iEnd;
	char Label[256];
	uFontInfo Info;
	double x,y,End,Start,Step,Exp;
	int n;
	
	if(!b || !r || !d)return 1;
	

	uGetFontInfo(d->myIcon,&Info);
	
	LineHeight=PixelToLocal(Info.LineHeight,d);
	
	cWidth=PixelToLocal(Info.CWidth,d);

	iyy = r->y-b->xMajorOut-LineHeight-0.5*cWidth;	
	
	End=log10(b->xViewMax)*(1+b->Small);
	iEnd=(int)End;
	iStart=(int)(b->xViewMinLog-.9999);
	Start=iStart;
	y=(b->yViewMax+b->yViewMin)/2.;
	if((iEnd-iStart) <= 9){
	    Step=1;
	}else{
	    Step=4;
	}

	if(((End-Start)/Step) > b->xMajorLimit){
	    Step=(End-Start)/(double)b->xMajorLimit;
	}


	b->xCount=0;
	for(x=Start;x <= End;x += Step){
	    b->xCount++;
	}
	
	if(b->xBoxes)cFree((char *)b->xBoxes);
	b->xBoxes=cMalloc(sizeof(rRect)*(1+b->xCount),8742);

	n=0;

	for(x=Start;x <= End;x += Step){
		Exp=pow(10.,x);

		if(PlotrPoint(b,&Exp,&y,&ix,&iy))continue;
		    
		msprintf(Label,sizeof(Label),"%g%c",Exp,0);
		width=PixelToLocal(uCStringWidth(Label,d->myIcon),d);

		ix -= 0.5*width;

		rMoveTo(ix+b->xBoxOffset.x,iyy+b->xBoxOffset.y,d);
		rDrawString(Label,d);		    
		if(b->xBoxes && (n < b->xCount)){
		    b->xBoxes[n].x=ix+b->xBoxOffset.x;
		    b->xBoxes[n].y=iyy-LineHeight+b->xBoxOffset.y;
		    b->xBoxes[n].xsize=width+cWidth;
		    b->xBoxes[n].ysize=LineHeight;
		    /*
			rBoxDraw(&b->xBoxes[n],d);
			*/
			++n;
		}
	}

	b->xCount=n;
	
	return 0;
}
static int CPlotyLin(struct uGridPlot *b,rRect *r,DOListPtr d)
{
	double LineHeight,width,cWidth;
	double ixx,iy;
	char Label[256];
	uFontInfo Info;
	double y,End,ySmall;
	int n;
	
	if(!b || !r || !d)return 1;

	ySmall=1.0e-5*(b->yViewMax-b->yViewMin);
	End=b->yViewMax+ySmall;
	
	uGetFontInfo(d->myIcon,&Info);
	
	LineHeight=PixelToLocal(Info.LineHeight,d);
	
	cWidth=PixelToLocal(Info.CWidth,d);
	
    ixx=r->x-b->yMajorOut-0.75*cWidth;
    ixx=r->x+b->yMajorOut+4.0*cWidth;

	if(((b->yViewMax-b->yViewMin)/b->yMajorStep) > b->yMajorLimit){
	    b->yMajorStep=(b->yViewMax-b->yViewMin)/(double)b->yMajorLimit;
	}
	
	b->yCount=0;
	for(y=b->yViewMin;y <= End;y += b->yMajorStep){
	    b->yCount++;
	}
	
	if(b->yBoxes)cFree((char *)b->yBoxes);
	b->yBoxes=cMalloc(sizeof(rRect)*(1+b->yCount),8743);
		
	n=0;
	for(y=b->yViewMin;y <= End;y += b->yMajorStep){
	    iy=(y-b->yViewMin)*b->yScale+b->yOff;
		msprintf(Label,sizeof(Label),"%g%c",y,0);
        

		// iy -= (LineHeight)/2;

		width=PixelToLocal(uCStringWidth(Label,d->myIcon),d);

		rMoveTo(ixx-width+b->yBoxOffset.x,iy+b->yBoxOffset.y,d);
        
		rDrawString(Label,d);		    
		if(b->yBoxes && (n < b->yCount)){
		    b->yBoxes[n].x=ixx-width+b->yBoxOffset.x;
		    b->yBoxes[n].y=iy-LineHeight+b->yBoxOffset.y;
		    b->yBoxes[n].xsize=width+cWidth;
		    b->yBoxes[n].ysize=LineHeight;
		    /*
			rBoxDraw(&b->yBoxes[n],d);
			*/
			++n;
		}
	}
	
	b->yCount=n;
	
	return 0;
}
static int CPlotyLog(struct uGridPlot *b,rRect *r,DOListPtr d)
{
	double ix,ixx,iy,LineHeight,cWidth,width;
	int iEnd,iStart;
	char Label[256];
	uFontInfo Info;
	double x,y,End,Start,Exp,Step;
	int n;
	
	if(!b || !r || !d)return 1;

	uGetFontInfo(d->myIcon,&Info);
	
	LineHeight=PixelToLocal(Info.LineHeight,d);
	
	cWidth=PixelToLocal(Info.CWidth,d);
	
	End=log10(b->yViewMax);
	End=End+fabs(End)*b->Small;
	iEnd=(int)End;
	iStart=(int)(b->yViewMinLog-.9999);
	Start=iStart;
	x=(b->xViewMax+b->xViewMin)/2.;
	if((iEnd-iStart) <= 9){
	    Step=1;
	}else{
	    Step=4;
	}
		
    ixx=r->x-b->yMajorOut-0.75*cWidth;
    
    ixx=r->x+b->yMajorOut+5*cWidth;

	if(((End-Start)/Step) > b->yMajorLimit){
	    Step=(End-Start)/(double)b->yMajorLimit;
	}
	
	b->yCount=0;

	for(y=Start;y <= End;y += Step){
	    b->yCount++;
	}
	
	if(b->yBoxes)cFree((char *)b->yBoxes);
	b->yBoxes=cMalloc(sizeof(rRect)*(1+b->yCount),8743);
		
	n=0;	
	for(y=Start;y <= End;y += Step){
		Exp=pow(10.,y);
		if(PlotrPoint(b,&x,&Exp,&ix,&iy))continue;
		msprintf(Label,sizeof(Label),"%g%c",Exp,0);
		width=PixelToLocal(uCStringWidth(Label,d->myIcon),d);


		iy -= 0.3*LineHeight;
        
		rMoveTo(ixx-width+b->yBoxOffset.x,iy+b->yBoxOffset.y,d);
		rDrawString(Label,d);		    
		if(b->yBoxes && (n < b->yCount)){
		    b->yBoxes[n].x=ixx-width+b->yBoxOffset.x;
		    b->yBoxes[n].y=iy-LineHeight+b->yBoxOffset.y;
		    b->yBoxes[n].xsize=width+cWidth;
		    b->yBoxes[n].ysize=LineHeight;
		    /*
			rBoxDraw(&b->yBoxes[n],d);
			*/
			++n;
		}
	}
	
	b->yCount=n;
	
	return 0;
}


static int GridPlotSetSelection(struct uGridPlot *b,DOListPtr d)
{
	double rmax,dr;
	rRect r;
	int n;
	
	if(!b || !d)return 1;
	
	if(b->xBoxes && (b->xCount > 0)){
	    r=b->xBoxes[0];
	    for(n=1;n<b->xCount;++n){
	        dr=r.x-b->xBoxes[n].x;
			if(dr > 0){
			    r.x=b->xBoxes[n].x;
			    r.xsize += dr;
			}
			rmax=b->xBoxes[n].x + b->xBoxes[n].xsize;
			if(rmax > (r.x + r.xsize))r.xsize=rmax-r.x;
			
	        dr=r.y-b->xBoxes[n].y;
			if(dr > 0){
			    r.y=b->xBoxes[n].y;
			    r.ysize += dr;
			}
			rmax=b->xBoxes[n].y + b->xBoxes[n].ysize;
			if(rmax > (r.y + r.ysize))r.ysize=rmax-r.y;
			
	    }
	    b->xBoxeSelect=r;
	}
	
	if(b->yBoxes && (b->yCount > 0)){
	    r=b->yBoxes[0];
	    for(n=1;n<b->yCount;++n){
	        dr=r.x-b->yBoxes[n].x;
			if(dr > 0){
			    r.x=b->yBoxes[n].x;
			    r.xsize += dr;
			}
			rmax=b->yBoxes[n].x + b->yBoxes[n].xsize;
			if(rmax > (r.x + r.xsize))r.xsize=rmax-r.x;
			
	        dr=r.y-b->yBoxes[n].y;
			if(dr > 0){
			    r.y=b->yBoxes[n].y;
			    r.ysize += dr;
			}
			
			
			
			rmax=b->yBoxes[n].y + b->yBoxes[n].ysize;
			if(rmax > (r.y + r.ysize))r.ysize=rmax-r.y;
	    }
	    b->yBoxeSelect=r;
	}
	
	return 0;
}

static int CPlotPlotLegends(struct uGridPlot *b,DOListPtr d)
{
	double LineHeight,width,widthMax,cWidth;
	struct uLineData *Line;
	double ix,iy,i30;
	uFontInfo Info;
	iRect image;
	int Symbol;
	uRect box;
	rRect r;
	int n;
	
	if(!b || !d)return 1;
			
	if(b->Lines && (b->LineCount > 0)){	
		Line=b->Lines[0];
		
		if(Line)uSetAttributes(d,&Line->Attributes,lPattern | lForeColor | lBackColor | tFont);
						
		r=b->box;

		uGetFontInfo(d->myIcon,&Info);
		
		LineHeight=PixelToLocal(Info.LineHeight,d);
		
		i30=PixelToLocal(30,d);

		cWidth=PixelToLocal(Info.CWidth,d);
		
		
		widthMax=0;
				
		for(n=0;n<b->LineCount;++n){
			Line=b->Lines[n];
			if(!Line)continue;
			uSetAttributes(d,&Line->Attributes,lPattern | lForeColor | lBackColor | lWidth);
/*		    
		    if(!Line->xData || !Line->yData || !Line->save[0])continue;
*/
			ix=r.x+r.xsize+b->xMajorOut+cWidth+b->yLegendOffset.x;
		
			iy=r.y+r.ysize+b->yLegendOffset.y;
		    
		    width=PixelToLocal(uCStringWidth(Line->save,d->myIcon),d);
		    
		    if(width > widthMax)widthMax=width;
		    
	        rMoveTo(ix+i30,iy-n*LineHeight,d);
		  //  rDrawString(Line->save,d);		    

		    box.x=LocalToPixel(ix,d);
		    box.y=LocalToPixel(iy-n*LineHeight-0.25*LineHeight,d);
		    box.xsize=30;
		    box.ysize=Info.LineHeight;
			    
			if(Line->Attributes.DashFlag){
				rLinePat(&box,(char *)&Line->Attributes.LineDash,d);
			}else{
			    rMoveTo(ix,iy-n*LineHeight+0.25*LineHeight,d);
			    rLineTo(ix+i30-i30/6,iy-n*LineHeight+0.25*LineHeight,d);
			}
			if(Line->Attributes.LineIncrement > 0){
				Symbol=Line->Attributes.LineSymbol;
				if(Symbol >= 0 && Symbol < 40){
				    image=d->Picts[Symbol];
				    image.box.x=box.x+10;
				    image.box.y=box.y;
				    rDrawImageRect(&image,d);	
				}
			}
		    
			Line->box.x=ix;
			Line->box.y=iy+n*LineHeight-LineHeight;
			Line->box.xsize=widthMax+cWidth+i30;
			Line->box.ysize=LineHeight;
			/*
			rBoxDraw(&Line->box,d);
			*/
		}				
	}
	
	return 0;
}
int CPlotPlotNumbers(struct uGridPlot *b,DOListPtr d)
{
	rRect r;
	
	if(!b || !d)return 1;
	
	r=b->box;
	
	if(b->bAxisNumber){
		uSetAttributes(d,&b->xAttributes,lPattern | lForeColor | lBackColor | tFont | lWidth);
		if(b->xLogScale){
		    CPlotxLog(b,&r,d);		 
		}else{		
		    CPlotxLin(b,&r,d);
		}
	}
	if(b->lAxisNumber){
		uSetAttributes(d,&b->yAttributes,lPattern | lForeColor | lBackColor | tFont | lWidth);
		if(b->yLogScale){
		    CPlotyLog(b,&r,d);		 
		}else{	
		    CPlotyLin(b,&r,d);		
		}
	}
	return 0;
}
static int GridPlotLineGrid(struct uGridPlot *b,DOListPtr d)
{
	struct uLineData *Line;
	int Increment,Symbol;
	double ix,iy;
	double xl,yl,xt,yt,xp,yp;
	double x,y;
	int i1,iMove;
	iRect image;
	long n,k;
	
	if(!b || !d)return 1;

	if(!b->Lines || (b->LineCount <= 0))return 0;	
		
	for(n=0;n<b->LineCount;++n){
		Line=b->Lines[n];
		if(!Line)continue;
	    
	    if(!Line->xData || !Line->yData )continue;
	    
		uSetAttributes(d,&Line->Attributes,lPattern | lForeColor | lBackColor | lWidth);
		
		
		Increment=Line->Attributes.LineIncrement;
		Symbol=Line->Attributes.LineSymbol;
		if(Increment > 0 && (Symbol >=0 && Symbol < 40)){
		    for(k=0;k<Line->dCount;++k){
				if(Increment > 0 && !(k % Increment)){
				    x=Line->xShift+Line->xScale*Line->xData[k];
				    y=Line->yShift+Line->yScale*Line->yData[k];
				    if(PlotrPoint(b,&x,&y,&ix,&iy))continue;
			        rMoveTo(ix,iy,d);
			        image=d->Picts[Symbol];
				    image.box.x=LocalToPixel(ix,d)-5;
				    image.box.y=LocalToPixel(iy,d)-5;
				    rDrawImageRect(&image,d);	
				}
		    }
		}		

		if(Line->Attributes.DashFlag){
	    	/* FlushCurrentPort(); */		
	        CPlotDashedLine(b,Line->xData,Line->yData,(int)Line->dCount,(char *)&Line->Attributes.LineDash,d,Line);
	    	/* FlushCurrentPort();	  */    
	      	continue;
	    }

		iMove=FALSE;
	    xl=Line->xShift+Line->xScale*Line->xData[0];
	    yl=Line->yShift+Line->yScale*Line->yData[0];
		if(!CPlotrPoint(b,&xl,&yl,&xp,&yp)){
			rMoveTo(xp,yp,d);
			iMove=TRUE;
		}	    
	    for(k=1;k<Line->dCount;++k){
			xl=Line->xShift+Line->xScale*Line->xData[k-1];
			yl=Line->yShift+Line->yScale*Line->yData[k-1];	
			xt=Line->xShift+Line->xScale*Line->xData[k];
			yt=Line->yShift+Line->yScale*Line->yData[k];	
			if(clip2(b,&xl,&yl,&xt,&yt,&i1)){
			    continue;
			}
			if(i1 != 0){
				if(CPlotrPoint(b,&xl,&yl,&xp,&yp)){
				    continue;
				}
				rMoveTo(xp,yp,d);
				iMove=TRUE;
			}

			if(CPlotrPoint(b,&xt,&yt,&xp,&yp)){
		    	continue;
			}
			if(!iMove){
			    rMoveTo(xp,yp,d);
				iMove=TRUE;
			}
			rLineTo(xp,yp,d);
	    }	    	    
	}
		
	return 0;
	
}

int GridPlotPlotGrid(struct uGridPlot *b,DOListPtr d)
{
	double xsize2,ysize2;
	double right,bottom;
	double mult;
/*	IconPtr myIcon; */
	rRect r;
	
	if(!b || !d)return 1;
	
	r=b->box;
	
/*
 	xsize2=b->vAttributes.xLineWidth/2;
 	ysize2=b->vAttributes.yLineWidth/2;
*/
	xsize2=0;
	ysize2=0;
	
	mult=1;
	
	right=r.x+r.xsize;
	bottom=r.y+r.ysize;
 	 	
 	
	uSetAttributes(d,&b->Attributes,lPattern | lForeColor | lBackColor | lWidth);

	if(b->lAxis){
	    GridPlotxGridDraw(b,d,bottom,b->xMajorIn,b->xMajorOut,b->xMinorIn,b->xMinorOut,mult,&r);
	    rMoveTo(r.x-xsize2,  bottom-ysize2,d);
	    rLineTo(right-xsize2,bottom-ysize2,d);
	}
	if(b->rAxis){
	    GridPlotxGridDraw(b,d,r.y,b->tMajorIn,b->tMajorOut,b->tMinorIn,b->tMinorOut,-mult,&r);
	    rMoveTo(r.x-xsize2,  r.y-ysize2,d);
	    rLineTo(right-xsize2,r.y-ysize2,d);
	}
	if(b->bAxis){
	    GridPlotyGridDraw(b,d,r.x,b->yMajorIn,b->yMajorOut,b->yMinorIn,b->yMinorOut,mult,&r);
	    rMoveTo(r.x-xsize2,r.y-ysize2,d);
	    rLineTo(r.x-xsize2,bottom-ysize2,d);
	}
	if(b->tAxis){
	    GridPlotyGridDraw(b,d,right,b->rMajorIn,b->rMajorOut,b->rMinorIn,b->rMinorOut,-mult,&r);
	    rMoveTo(right-xsize2,r.y-ysize2,d);
	    rLineTo(right-xsize2,bottom-ysize2,d);
	}
	/* SetPenNormal(myIcon); */
	return  0;
}
static void GridPlotyGridDraw(struct uGridPlot *b,DOListPtr d,double x,double yMajorIn,double yMajorOut,double yMinorIn,double yMinorOut,double mult,rRect *r)
{
	double MajorIn,MajorOut,MinorIn,MinorOut;
	double xx,y,yy,End,mEnd,Start,Exp,Step,Expm,ySmall;
	int iStart,iEnd,n;
	double xsize2,ysize2;
	double ix,iy;
	
	if(!b || !r || !d)return;
	
	/*
 	xsize2=b->vAttributes.xLineWidth/2;
 	ysize2=b->vAttributes.yLineWidth/2;
	*/

 	xsize2=0;
 	ysize2=0;
	
	MajorIn=x+yMajorIn*mult;
	MajorOut=x-yMajorOut*mult;
	MinorIn=x+yMinorIn*mult;
	MinorOut=x-yMinorOut*mult;
	End=b->yViewMax*(1+b->Small);
	mEnd=b->yMajorStep*(1-b->Small);
	if(b->yLogScale){
	    End=log10(b->yViewMax)*(1+b->Small);
	    iEnd=(int)End;
	    iStart=(int)(b->yViewMinLog-.9999);
	    Start=iStart;
	    xx=(b->xViewMax+b->xViewMin)/2.;
	    if((iEnd-iStart) <= 9){
	        Step=1;
	    }else{
	        Step=4;
	    }

		if(((End-Start)/Step) > b->yMajorLimit){
		    Step=(End-Start)/(double)b->yMajorLimit;
		}
	    	    
	    for(y=Start;y <= End;y += Step){
	        Exp=pow(10.,y);
	        if(!PlotrPoint(b,&xx,&Exp,&ix,&iy)){
	            rMoveTo(MajorIn-xsize2,iy-ysize2,d);
	            rLineTo(MajorOut-xsize2,iy-ysize2,d);
	            if(b->gridHash){
	            	double xx,yy,x1,x2;
	            	yy=LocalToPixel(iy,d);
	            	x1=LocalToPixel(r->x,d);
	            	x2=LocalToPixel(r->x+r->xsize,d);
	                for(xx=x1;xx<x2;xx += 5){
	            	    riMoveTo((int)xx,(int)yy,d);
	            	    riLineTo((int)(xx+1),(int)yy,d);
	            	}
	            }
	        }
	        if(Step == 1){
	            Expm=Exp+Exp;
	            for(n=0;(n<8) && (log10(Expm) < End);++n,Expm += Exp){
					if(!PlotrPoint(b,&xx,&Expm,&ix,&iy)){
					    rMoveTo(MinorIn-xsize2,iy-ysize2,d);
					    rLineTo(MinorOut-xsize2,iy-ysize2,d);
					}
		        }
	        }
	    }
	}else{

		if(((b->yViewMax-b->yViewMin)/b->yMajorStep) > b->yMajorLimit){
	    	b->yMajorStep=(b->yViewMax-b->yViewMin)/(double)b->yMajorLimit;
		}

	    ySmall=1.0e-5*(b->yViewMax-b->yViewMin);
	    End=b->yViewMax+ySmall;
	    mEnd=b->yMajorStep-ySmall;
	   	Start=b->yViewMin;
	   	
		if(((b->yMajorStep)/b->yMinorStep) > b->yMinorLimit){
	    	b->yMinorStep=(b->yMajorStep)/(double)b->yMinorLimit;
		}	   	
	   	
	    for(y=Start;y <= End;y += b->yMajorStep){
	            iy=(y-b->yViewMin)*b->yScale+b->yOff;
	            rMoveTo(MajorIn-xsize2,iy-ysize2,d);
	            rLineTo(MajorOut-xsize2,iy-ysize2,d);
	            if(b->gridHash){
	            	double xx,yy,x1,x2;
	            	yy=LocalToPixel(iy,d);
	            	x1=LocalToPixel(r->x,d);
	            	x2=LocalToPixel(r->x+r->xsize,d);
	                for(xx=x1;xx<x2;xx += 5){
	            	    riMoveTo((int)xx,(int)(yy+1),d);
	            	    riLineTo((int)(xx+1),(int)(yy+1),d);
	            	}
	            }
	        for(yy=y+b->yMinorStep;
	            yy < y+mEnd && yy < End;yy += b->yMinorStep){
	                iy=(yy-b->yViewMin)*b->yScale+b->yOff;
	                rMoveTo(MinorIn-xsize2,iy-ysize2,d);
	                rLineTo(MinorOut-xsize2,iy-ysize2,d);
	        }
	    }
	}
}
static void GridPlotxGridDraw(struct uGridPlot *b,DOListPtr d,double y,double xMajorIn,double xMajorOut,double xMinorIn,double xMinorOut,double mult,rRect *r)
{
	double MajorIn,MajorOut,MinorIn,MinorOut;
	double x,End,mEnd,Start,Exp,yy,Step,Expm,xSmall;
	int iStart,iEnd,n;
	double xsize2,ysize2;
	double ix,iy;
	
	if(!b || !r || !d)return;
/*
 	xsize2=b->vAttributes.xLineWidth/2;
 	ysize2=b->vAttributes.yLineWidth/2;
*/

 	xsize2=0;
 	ysize2=0;
	
	MajorIn=y-xMajorIn*mult;
	MajorOut=y+xMajorOut*mult;
	MinorIn=y-xMinorIn*mult;
	MinorOut=y+xMinorOut*mult;
	End=b->xViewMax*(1+b->Small);
	mEnd=b->xMajorStep*(1-b->Small);
	if(b->xLogScale){
	    End=log10(b->xViewMax)*(1+b->Small);
	    iEnd=(int)End;
	    iStart=(int)(b->xViewMinLog-.9999);
	    Start=iStart;
	    yy=(b->yViewMax+b->yViewMin)/2.;
	    if((iEnd-iStart) <= 9){
	        Step=1;
	    }else{
	        Step=4;
	    }
	    
		if(((End-Start)/Step) > b->xMajorLimit){
		    Step=(End-Start)/(double)b->xMajorLimit;
		}
	    
	    
	    for(x=Start;x <= End;x += Step){
	        Exp=pow(10.,x);
	        if(!PlotrPoint(b,&Exp,&yy,&ix,&iy)){
	            rMoveTo(ix-xsize2,MajorIn-ysize2,d);
	            rLineTo(ix-xsize2,MajorOut-ysize2,d);
	            if(b->gridHash){
	            	double xx,yy,y1,y2;
	            	xx=LocalToPixel(ix,d);
	            	y1=LocalToPixel(r->y,d);
	            	y2=LocalToPixel(r->y+r->ysize,d);
	                for(yy=y1;yy<y2;yy += 5){
	            	    riMoveTo((int)xx,(int)yy,d);
	            	    riLineTo((int)xx,(int)(yy+1),d);
	            	}
	            }
	        }
	        if(Step == 1){
	            Expm=Exp+Exp;
	            for(n=0;(n<8) && (log10(Expm) < End);++n,Expm += Exp){
					if(!PlotrPoint(b,&Expm,&yy,&ix,&iy)){
					    rMoveTo(ix-xsize2,MinorIn-ysize2,d);
					    rLineTo(ix-xsize2,MinorOut-ysize2,d);
					}
		    	}
	        }
	    }
	}else{
	
		if(((b->xViewMax-b->xViewMin)/b->xMajorStep) > b->xMajorLimit){
	    	b->xMajorStep=(b->xViewMax-b->xViewMin)/(double)b->xMajorLimit;
		}
	
	    xSmall=1.0e-5*(b->xViewMax-b->xViewMin);
	    End=b->xViewMax+xSmall;
	    mEnd=b->xMajorStep-xSmall;
	    Start=b->xViewMin;
	    
		if(((b->xMajorStep)/b->xMinorStep) > b->xMinorLimit){
	    	b->xMinorStep=(b->xMajorStep)/(double)b->xMinorLimit;
		}
        
        Start=b->xMajorStep*((long long)(b->xViewMin/b->xMajorStep))-b->xMajorStep;
        End=b->xMajorStep*((long long)(b->xViewMax/b->xMajorStep))+b->xMajorStep;

        //fprintf(stderr,"Start %f End %f Step %f\n",Start,End,b->xMajorStep);
        
	    for(x=Start;x <= End;x += b->xMajorStep){
            if(x < (b->xViewMin-xSmall) || x > (b->xViewMax+xSmall))continue;
	            ix=(x-b->xViewMin)*b->xScale+b->xOff;
	            rMoveTo(ix-xsize2,MajorIn-ysize2,d);
	            rLineTo(ix-xsize2,MajorOut-ysize2,d);
	            if(b->gridHash){
	            	double xx,yy,y1,y2;
	            	xx=LocalToPixel(ix,d);
	            	y1=LocalToPixel(r->y,d);
	            	y2=LocalToPixel(r->y+r->ysize,d);
	                for(yy=y1;yy<y2;yy += 5){
	            	    riMoveTo((int)xx,(int)yy,d);
	            	    riLineTo((int)xx,(int)(yy+1),d);
	            	}
	            }
            /*
	        for(xx=x+b->xMinorStep;
	            xx < x+mEnd && xx < End;xx += b->xMinorStep){
	                ix=(xx-b->xViewMin)*b->xScale+b->xOff;
	                rMoveTo(ix-xsize2,MinorIn-ysize2,d);
	                rLineTo(ix-xsize2,MinorOut-ysize2,d);
	        }
            */
  	    }
	}
}
static void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small)
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

static int GridPlotRange(struct uGridPlot *Plot)
{
	struct uLineData *Line;
	double xmin,xmax,ymin,ymax,amax,yminPlus,xminPlus;
	double xx,yy;
	long n,k;
	
	if(!Plot)return 1;
	
	amax=1e60;
	if(Plot->Lines && (Plot->LineCount > 0)){	
		xmin=amax;
		xmax=-amax;
		ymin=amax;
		ymax=-amax;
		yminPlus=amax;
		xminPlus=amax;
		for(n=0;n<Plot->LineCount;++n){
		    Line=Plot->Lines[n];
		    if(!Line || !Line->xData || !Line->yData)continue;
		    for(k=0;k<Line->dCount;++k){
		        xx=Line->xShift+Line->xScale*Line->xData[k];
		        yy=Line->yShift+Line->yScale*Line->yData[k];
			    if(!Plot->xAutoMinimum && (xx < Plot->xSetMinimum))continue;
			    if(!Plot->yAutoMinimum && (yy < Plot->ySetMinimum))continue;
			    if(!Plot->xAutoMaximum && (xx > Plot->xSetMaximum))continue;
			    if(!Plot->yAutoMaximum && (yy > Plot->ySetMaximum))continue;
			    if(xx < xmin)xmin=xx;
			    if(yy < ymin)ymin=yy;
			    if(xx > xmax)xmax=xx;
			    if(yy > ymax)ymax=yy;
			    if(xx > 0 && xx < xminPlus)xminPlus=xx;
			    if(yy > 0 && yy < yminPlus)yminPlus=yy;
		    }
		    if(Line->li.integrate){
		        WarningPrint("Current Frame : %ld Line Number : %ld  Line Name : %s Line Integral : %g\n",
		          Line->li.CurrentFrame,n+1,Line->save,Line->li.integral);
		    }
		}
	}else{
		xmin=0;
		xmax=1;
		ymin=0;
		ymax=1;
		yminPlus=.1;
		xminPlus=.1;
	}
	
	if(Plot->xAutoMinimum){
		if(xmin < xmax)Plot->xMinimum=xmin;
	}else{
		Plot->xMinimum=Plot->xSetMinimum;
	}
	
	
	if(Plot->xAutoMaximum){
		if(xmin < xmax)Plot->xMaximum=xmax;
	}else{
		Plot->xMaximum=Plot->xSetMaximum;
	}
	
	if(Plot->xLogScale){	
		if(Plot->xMaximum <= 0){
			Plot->xMaximum=1.0;
		}		
		if(Plot->xMinimum <= 0){
		    if(xminPlus < amax){
				Plot->xMinimum=xminPlus;
			}else{
				Plot->xMinimum=0.09*Plot->xMaximum;
			}
		}
		if(Plot->xMinimum >= 0.09*Plot->xMaximum){
			Plot->xMinimum=0.09*Plot->xMaximum;
		}
		Plot->xMinimum *= 0.9;
		Plot->xMaximum *= 1.1;
	}	
	
	if(Plot->yAutoMinimum){
		if(ymin <= ymax)Plot->yMinimum=ymin;
	}else{
		Plot->yMinimum=Plot->ySetMinimum;
	}
	
	if(Plot->yAutoMaximum){
		if(ymin <= ymax)Plot->yMaximum=ymax;
	}else{
		Plot->yMaximum=Plot->ySetMaximum;
	}
	
	if(Plot->yLogScale){	
		if(Plot->yMaximum <= 0){
			Plot->yMaximum=1.0;
		}		
		if(Plot->yMinimum <= 0){
		    if(yminPlus < amax){
				Plot->yMinimum=yminPlus;
			}else{
				Plot->yMinimum=0.09*Plot->yMaximum;
			}
		}
				
		if(Plot->yMinimum >= 0.09*Plot->yMaximum){
			Plot->yMinimum=0.09*Plot->yMaximum;
		}
		Plot->yMinimum *= 0.9;
		Plot->yMaximum *= 1.1;
	}	
	
	return 0;
}
int GridPlotScale(struct uGridPlot *b)
{
	double xmax,xmin,ymax,ymin;
	double xLarge,xSmall,yLarge,ySmall;
	rRect r;
	
	
	if(!b)return 1;
	
	if(b->xAutoMinimum || b->xAutoMaximum || (b->xSetMinimum >= b->xSetMaximum)){
	    xmin=b->xMinimum;
	    xmax=b->xMaximum;
	    if(xmax <= xmin){
			xmin=xmax-1.0-0.001*fabs(xmax);
			xmax=xmax+1.0+0.001*fabs(xmax);
	    }
	    if(!b->xLogScale){
			GridPlotNeat(&xmin,&xmax,&xLarge,&xSmall);
			b->xMajorStep=xLarge;
			b->xMinorStep=xSmall;
	    }
	    b->xViewMin=xmin;
	    b->xViewMax=xmax;
	}else{
	    b->xViewMin=b->xSetMinimum;
	    b->xViewMax=b->xSetMaximum;
	}

	if(b->yAutoMinimum || b->yAutoMaximum || (b->ySetMinimum >= b->ySetMaximum)){
	    ymin=b->yMinimum;
	    ymax=b->yMaximum;
	    if(ymax <= ymin){
			ymin=ymax-1.0-0.001*fabs(ymax);
			ymax=ymax+1.0+0.001*fabs(ymax);
	    }
	    if(!b->yLogScale){
			GridPlotNeat(&ymin,&ymax,&yLarge,&ySmall);
			b->yMajorStep=yLarge;
			b->yMinorStep=ySmall;
	    }
	    b->yViewMin=ymin;
	    b->yViewMax=ymax;
	}else{
	    b->yViewMin=b->ySetMinimum;
	    b->yViewMax=b->ySetMaximum;
	}
	r= b->box;
	
	b->xOff=r.x;
	b->yOff=r.y;

	b->xScale=(r.xsize)/(b->xViewMax-b->xViewMin);
	b->yScale=(r.ysize)/(b->yViewMax-b->yViewMin);
	if(b->xLogScale){
	    b->xViewMinLog=log10(b->xViewMin);
	    b->xScaleLog=(r.xsize)/(log10(b->xViewMax)-b->xViewMinLog);
	}
	if(b->yLogScale){
	    b->yViewMinLog=log10(b->yViewMin);
	    b->yScaleLog=(r.ysize)/(log10(b->yViewMax)-b->yViewMinLog);
	}
	
	b->dxLim=1e-10*(b->xViewMax-b->xViewMin);
	b->dyLim=1e-10*(b->yViewMax-b->yViewMin);
	
	return 0;
}
#ifdef GRIDPLOTIN
static int GridPlotSetFrame(struct DObject *Object,long CurrentFrame)
{
	struct uGridPlot *b=(struct uGridPlot *)Object;
	double xmin,xmax,ymin,ymax,amax;
	int n;
	
	if(!b)return 1;
	
	if(b->Lines && (b->LineCount > 0)){	
	    struct uLineData *Line;
		amax=1e60;
		xmin=amax;
		xmax=-amax;
		ymin=amax;
		ymax=-amax;
		for(n=0;n<b->LineCount;++n){
		    Line=b->Lines[n];
			/* LineDataSetFrame((struct DObject *)Line,CurrentFrame); */
		    if(Line->xmin < xmin)xmin=Line->xmin;
		    if(Line->ymin < ymin)ymin=Line->ymin;
		    if(Line->xmax > xmax)xmax=Line->xmax;
		    if(Line->ymax > ymax)ymax=Line->ymax;
		}
	}else{
		xmin=0;
		xmax=1;
		ymin=0;
		ymax=1;
	}
	b->xMinimum=xmin;
	b->xMaximum=xmax;
	b->yMinimum=ymin;
	b->yMaximum=ymax;
	return 0;
}
#endif
static void GridPlotKill(struct DObject *Object)
{
	struct uGridPlot *b=(struct uGridPlot *)Object;
	int n;
	
	if(!b)return;
	
	if(b->Lines && (b->LineCount > 0)){	
		for(n=0;n<b->LineCount;++n){
			struct uLineData *Line;
			Line=b->Lines[n];
			if(!Line || !Line->Kill)continue;
			(*Line->Kill)((DObjPtr)Line);
		}
	}	
	
	
	if(b->Lines)cFree((char *)b->Lines);
	b->Lines=NULL;
	
	if(b->xBoxes)cFree((char *)b->xBoxes);
	b->xBoxes=NULL;
	
	if(b->yBoxes)cFree((char *)b->yBoxes);
	b->yBoxes=NULL;
			
	ObjectKill(Object);
		
}
struct DObject *ObjectCreate(char *name,int type,rRect *box,long size)
{
	struct DObject *Object;
	
	if(!name || !box )return NULL;
	
	Object=(struct DObject *)cMalloc(size,8213);
	if(!Object)return NULL;
	zerol((char *)Object,size);
	
	Object->box  = *box;
	Object->type = type;	
	Object->size = size;	
						
	mstrncpy(Object->name,name,64);
	
	Object->Selected=TRUE;
	
	/* Object->oStatus=StatusNew; */
	
	Object->ItWas = -1;
	
	ObjectSet(Object);
	
	return Object;
}
void ObjectKill(struct DObject *Object)
{
	if(!Object)return;
	
	zerol((char *)Object,Object->size);
	cFree((char *)Object);
}
static int ObjectDraw(struct DObject *Object,DOListPtr l)
{
	if(!Object || !l)return 1;
	
	return 0;
	
}
int ObjectSet(DObjPtr Object)
{
	if(!Object)return 1;
	
	Object->Kill=ObjectKill;

	Object->Draw=ObjectDraw;
/*
	
	Object->GetAttributes=ObjectAttributes;

	Object->GetFrameCount=ObjectFramCount;
	
	Object->SetFrame=ObjectSetFrame;
	
	Object->FindObject=ObjectFindObject;
	
	Object->DoubleClick=ObjectDoubleClick;
	
	Object->Update=ObjectUpdate;
	
	Object->Duplicate=ObjectDuplicate;
	
	Object->Attribute=ObjectAttribute;
	
	Object->DeSelectOne=ObjectDeSelectOne;
	
	Object->SelectOne=ObjectSelectOne;

	Object->SelectedPoint=ObjectSelectedPoint;
	
	Object->SetSelectedRegion=ObjectSetSelectedRegion;
	
	Object->Message=ObjectGetMessage;
	
	Object->CheckStretch=ObjectCheckStretch;
	
	Object->Stretch=ObjectStretch;
	
	Object->Move=ObjectMove;	

	Object->KillSelected=ObjectKillSelected;
	
	Object->GetFontSelected=ObjectGetFontSelected;
	
	Object->SetFontSelected=ObjectSetFontSelected;
	
	Object->Copy=ObjectCopy;
	
	Object->DoInformation=ObjectDoInformation;
	
	Object->DrawSelected=ObjectDrawSelected;
*/
	return 0;
}

static struct DObject *GridPlotCreate(char *name,rRect *box,struct uAttributes *Attributes)
{
	uGridPlotPtr b;
	struct DObject *Object;
	
	
	if(!name || !box || !Attributes)return NULL;
	
    Object=ObjectCreate(name,GridPlotType,box,sizeof(struct uGridPlot));
    if(!Object)return NULL;    
    
    Object->Attributes= *Attributes;    
		
    b=(uGridPlotPtr)Object;
	        
	uGridPlotInit(b,Attributes);
		   	
	return Object;
}
int uGridPlotInit(uGridPlotPtr b,struct uAttributes *Attributes)
{
	if(!b || !Attributes)return 1;

	b->xMinimum=0;
	b->yMinimum=0;
	b->xMaximum=1;
	b->yMaximum=1;

	b->xSetMinimum=0;
	b->ySetMinimum=0;
	b->xSetMaximum=1;
	b->ySetMaximum=1;

	b->rMajorOut=b->tMajorOut=b->yMajorOut=b->xMajorOut=.1;
	b->rMinorOut=b->tMinorOut=b->yMinorOut=b->xMinorOut=.05;
	b->xAutoMinimum=b->yAutoMinimum=TRUE;
	b->xAutoMaximum=b->yAutoMaximum=TRUE;
	

	b->xMajorLimit=25;
	b->yMajorLimit=25;
	
	b->xMinorLimit=20;
	b->yMinorLimit=20;
	
	b->rAxis=b->tAxis=TRUE;
	b->lAxis=b->bAxis=TRUE;
	b->rAxisNumber=b->tAxisNumber=TRUE;
	b->lAxisNumber=b->bAxisNumber=TRUE;
	b->xLogScale=b->yLogScale=FALSE;		     
	b->xMajorIn=b->yMajorIn=b->rMajorIn=b->tMajorIn=0;
	b->xMinorIn=b->yMinorIn=b->rMinorIn=b->tMinorIn=0;
	b->xAttributes= *Attributes;
	b->yAttributes= *Attributes;
	
 	b->pSelected=TRUE;
	b->xSelected=TRUE;
	b->ySelected=TRUE;
	b->Small=1.0e-6;

	GridPlotSet(b);
	
	return 0;
}
int uPutAttributes(struct uAttributes *Aout,struct uAttributes *Ain,int Flag)
{
	if(!Aout || !Ain)return 1;
	
	if(Flag & sForeColor){
	    Aout->nSolidFore=Ain->nSolidFore;
	}
	if(Flag & sBackColor){
	    Aout->nSolidBack=Ain->nSolidBack;
	}
	if(Flag & sPattern){
	    Aout->SolidPattern=Ain->SolidPattern;
	}
	if(Flag & lForeColor){
	    Aout->nLineFore=Ain->nLineFore;
	}
	if(Flag & lBackColor){
	    Aout->nLineBack=Ain->nLineBack;
	}
	if(Flag & lPattern){
	    Aout->LinePattern=Ain->LinePattern;
	}
	
	if(Flag & lSymbol){
	    Aout->LineSymbol=Ain->LineSymbol;
	}
	
	if(Flag & lLine){
	    mstrncpy((char *)&Aout->LineDash,(char *)&Ain->LineDash,16);
	    Aout->DashFlag=Ain->DashFlag;
	    Aout->LineDashNumber=Ain->LineDashNumber;
	}
	
	if(Flag & lWidth){
	    Aout->xLineWidth=Ain->xLineWidth;
	    Aout->yLineWidth=Ain->yLineWidth;
	}
	
	if(Flag & lAnimate){
	    Aout->Animation=Ain->Animation;
	    Aout->AnimationStart=Ain->AnimationStart;
	    Aout->AnimationStep=Ain->AnimationStep;
	    Aout->AnimationJust=Ain->AnimationJust;
	    mstrncpy((char *)&Aout->AnimationFormat,(char *)&Ain->AnimationFormat,12);
	}
	return 0;
	
}
static void GridPlotAttribute(DObjPtr o,struct uAttributes *Attributes,int Flag)
{
	uGridPlotPtr b=(uGridPlotPtr)o;
	long n;
	
	if(!b || !Attributes)return;
	
	if(b->pSelected)uPutAttributes(&b->Attributes,Attributes,Flag);
	if(b->xSelected)uPutAttributes(&b->xAttributes,Attributes,Flag);
	if(b->ySelected)uPutAttributes(&b->yAttributes,Attributes,Flag);
	
	if(b->Lines && (b->LineCount > 0)){	
		for(n=0;n<b->LineCount;++n){
			struct uLineData *Line;
			Line=b->Lines[n];
			if(!Line)continue;
		    if(Line->Selected)uPutAttributes(&Line->Attributes,Attributes,Flag);
		}
	}
		
}

int GridPlotSet(uGridPlotPtr Object)
{
	if(!Object)return 1;
	
    Object->Draw=GridPlotDraw;
	
    Object->Kill=GridPlotKill;
	
    Object->Attribute=GridPlotAttribute;
/*
    
    
    Object->FindObject=GridPlotFindObject;
    
    Object->DoubleClick=GridPlotDoubleClick;
    
    Object->Update=GridPlotUpdate;
    
    Object->SetFrame=GridPlotSetFrame;
    
    Object->GetFrameCount=GridPlotFrameCount;
    
    Object->Attribute=GridPlotAttribute;
    
	Object->SelectedPoint=GridPlotSelectedPoint;

  	Object->SelectOne=GridPlotSelectOne;
  	
	Object->DeSelectOne=GridPlotDeSelectOne;
  
	Object->Duplicate=GridPlotDuplicate;
	
	Object->GetAttributes=GridPlotGetAttributes;

	Object->SetSelectedRegion=GridPlotSetSelectedRegion;
	
	Object->Message=GridPlotMessage;	
	
	Object->DrawSelected=GridPlotDrawSelected;
	
	Object->Move=GridPlotMove;
	
	Object->KillSelected=GridPlotKillSelected;
	
	Object->DoInformation=GridPlotDoInformation;	
	
	Object->GetFontSelected=GridPlotGetFontSelected;	
	
	Object->SetFontSelected=GridPlotSetFontSelected;	
	
	Object->Copy=GridPlotCopy;
	
	Object->ReadWrite=uGridPlotWrite;
*/
	return 0;
}
