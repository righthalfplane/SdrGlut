/*
 *  GridPlot.h
 *  
 *
 *  Created by Dale Ranta on 10/18/11.
 *  Copyright 2011 Dale Ranta. All rights reserved.
 *
 */
#ifndef _GridPlot_
#define _GridPlot_

#include "Utilities.h"
#include "uAttributes.h"
#include "Linedata.h"


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
#define AnyType			0
#define GridPlotType	1
#define LineDataType	2
#define ImagePlotType	3
#define TruePlotType	4
#define GroupsType		5
#define AlphaType		6
#define LineType		7
#define BoxType			8
#define OvalType		9
#define RoundRectType	10
#define GridPlotLine	11
#define PIO2D_PlotType	12
#define TekPlotType		14

#define DISPLAY_PIXELS   0
#define DISPLAY_INCHES   1


#define sForeColor    1
#define sBackColor    2
#define sPattern      4
#define lForeColor    8
#define lBackColor   16
#define lPattern     32
#define lWidth       64
#define lSymbol     128
#define lLine       256
#define lAnimate    512
#define tFont      1024


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct iRectStruct{
    int type;
    uRect box;
    unsigned char *data;
} iRect;


typedef struct rPointStruct2{
	double x,y;
} rPoint;

typedef struct rRectStruct{
    double x,y;
    double xsize,ysize;
} rRect;

extern struct DOList *ldummy777;

struct DObject {
/* Start of Object variables */
	struct uAttributes Attributes;
    char name[64];
	rRect box;
	int type;
	int AngleFlag;
	int FlipFlag;
	
	struct DOList *l;
	long size;	
	
    double xDup,yDup;
	int Selected;
	int oStatus;

    long ItWas;
    	
   /* struct DObject *((*ReadWrite)(struct DObject *o,FILE8 *inOut,struct DOList *l)); */
	void (*Kill)(struct DObject *);
	int (*Draw)(struct DObject *,struct DOList *l);
	int (*GetFrameCount)(struct DObject *,long *FrameCount);
	int (*SetFrame)(struct DObject *,long FrameCount);	
	struct DObject *(*FindObject)(struct DObject *Object,rPoint *r);	
	struct DObject *(*DoubleClick)(struct DObject *Object,rPoint *r);	
	int (*Update)(struct DObject *);
	struct uAttributes *(*GetAttributes)(struct DObject *Object);	
	void (*Attribute)(struct DObject *,struct uAttributes *vAttributes,int Flag);
    struct DObject *((*Duplicate)(struct DObject *,int iDup,double *xOff,double *yOff));
	void (*DeSelectOne)(struct DObject *,rPoint *r);
	void (*SelectOne)(struct DObject *,rPoint *r);
	int (*SelectedPoint)(struct DObject *,rPoint *ClickPoint);
	int (*SetSelectedRegion)(struct DObject *,struct DOList *l);
	int (*Message)(struct DObject *,long MessageType,void *MessageData);
	int (*DrawSelected)(struct DObject *,struct DOList *l);
 	/* int (*CheckStretch)(uPoint pt,struct DObject *,struct DOList *l); */
 	/* struct DObject * (*Stretch)(uPoint pt,struct DObject *,struct DOList *l); */
 	struct DObject * (*Move)(struct DObject *,double *xOff,double *yOff);
 	struct DObject * (*KillSelected)(struct DObject *);
	char *((*Copy)(struct DObject *,long *Length));
    struct DObject *((*DoInformation)(struct DObject *));
    struct DObject *((*DoRotateDirection)(struct DObject *,int direct));    
    struct DObject *((*DoFlipDirection)(struct DObject *,int direct));
 	void (*GetFontSelected)(struct DObject *);
 	void (*SetFontSelected)(struct DObject *);
    struct DObject *((*SetPalette)(struct DObject *,unsigned char *palette));
/* End of Object variables */
};

typedef struct DObject *DObjPtr;



struct uGridPlot{
/* Start of Object variables */
	struct uAttributes Attributes;
    char name[64];
	rRect box;
	int type;
	int AngleFlag;
	int FlipFlag;
	
