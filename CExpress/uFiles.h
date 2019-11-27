#ifndef _RFILES_
#define _RFILES_

#include "firstFile.h"
#include <math.h>
#include <string.h>

#include "SetFrameData.h"
#include "paletteDraw.h"
#include "Limits2.h"
#include "File8.h"
#include "Linedata.h"
#include "BatchFile.h"
#include "FileList.h"
#include "Message.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */

void *cMalloc(unsigned long r,int tag);
char *strsave(char *data,int tag);
extern char WarningBuff[256];

#define HDF_DISK		 	1
#define HDF_MEMORY	 		2
#define PICT_DISK		 	3
#define PICT_MEMORY		 	4
#define TRUE_DISK		 	5
#define TRUE_MEMORY		 	6
#define TEK_DISK		 	7
#define TEK_MEMORY		 	8
#define RGB_DISK			9
#define RGB_MEMORY		   10
#define SDS2D_DISK		   11
#define SDS2D_MEMORY	   12
#define VIEWLIB_MEMORY	   13
#define BMP_DISK		   14
#define BMP_MEMORY		   15
#define GIF_DISK		   16
#define GIF_MEMORY		   17
#define JPG_DISK		   18
#define JPG_MEMORY		   19
#define QUICKTIME_DISK	   20
#define QUICKTIME_MEMORY   21
#define TEXT_MEMORY   	   22

#define SAGE1D_DISK   	   23
#define SAGE2D_DISK   	   24
#define SAGE3D_DISK   	   25
#define VRML_DISK   	   26
#define VRML_MEMORY   	   27
#define SDS3D_DISK		   28
#define SDS3D_MEMORY	   29
#define SHOW_DISK		   30
#define SHOW_MEMORY		   31

#define SAGE_DISK   	   33
#define SAGE_DUMP   	   34

#define SAGEEOS_DISK   	   35

#define STL_DISK		    36
#define CONF_DISK           37
#define STL_BINARY          38
#define CANSCAN_TEXT        39
#define CSV_DISK			40


#define OPEN_ALL_FILE_TYPES	   	99

#define HDF_DISK_NAME		 	"HDF_DISK"
#define HDF_MEMORY_NAME	 		"HDF_MEMORY"
#define PICT_DISK_NAME		 	"PICT_DISK"
#define PICT_MEMORY_NAME		"PICT_MEMORY"
#define TRUE_DISK_NAME		 	"TRUE_DISK"
#define TRUE_MEMORY_NAME		"TRUE_MEMORY"
#define TEK_DISK_NAME		 	"TEK_DISK"
#define TEK_MEMORY_NAME		 	"TEK_MEMORY"
#define RGB_DISK_NAME			"RGB_DISK"
#define RGB_MEMORY_NAME		   	"RGB_MEMORY"
#define SDS2D_DISK_NAME		   	"SDS2D_DISK"
#define SDS2D_MEMORY_NAME	   	"SDS2D_MEMORY"
#define VIEWLIB_MEMORY_NAME	   	"VIEWLIB_MEMORY"
#define BMP_DISK_NAME		   	"BMP_DISK"
#define BMP_MEMORY_NAME		   	"BMP_MEMORY"
#define GIF_DISK_NAME		   	"GIF_DISK"
#define GIF_MEMORY_NAME		   	"GIF_MEMORY"
#define JPG_DISK_NAME		   	"JPG_DISK"
#define JPG_MEMORY_NAME		   	"JPG_MEMORY"
#define QUICKTIME_DISK_NAME	   	"QUICKTIME_DISK"
#define QUICKTIME_MEMORY_NAME   "QUICKTIME_MEMORY"
#define TEXT_MEMOR_NAME   	   	"TEXT_MEMORY"


