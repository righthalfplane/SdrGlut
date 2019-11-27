#ifndef SETFRAMEDATA
#define SETFRAMEDATA

#include "paletteDraw.h"
#include "Limits2.h"

#define FRAME_DATA_FLOAT			1
#define FRAME_DATA_RASTER			2
#define FRAME_DATA_RGB				4
#define FRAME_DATA_PICT				8
#define FRAME_DATA_FLOAT_RASTER		3

#define FRAME_RANGE_NONE	0
#define FRAME_RANGE_LIMITS	1
#define FRAME_RANGE_RASTER	2

#define FRAME_SAMPLE_SIMPLE			0
#define FRAME_SAMPLE_INTERPLOATE	1
#define FRAME_SAMPLE_ANITALIAS		2

#define FRAME_BOX_SAMPLE			0
#define FRAME_BOX_FILL				1

struct PlotStreamData{
     int streamsReturned;
     int streamGradients;
     long streamSteps;
     long streamPasses;
     int streamOn;
};


struct PlotVectorData{
    char vectorxName[32];
    char vectoryName[32];
    char vectorzName[32];
    double arrowLength;
    double arrowAngle;
    double vspace;
    double vfac;
    double vmax;
    int DrawVectors;    
    int arrowHeads;
};

struct PlotStreamlineData{
    double speedCutoff,error;
	double xStart,yStart,xEnd,yEnd;
    int num;
    int dir;
    int maxCells;
    int maxSteps;
    int doGrad;
    int DrawStreamlines;    
};

struct PlotPlaneData{
    double Plane_dmin;
    double Plane_dmax;
    double Plane_xmin;
    double Plane_xmax;
    double Plane_xloc;
    double Plane_ymin;
    double Plane_ymax;
    double Plane_yloc;
    double Plane_zmin;
    double Plane_zmax;
    double Plane_zloc;
    
    double Plane_gmin;
    double Plane_gmax;
    double Plane_gloc;
    double Plane_gx1;
    double Plane_gy1;
    double Plane_gz1;
    double Plane_gx2;
    double Plane_gy2;
    double Plane_gz2;
    double Plane_gx3;
    double Plane_gy3;
    double Plane_gz3;
    
    long Plane_xstep;
    long Plane_xcount;
    long Plane_ystep;
    long Plane_ycount;
    long Plane_zstep;
    long Plane_zcount;
    long Plane_gstep;
    long Plane_gcount;
    
    int Plane_type;
};

struct PlotRangeData{
    double AspectRatio;
    
    double xminData;
    double xmaxData;
    double yminData;
    double ymaxData;
    double zminData;
    double zmaxData;

    double xScale;
    double yScale;
  
    long ixminData;
    long ixmaxData;
    long iyminData;
    long iymaxData;
    long izminData;
    long izmaxData;

    long ixmax;
    long iymax;
    long izmax;
    
    int xminSet;
    int xmaxSet;
    int yminSet;
    int ymaxSet;
    int zminSet;
    int zmaxSet;
    
    
    int MaintainAspectRatio;
    
    int receiveRange;
       
};

struct PlotRasterData{
    double xScale;
    double yScale;
    int sampleType;
    int plotType;
    int boxType;
    long xsize;
    long ysize;
    uRect box;
};


struct PlotRangeRange{
	double xmin;
	double ymin;
	double zmin;
	double xmax;
	double ymax;
	double zmax;
	long xsize;
	long ysize;
	long zsize;
};

struct PlotAttributes{
    long BlockSize;
    int DrawZonesColor;
    int flagGradients;
    int DrawZones;
    int ZoneLevel;
    int xReflect;
};

struct SetFrameData{
   struct PlotStreamData streamline3d;
   struct PlotStreamData streamline2d;
   struct PlotStreamlineData streamline;
   struct PlotRangeRange rangeData;
   struct PlotRasterData raster;
   struct PlotVectorData vector;
   struct PlotRangeData range;
   struct PlotPlaneData plane;
   struct PlotAttributes pa;
   struct paletteDraw pd;
   struct LIMITS limits;
   long long numcell;
   long CurrentFrame;
   char pioName[64];
   double pioTime;
   int pioIndex;
   int typePlotRange;
   long SetFrame;
   void *data;
   void *data2;
   long xsize;
   long ysize;
   long zsize;
   long length; 
   char *name;
   char *pal;
   int type;
};


#endif