	struct DOList *l;
	long size;	
	
    double xDup,yDup;
	int Selected;
	int oStatus;

    long ItWas;
    	
    /* struct DObject *((*ReadWrite)(struct DObject *o,FILE8 *inOut,struct DOList *l)); */
	void (*Kill)(struct DObject *);
	int (*Draw)(struct DObject *,struct DOList *l);
	int (*GetFrameCount)(struct DObject *,long *FrameCount);
	int (*SetFrame)(struct DObject *,long FrameCount);	
	struct DObject *(*FindObject)(struct DObject *Object,rPoint *r);	
	struct DObject *(*DoubleClick)(struct DObject *Object,rPoint *r);	
	int (*Update)(struct DObject *);
	struct uAttributes *(*GetAttributes)(struct DObject *Object);	
	void (*Attribute)(struct DObject *,struct uAttributes *vAttributes,int Flag);
    struct DObject *((*Duplicate)(struct DObject *,int iDup,double *xOff,double *yOff));
	void (*DeSelectOne)(struct DObject *,rPoint *r);
	void (*SelectOne)(struct DObject *,rPoint *r);
	int (*SelectedPoint)(struct DObject *,rPoint *ClickPoint);
	int (*SetSelectedRegion)(struct DObject *,struct DOList *l);
	int (*Message)(struct DObject *,long MessageType,void *MessageData);
	int (*DrawSelected)(struct DObject *,struct DOList *l);
 	/* int (*CheckStretch)(uPoint pt,struct DObject *,struct DOList *l); */
 	/* struct DObject * (*Stretch)(uPoint pt,struct DObject *,struct DOList *l); */
 	struct DObject * (*Move)(struct DObject *,double *xOff,double *yOff);
 	struct DObject * (*KillSelected)(struct DObject *);
	char *((*Copy)(struct DObject *,long *Length));
    struct DObject *((*DoInformation)(struct DObject *));
    struct DObject *((*DoRotateDirection)(struct DObject *,int direct));    
    struct DObject *((*DoFlipDirection)(struct DObject *,int direct));
 	void (*GetFontSelected)(struct DObject *);
 	void (*SetFontSelected)(struct DObject *);
    struct DObject *((*SetPalette)(struct DObject *,unsigned char *palette));
/* End of Object variables */


    double xMinimum;
    double yMinimum;
    double xMaximum;
    double yMaximum;
    
    double xMajorIn,xMajorOut;
    double yMajorIn,yMajorOut;
    double rMajorIn,rMajorOut;
    double tMajorIn,tMajorOut;
    double xMinorIn,xMinorOut;
    double yMinorIn,yMinorOut;
    double rMinorIn,rMinorOut;
    double tMinorIn,tMinorOut;
    int lAxis;			     /* Left Axis Flag		*/
    int bAxis;			     /* Bottom Axis Flag	*/
    int rAxis;			     /* Right Axis Flag		*/
    int tAxis;			     /* Top Axis Flag		*/
    int lAxisNumber;		     /* Left Axis Flag		*/
    int bAxisNumber;		     /* Bottom Axis Flag	*/
    int rAxisNumber;		     /* Right Axis Flag		*/
    int tAxisNumber;		     /* Top Axis Flag		*/
    int xGrid;			     /* X Axis Cross Hash	*/
    int yGrid;			     /* Y Axis Cross Hash	*/
    int xLogScale;		     /* X Axis Log Flag		*/
    int yLogScale;		     /* Y Axis Log Flag		*/
    double xViewMin,xViewMax;        /* Window X Limits		*/
    double yViewMin,yViewMax;        /* Window Y Limits		*/
    double xViewMinLog,yViewMinLog;  /* Log10 of Limits		*/
    double xScale,yScale;            /* Window Scale Factors	*/
    double xScaleLog,yScaleLog;      /* Window Log of Scales	*/
    double xOff,yOff;                /* Window Off Sets		*/
    double xMajorStep,xMinorStep;    /* X Grid Step Sizes	*/
    double yMajorStep,yMinorStep;    /* Y Grid Step Sizes	*/
    double dxLim,dyLim;
    double Small;
    
    
	struct uAttributes xAttributes;
	struct uAttributes yAttributes;
	