#define SAGE1D_DISK_NAME   	   	"SAGE1D_DISK"
#define SAGE2D_DISK_NAME   	   	"SAGE2D_DISK"
#define SAGE3D_DISK_NAME   	   	"SAGE3D_DISK"
#define VRML_MEMORY_NAME   	   	"VRML_MEMORY"
#define VRML_DISK_NAME   	   	"VRML_DISK"
#define SDS3D_MEMORY_NAME	   	"SDS3D_MEMORY"
#define SDS3D_DISK_NAME		   	"SDS3D_DISK"
#define SHOW_MEMORY_NAME		"SHOW_MEMORY"
#define SHOW_DISK_NAME		   	"SHOW_DISK"
#define SAGEEOS_DISK_NAME		"SAGEEOS_DISK"
#define STL_DISK_NAME		    "STL_DISK"
#define CONF_DISK_NAME		    "CONFIG_DISK"
#define STL_BINARY_NAME		    "STL_BINARY"
#define CANSCAN_TEXT_NAME		"CANSCAN_TEXT"
#define CSV_DISK_NAME		    "CSV_DISK"





#define TRUE_DISK_MERGED   56
#define FOLDER_MESSAGE		8


#define IMAGE_INCREMENT 5000

#define	HDF_FILES			1
#define	TRUE_FILES			2
#define	SDS2D_FILES			3
#define	SDS3D_FILES			4
#define	TEK_FILES			5
#define	RGB_FILES  			6
#define	PICT_FILES  		7
#define	VIEW_FILES  		8
#define	BMP_FILES			9
#define GIF_FILES			10
#define	JPG_FILES			11
#define QUICKTIME_FILES		12
#define PIO_FILES			13
#define PIO1D_FILES			14
#define PIO2D_FILES			15
#define PIO3D_FILES			16
#define VRML_FILES			17
#define SHOW_FILES			18
#define PIOEOS_FILES		19
#define STL_FILES			20
#define CANSCAN_FILES		21

#define	HDF_FILES_NAME			"HDF_FILES"
#define	TRUE_FILES_NAME			"TRUE_FILES"
#define	SDS2D_FILES_NAME		"SDS2D_FILES"
#define	SDS3D_FILES_NAME		"SDS3D_FILES"
#define	TEK_FILES_NAME			"TEK_FILES"
#define	RGB_FILES_NAME			"RGB_FILES"
#define	PICT_FILES_NAME			"PICT_FILES"
#define	VIEW_FILES_NAME			"VIEW_FILES"
#define	BMP_FILES_NAME			"BMP_FILES"
#define	GIF_FILES_NAME			"GIF_FILES"
#define	JPG_FILES_NAME			"JPG_FILES"
#define	QUICKTIME_FILES_NAME	"QUICKTIME_FILES"
#define	PIO_FILES_NAME			"PIO_FILES"
#define	PIO1D_FILES_NAME		"PIO1D_FILES"
#define	PIO2D_FILES_NAME		"PIO2D_FILES"
#define	PIO3D_FILES_NAME		"PIO3D_FILES"
#define	VRML_FILES_NAME			"VRML_FILES"
#define	SHOW_FILES_NAME			"SHOWL_FILES"
#define	PIOEOS_FILES_NAME		"PIOEOS_FILES"
#define	STL_FILES_NAME			"STL_FILES"
#define	CANSCAN_FILES_NAME		"CANSCAN_FILES"


#define	HDF_FILES_TAG			"HDF"
#define	TRUE_FILES_TAG			"TRU"
#define	SDS2D_FILES_TAG			"S2D"
#define	SDS3D_FILES_TAG			"S3D"
#define	TEK_FILES_TAG			"TEK"
#define	RGB_FILES_TAG			"RGB"
#define	PICT_FILES_TAG			"PIC"
#define	VIEW_FILES_TAG			"VIE"
#define	BMP_FILES_TAG			"BMP"
#define	GIF_FILES_TAG			"GIF"
#define	JPG_FILES_TAG			"JPG"
#define	QUICKTIME_FILES_TAG		"QUI"
#define	PIO_FILES_TAG			"PIO"
#define	PIO1D_FILES_TAG			"P1D"
#define	PIO2D_FILES_TAG			"P2D"
#define	PIO3D_FILES_TAG			"P3D"
#define	VRML_FILES_TAG			"VRM"
#define	SHOW_FILES_TAG			"SHO"
#define	PIOEOS_FILES_TAG		"TOS"
#define	STL_FILES_TAG			"STL"
#define	CANSCAN_FILES_TAG		"SCN"



