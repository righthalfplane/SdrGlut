#ifndef _ULIBTYPES_
#define _ULIBTYPES_


typedef struct uRectStruct {
    int x;
    int y;
    int xsize;
    int ysize;
} uRect;

typedef struct uPointStruct {
    int x;
    int y;
} uPoint;


typedef struct uFontStruct {
   int LineHeight;
   int Descent;
   int CWidth;
} uFontInfo;



#ifdef WIN32
#include <direct.h>
#define FILE_NAME_SEPERATOR			"\\"
#define FILE_NAME_SEPERATOR_CHAR	'\\'
#ifdef _int64
typedef _int64 INT8_64;
#else
typedef long long INT8_64;
#endif
#else
/* #include <GL/glew.h>	*/ /* GLUT OpenGL includes */
#define FILE_NAME_SEPERATOR			"/"
#define FILE_NAME_SEPERATOR_CHAR	'/'
typedef long long INT8_64;
#endif


#endif