    struct uLineData **Lines;
    long LineCount;
    long LineMax;
    
    rRect xBoxeSelect;
    rRect yBoxeSelect;
    
    rPoint xBoxOffset;    
    rPoint yBoxOffset;
    rPoint yLegendOffset;
    
    rRect *xBoxes;
    rRect *yBoxes;
    int xCount;
    int yCount;
    
	int xSelected;
	int ySelected;
    
	int pSelected;
	int lSelected;
	
	int gridHash;
    
	int boxLegend;
    
    int xAutoMinimum;
    int yAutoMinimum;
    int xAutoMaximum;
    int yAutoMaximum;
    int xMajorLimit;
    int xMinorLimit;
    int yMajorLimit;
    int yMinorLimit;
    
    double xSetMinimum;
    double ySetMinimum;
    double xSetMaximum;
    double ySetMaximum;
    
};

typedef struct uGridPlot *uGridPlotPtr;

struct uAlpha {
/* Start of Object variables */
	struct uAttributes Attributes;
    char name[64];
	rRect box;
	int type;
	int AngleFlag;
	int FlipFlag;
	
	struct DOList *l;
	long size;	
	
    double xDup,yDup;
	int Selected;
	int oStatus;

    long ItWas;
    	
   /* struct DObject *((*ReadWrite)(struct DObject *o,FILE8 *inOut,struct DOList *l)); */
	void (*Kill)(struct DObject *);
	int (*Draw)(struct DObject *,struct DOList *l);
	int (*GetFrameCount)(struct DObject *,long *FrameCount);
	int (*SetFrame)(struct DObject *,long FrameCount);	
	struct DObject *(*FindObject)(struct DObject *Object,rPoint *r);	
	struct DObject *(*DoubleClick)(struct DObject *Object,rPoint *r);	
	int (*Update)(struct DObject *);
	struct uAttributes *(*GetAttributes)(struct DObject *Object);	
	void (*Attribute)(struct DObject *,struct uAttributes *vAttributes,int Flag);
    struct DObject *((*Duplicate)(struct DObject *,int iDup,double *xOff,double *yOff));
	void (*DeSelectOne)(struct DObject *,rPoint *r);
	void (*SelectOne)(struct DObject *,rPoint *r);
	int (*SelectedPoint)(struct DObject *,rPoint *ClickPoint);
	int (*SetSelectedRegion)(struct DObject *,struct DOList *l);
	int (*Message)(struct DObject *,long MessageType,void *MessageData);
	int (*DrawSelected)(struct DObject *,struct DOList *l);
 	/*int (*CheckStretch)(uPoint pt,struct DObject *,struct DOList *l); */
 	/* struct DObject * (*Stretch)(uPoint pt,struct DObject *,struct DOList *l); */
 	struct DObject * (*Move)(struct DObject *,double *xOff,double *yOff);
 	struct DObject * (*KillSelected)(struct DObject *);
	char *((*Copy)(struct DObject *,long *Length));
    struct DObject *((*DoInformation)(struct DObject *));
    struct DObject *((*DoRotateDirection)(struct DObject *,int direct));    
    struct DObject *((*DoFlipDirection)(struct DObject *,int direct));
 	void (*GetFontSelected)(struct DObject *);
 	void (*SetFontSelected)(struct DObject *);
    struct DObject *((*SetPalette)(struct DObject *,unsigned char *palette));
/* End of Object variables */
	struct FileInfo2 *Files;	
	char *fileText;
	long fileLength;
	int AlphaNumber;
	double Angle;
	double scale;
	char save[512];
	int n;
};

typedef struct uAlpha *uAlphaPtr;


struct DOList{
	struct uAttributes Attributes;
	DObjPtr *oOld;
	long oMaximun;
	DObjPtr *oCurrent;
	long oCount;
    struct screenData s;
    long ObjectIDCount;
	int displayType;
	unsigned char *data;
	long xsize;
	long ysize;
	long dpi;
	int raster;
	long length;
	char palette[256*3];
	char palname[256];
	unsigned char red,green,blue;
	
