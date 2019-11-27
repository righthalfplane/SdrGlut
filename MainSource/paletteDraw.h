#ifndef _PALETTEDRAW_
#define _PALETTEDRAW_
#include "ulibTypes.h"


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
    
#define RASTER_8      8
#define RASTER_24    24
#define RASTER_32    32
#define RASTER_64    64

struct screenData{
    uRect viewRect;
    int ScreenBits;
    unsigned char *buffer;
    unsigned char *palette;
	struct Icon *myIcon;
	int xLineWidth;
	int yLineWidth;
	int nLineFore;
	int nLineBack;
	int SolidPattern;
	int LinePattern;
	int nSolidFore;
	int nSolidBack;
    int ZoneLevel;
	int noFlip;
};

struct paletteDraw{
    
    unsigned char palette[768];
    unsigned char *buffer;  
    
    double sPmin;
    double sPmax;
    
    double dmin;
    double dmax;

    int ScreenBits;
    int sType;			/* Scale Type */
        
    long top;
    long left;
    long xsize;
    long ysize;

    int paletteFont;       
              
    int LabeledPalette;
    int UsePlotScales;
    int UsePlotTime;
    int UsePlotRange;
       
    double red;
    double green;
    double blue;

    int AreaFill;
    int Rotate1d;
        
};

extern int drawPalette(struct paletteDraw *pd);

extern int getPaletteByName(char *name,unsigned char *pal);


extern int FloatToImage(double *d,long length,struct paletteDraw *pd,unsigned char *bp);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif

