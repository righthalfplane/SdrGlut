#ifndef LINEDATA
#define LINEDATA

#include "paletteDraw.h"
#include "SetFrameData.h"

struct linedata{
    char sourceName[256];
   	int type;
    double x1;
    double y1;
    double z1;
    double x2;
    double y2;
    double z2;
    long FirstFrame;
    long LastFrame;
    double FirstTime;
    double LastTime;
    long CurrentFrame;
    long FrameCount;
    double *xData;
    double *yData;
    double rymin;
    double rymax;
    long dCount;
    
    double SampleRadius;
    long SampleSteps;
    
    int LineSourceMode;
    int LineSelectionMode;
    
    int replaceData;
    struct CPlot *Plot;
    int Polynomial;
    int PolynomialOrder;
    int PolynomialPrint;
    
    char pioName[256];
    long pioIndex;
    
    char pioName2[256];
    long pioIndex2;
    
    int PointLineCount;
    double *x;
    double *y;
    double *z;
    double *v;
    
    double integral;
    int integrate;
};


struct areadata{
	int  (*uDraw)(void *info,double *x,double *y,double *z,double *v,long ncell);
	void *info;
	struct PlotAttributes pa;
   	struct paletteDraw pd;
    char targetName[256];
    double *data;
    double xminArea;
    double yminArea;
    double zminArea;
    double vminArea;
    double xmaxArea;
    double ymaxArea;
    double zmaxArea;
    double vmaxArea;
    long CurrentFrame;
    char pioName[256];
    double pioTime;
    long pioIndex;
    long xsize;
    long ysize;
    long zsize;
    int type;
    
    double x1;
    double y1;
    double z1;
    double x2;
    double y2;
    double z2;
    double x3;
    double y3;
    double z3;
    
    double gmin;
    double gmax;
    double gloc;

};



#define LINEDATA_LINE_DATA			1
#define LINEDATA_POINT_DATA			2
#define LINEDATA_LIST_DATA			4
#define AREADATA_GRID_DATA	    	8
#define AREADATA_AREA_DATA	    	9
#define GENERAL_OFFSET_DATA	    	10
#define GENERAL_PLANE_DATA	    	11
#define LINEDATA_CELL_DATA			12
#define LINEDATA_HISTOGRAM_DATA		14
#define LINEDATA_STREAMLINE_DATA	15
#define LINEDATA_BHANGMETER_DATA	16

#define LINEDATA_SELECTION_MODE_XY		0
#define LINEDATA_SELECTION_MODE_RT		1

#define LINEDATA_DISTANCE_MODE		0
#define LINEDATA_CROSSPLOT_MODE		1


#endif