#define TRUE_EXPANDED			7
#define TRUE_COMRESSED			8
#define TRUE_COMRESSED_POINTER	9


#define DFNT_FLOAT32		5
#define DFNT_FLOAT64		6

#define SDS2D_OUTPUT_SDS2D		18
#define SDS2D_OUTPUT_HDF		19


struct RayList{
    char **nameList;
    long nameCount;	
};


/* struct FileInfo2 *DummyHere; */

struct RayList *GetRayList(struct FileInfo2 *Files);

struct SDS2D{
    unsigned char *buffer;
    int xsize;
    int ysize;
    long length;
    long HdfSize;
    long Offset;
	struct LIMITS limits;
};

struct SDS3D{
    unsigned char *buffer;
    int xsize;
    int ysize;
    int zsize;
    long length;
    long HdfSize;
    long Offset;
	struct LIMITS limits;
};

struct TEKI{
    unsigned char *buffer;
    long length;
};

struct VRML{
    unsigned char *buffer;
    long length;
};

struct PICTI{
    unsigned char *buffer;
    int xsize;
    int ysize;
    long length;
};


struct QUICKTIMEI{
    unsigned char *buffer;
    int xsize;
    int ysize;
    long length;
    long inTime;
	//Movie movie;
};

struct TRUEI{
    unsigned char *buffer;
    int xsize;
    int ysize;
    long UnCompressed;
    long Compressed;
    unsigned int Esc1;
    unsigned int Esc2;
    long Offset;
};

struct RGBI{
    unsigned char *buffer;
    int xsize;
    int ysize;
    int itype;
    int izsize;
    long length;
};



struct HDF{
    unsigned char *buffer;
    int xsize;
    int ysize;
    long length;
    long HdfSize;
    long Offset;
	struct LIMITS limits;
};

struct BMPI{
    unsigned char *buffer;
    int xsize;
    int ysize;
    long UnCompressed;
    long Compressed;
    unsigned int Esc1;
    unsigned int Esc2;
    long length;
};

struct JPGI{
    unsigned char *buffer;
    int xsize;
    int ysize;
    long UnCompressed;
    long Compressed;
    unsigned int Esc1;
    unsigned int Esc2;
    long length;
};

struct GIF{
    unsigned char *buffer;
    unsigned char pal[768];
    int xsize;
    int ysize;
    long length;
    long HdfSize;
    long Offset;
};

#define SAGE_DOUBLE     10

struct PIOData{
    char *sio_names;
    int sio_indexs;
    int sio_lengths;
    INT8_64 sio_positions;
    int sio_types;
};

struct PIO{
    struct PIOData *data;
    int sio_num;
    int SageFlip;
    FILE8 *in;
    double dxset;
    double dyset;
    double dzset;
    double time;
    long numcell;
    long nummat;
    long numdim;
    int mixed_cell;
    int strength;
    int strength_option;
    int *strength_nm;
    int tracer_num_pnts;
    int tracer_num_vars;
    int tracer_words_per_record;
    int tracer_words_per_point;
    double xzero;
    double yzero;
    double zzero;
    long ixmax;
    long iymax;
    long izmax;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double zmin;
    double zmax;
};

int SageGetData(struct PIO *sage,char *dataName,int index,
                double **data,long *dataLength,int *dataType);

int SageGetItem(struct PIO *sage,char *dataName,long plotItem,
                double **data,long *dataLength,int *dataType);