	int myIcon;
	
	uAlphaPtr Alpha;
	/* uEditPtr Edit; */
	int AlphaNumber;
	
	rRect *Boxes;
    long rCountCount;
    long rCountMax;
    
	uRect *Stretch;
    long stretchCount;
    long stretchMax;
    
	float *itemList;
    
    double xDuplicate,yDuplicate;
    int DuplicateFlag;
    
    long CurrentFrame;
    
    char LinePatterns[8][16];
    int NumberOfPatterns;
    
	iRect Picts[40];
    
	int nLineFore;
	int nLineBack;
	
	uRect PrintRect;
	int PrintFlag;

	struct uAttributes *lAttributes;
	long lineAttributesCount;
	long lineAttributesMax;

};

typedef struct DOList *DOListPtr;

struct uLineData{
/* Start of Object variables */
	struct uAttributes Attributes;
    char name[64];
	rRect box;
	int type;
	int AngleFlag;
	int FlipFlag;
	
	struct DOList *l;
	long size;	
	
    double xDup,yDup;
	int Selected;
	int oStatus;

    long ItWas;
    	
    /* struct DObject *((*ReadWrite)(struct DObject *o,FILE8 *inOut,struct DOList *l)); */
	void (*Kill)(struct DObject *);
	int (*Draw)(struct DObject *,struct DOList *l);
	int (*GetFrameCount)(struct DObject *,long *FrameCount);
	int (*SetFrame)(struct DObject *,long FrameCount);	
	struct DObject *(*FindObject)(struct DObject *Object,rPoint *r);	
	struct DObject *(*DoubleClick)(struct DObject *Object,rPoint *r);	
	int (*Update)(struct DObject *);
	struct uAttributes *(*GetAttributes)(struct DObject *Object);	
	void (*Attribute)(struct DObject *,struct uAttributes *vAttributes,int Flag);
    struct DObject *((*Duplicate)(struct DObject *,int iDup,double *xOff,double *yOff));
	void (*DeSelectOne)(struct DObject *,rPoint *r);
	void (*SelectOne)(struct DObject *,rPoint *r);
	int (*SelectedPoint)(struct DObject *,rPoint *ClickPoint);
	int (*SetSelectedRegion)(struct DObject *,struct DOList *l);
	int (*Message)(struct DObject *,long MessageType,void *MessageData);
	int (*DrawSelected)(struct DObject *,struct DOList *l);
 	/* int (*CheckStretch)(uPoint pt,struct DObject *,struct DOList *l); */
 	/* struct DObject * (*Stretch)(uPoint pt,struct DObject *,struct DOList *l); */
 	struct DObject * (*Move)(struct DObject *,double *xOff,double *yOff);
 	struct DObject * (*KillSelected)(struct DObject *);
	char *((*Copy)(struct DObject *,long *Length));
    struct DObject *((*DoInformation)(struct DObject *));
    struct DObject *((*DoRotateDirection)(struct DObject *,int direct));    
    struct DObject *((*DoFlipDirection)(struct DObject *,int direct));
 	void (*GetFontSelected)(struct DObject *);
 	void (*SetFontSelected)(struct DObject *);
    struct DObject *((*SetPalette)(struct DObject *,unsigned char *palette));
/* End of Object variables */
    char save[64];
	struct linedata li;
    double *xData;
    double *yData;
    long dCount;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double xShift;
    double yShift;
    double xScale;
    double yScale;
};

typedef struct uLineData *uLineDataPtr;

struct DObject *ObjectCreate(char *name,int type,rRect *box,long size);

uGridPlotPtr DOGridPlotCreate(DOListPtr l);
uLineDataPtr DOLineDataCreate(struct linedata *li,struct uAttributes *Attributes);
int DODraw(DOListPtr l,long CurrentFrame);
int DOListDelete(DOListPtr l);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */


#endif