struct SHOW{
    long nodeCount;
    long valueCount;
    long normalCount;
    long dxyzCount;
    long rsCount;
    long localSystem;
    long elementCount;
    long stressCount;
    long nodesMax;
    long offset;
    double *x;
    double *y;
    double *z;
    double *v;
    double *nx;
    double *ny;
    double *nz;
    double *dx;
    double *dy;
    double *dz;
    double *r;
    double *s;
    double *stress;
    long *elements;
};


 struct FilePIOInfo{
	struct PlotRangeRange rangeData;
	struct PlotRangeData range;
	struct PlotPlaneData plane;
	struct PlotAttributes pa;
    struct paletteDraw pd;
	struct PIO *sage;
    char pioNameOld[256];
    char pioName[256];
    long *location;
    long CurrentFrame;
    long pioIndexOld;
    long pioIndex;
    double dxset;
    double dyset;
    double dzset;
    long numcell;
    long numcellGlobal;
    long nummat;
    long numdim;
    double *xcenter;
    double *ycenter;
    double *zcenter;
    double *level;
    double *value;
    double *daughter;
    double *active;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double zmin;
    double zmax;
    double lmin;
    double lmax;
    double cmin;
    double cmax;
    double cscale;
    double dx2[100];
    double dy2[100];
    double dz2[100];
    

    
    double a;
    double b;
    
    long CurrentFrameGradients;
    double *gradx;
    double *grady;
    double *gradz;
    int doGradients;
    
    
    /* vector stuff */
   
   
  	struct PlotVectorData vector;       
    long CurrentFrameVectors;
    char vectorxNameOld[32];
    char vectoryNameOld[32];
    char vectorzNameOld[32];
    double vmaxVectors;
    double *vectorx;
    double *vectory;
    double *vectorz;
    long vectorLength;
    
	/* Streamline */
	struct PlotStreamlineData streamline;
	
	struct PlotStreamData streamline3d;
	
	struct PlotStreamData streamline2d;


    /* Raster Plot Information */
    
    struct PlotRasterData raster;
    
    int GetDataFlag;
    
    int *LaserPlotList;
    long LaserPlotCount;
    
    int *TracerPlotList;
    long TracerPlotCount;

    int tracer_num_pnts;
    int tracer_num_vars;
    int tracer_words_per_record;
    int tracer_words_per_point;
    
    /* Block info */
    
    long BlockSize;
    long BlockNumber;
    
    double *bt1;
    double *bt2;
    double *bt3;
    double *bt4;
    long BlockSizeTemp;

    /* Block Gradient Stuff */
    
    double *next[6];
    double *gradl;
    double *gradh;
    double *lo;
    double *hi;
    
    
 };



struct gradDataStruct{
    double *next;
    float *gradh;
    float *gradl;
};

typedef float tFlt;

typedef struct SageInfo{
    char pioNameOld[256];
    char pioName[256];
    int pioIndexOld;
    int pioIndex;
    double dxset;
    double dyset;
    double dzset;
    long numcell;
    long nummat;
    long numdim;
    tFlt *xcenter;
    tFlt *ycenter;
    tFlt *zcenter;
    unsigned char *level;
    tFlt *value;
    double *daughter;
    double dx2[100];
    double dy2[100];
    double dz2[100];
    struct PIO *sage;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double zmin;
    double zmax;
    long *location;
    float *gradx;
    float *grady;
    float *gradz;
    struct gradDataStruct gradData;
    char *hasData;
    int HasGradients;
    long CurrentFrame;
    double lmax;
} *SagePtr;


typedef struct FileInfo2{
    char directory[1024];
    struct QUICKTIMEI *QUICKTIMEList;
    struct SDS2D *SDS2DList;
    struct SDS3D *SDS3DList;
    struct TRUEI *TRUEList;
    struct PICTI *PICTList;
    struct PIO  *PIO2DList;
    struct BMPI  *BMPList;
    struct JPGI  *JPGList;
    struct SHOW *SHOWList;
    struct VRML *VRMLList;
    struct GIF  *GIFList;
    struct HDF  *HDFList;
    struct TEKI *TEKList;
    struct RGBI *RGBList;
    char *SaveDirectory;
    char **FilesNames;
    char **ImageNames;
    long *ImageFiles;
    long CurrentFrame;
    long ImageCount;
    long FileCount;
    long ImageMax;
    long xmin;
    long ymin;
    long zmin;
    long xmax;
    long ymax;
    long zmax;
    double xminr;
    double yminr;
    double zminr;
    double xmaxr;
    double ymaxr;
    double zmaxr;
    double vminr;
    double vmaxr;
    long iGotData;
    long length;
    int fileType;
    int fileClass;
    int floatType;
    int loadToMemory;
    int (*pdefault)(struct FileInfo2 *files);
    int (*pSetFrame)(long CurrentFrame,
                     struct FileInfo2 *Files);
                     
     int (*pGetData)(double **data,long *length,
                     long CurrentFrame,
                     struct FileInfo2 *Files);

	int  (*pGetData2)(struct FileInfo2 *files,long CurrentFrame,struct SetFrameData *sd);
                     
	int  (*pGetData3)(struct FileInfo2 *files,long CurrentFrame,struct SetFrameData *sd);
                     
	int  (*pGetCellData)(struct FileInfo2 *files,long CurrentFrame,struct SetFrameData *sd);
	
	int  (*pSetCellData)(struct FileInfo2 *files,long CurrentFrame,struct SetFrameData *sd);
	
	int  (*pPutData2)(struct FileInfo2 *files,long CurrentFrame,struct SetFrameData *sd);
                     
	int  (*pInformation)(struct FileInfo2 *files,struct SetFrameData *sd);
                     
	int  (*pGetAreaData)(struct FileInfo2 *files,struct areadata *ai);
	
	int  (*pGetLineData)(struct FileInfo2 *files,struct linedata *li);
                     
	int  (*pPlotLineData)(struct FileInfo2 *files,struct Message1000 *m);
                     
	int  (*pDoDialog)(struct FileInfo2 *files,int *type);
		
    int (*pSaveOne)(char *Name,long CurrentFrame,
                    struct FileInfo2 *Files);
     int (*pSaveOneMerged)(char *filename,char *Name,
                           long CurrentFrame2,
                           struct FileInfo2 *Files);
    int (*pGetInfo)(char **ImageName,long *xsize,long *ysize,
	          long *Length,long CurrentFrame,
	          struct FileInfo2 *Files);

	int  (*pCommandSend)(struct FileInfo2 *files,CommandPtr Commands);

    unsigned char *dataSave;
    struct file8 *in8;
    FILE *in;
    FILE *out;
    long FileOpen;
    long FileTagCount;
    char paletteName[256];
    char palette[768];
    struct SetFrameData SetData;
    long ImageIncrement;
	struct LIMITS limits;
    char pioName[256];
    long pioIndex;
    double pioTime;
    int doNotAutoRemove;
    int openedByFileManager;
    struct FilePIOInfo pioData;
    //struct vIcon vmyIcon;
    char fileListName[256];
    char outputDirectory[1024];
    char outputName[256];
    char outputSequenceName[256];
    long outputSequenceNumber;
    int outputType;
    int inputType;
    long xSize;
    long ySize;
    long zSize;
    unsigned char *reflectSave;
    long reflectLength;
    int Reflect_x;    
} *FilePtr;

#define FILES_SET_REFLECT_X 1

struct FilesSetList{
    int Reflect_x;
};

int FilesSetParameters(struct FileInfo2 *Files,struct FilesSetList *list,int Item);

int SageGetFileItem(struct FileInfo2 *Files,char *dataName,long plotItem,
                double **data,long *dataLength,long CurrentFrame);

int SageGetFileItemBlock(struct FileInfo2 *Files,char *dataName,long plotItem,
                         double *data,long start,long dataLength,long CurrentFrame);

int SageSetFileItem(struct FileInfo2 *Files,char *dataName,long plotItem,
                    void *data,long dataLength,long CurrentFrame);

int FilesSaveTEKToMemory(char *name,unsigned char *bline,
                         long size,struct FileInfo2 *Files);

//IconPtr OpenTekFiles(struct FileInfo2 *Files);


struct FileInfo2 *FilesTEK(void);

struct FileInfo2 *FilesSDS2D(void);

struct FileInfo2 *FilesOpenList(struct FileList *FilesIn, int flag);

int  FileListFree(struct FileList *FilesIn);

int FileTrueInfo(char **ImageName,long *xsize,long *ysize,
	long *UnCompressed,long *Compressed,
	unsigned int *Esc1, unsigned int *Esc2,
	long CurrentFrame,struct FileInfo2 *Files);

int FilesGetTRUE(unsigned char **buffer,long length,
                long CurrentFrame,int How,struct FileInfo2 *Files);

int FilesSetFrame(long CurrentFrame,struct FileInfo2 *Files);

int freeFileList(struct FileInfo2 *Files);

int FilesGetSDS2DTextvalue(char *buff,int ix,int iy,long CurrentFrame,
						struct FileInfo2 *Files);
						
int FilesGetSDS2Dvalue(double *value,int ix,int iy,long CurrentFrame,
						struct FileInfo2 *Files);
						
int FileHdfInfo(long *xsize,long *ysize,long *length,
                long CurrentFrame,struct FileInfo2 *Files);
						
int FileAppendHdf(char *name,int xsize,int ysize,unsigned char *image,
                  long Length,struct FileInfo2 **iFiles);
                  
int FilePutLimits(struct LIMITS *limits,long CurrentFrame,struct FileInfo2 *Files);

int FilePutLimitsString(char *limits,long CurrentFrame,struct FileInfo2 *Files);
				
int FileGetLimits(struct LIMITS **limits,long CurrentFrame,struct FileInfo2 *Files);
				
int FileAppendSDS2D(char *name,int xsize,int ysize,
                  double *image,long CurrentFrame,struct FileInfo2 **iFiles);

int FileAppendSDS3D(char *name,int xsize,int ysize,int zsize,
                  double *image,long CurrentFrame,struct FileInfo2 **iFiles);
                  
//int filePIOList(struct FileInfo2 *Files,long CurrrentFrame,struct ScrollList *List,int flag);

char *FilesImageName(struct FileInfo2 *Files,long CurrentFrame);

int FilesOpenRemoveNotOwned(void);

int FileOpenIsOpen(struct FileInfo2 *File);

int FileOpenSave(struct FileInfo2 *Files);

int SetFileToRead(long CurrentFrame,struct FileInfo2 *Files);

int SetFileToRead8(long CurrentFrame,struct FileInfo2 *Files);

FILE8 *fopen8(char *name,char *mode);

int fclose8(FILE8 *file);

int rewind8(FILE8 *file);

INT8_64 ftell8(FILE8 *file);

int fseek8(FILE8 *file,INT8_64 offset,int from);

int fput8(FILE8 *file,char *data,INT8_64 length);

int fget8(FILE8 *file,char *date,INT8_64 length);

char *FilesClassName(struct FileInfo2 *Files);

char *DefaultPathString(void);

int freeFileList(struct FileInfo2 *Files);

int doSDS2DStart(struct FileInfo2 *Files);

int do24BitStart(struct FileInfo2 *Files);

int FilesSaveFiles(struct FileInfo2 *Files);

int FilesAppendFiles(struct FileInfo2 *FilesOld,struct FileInfo2 *FilesNew);


#define FLOAT_NOT_SET ((double)(-1.23e-21))


int FilesOpenDefault(struct FileInfo2 *files);

int UnPack(register unsigned char *image,register unsigned char *buff,register long Length);

int FilePutCommand(struct LIMITS *limits);

int FilePutCommandBuff(char *buff,double xmin,double xmax,double ymin,double ymax,double zmin,double zmax,
                       double vmin,double vmax,int dolog,int pioIndex,unsigned char *pioName,double time);

char *FilesClassTag(struct FileInfo2 *Files);

//char *FilesTypeName(struct FileList *Files);

int IsFile(char *name);

//struct FileInfo2 *initFILES(struct FileList *FilesIn);

int printFileInformation(struct FileInfo2 *files);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif


