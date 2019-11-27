#ifndef _FILE8_

#define _FILE8_ 1

#include "firstFile.h"

#include <stdio.h>

#include "ulibTypes.h"

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
struct file8{
   FILE *file;
   unsigned int crcval;
   int saveCRC;
   char mode[10];
   char name[2048];
   int ifile;  /* used in windows version */
   int convertRead;
   int convertWrite;
};

typedef struct file8 FILE8;


FILE8 *fopen8(char *name,char *mode);


int fclose8(FILE8 *file);

INT8_64 ftell8(FILE8 *file);

int fseek8(FILE8 *file,INT8_64 offset,int from);

int fput8(FILE8 *file,char *data,INT8_64 length);

int fget8(FILE8 *file,char *data,INT8_64 length);


int getFloat8(float *s,FILE8 *in);
int putFloat8(float s,FILE8 *in);

int getDouble8(double *s,FILE8 *in);
int putDouble8(double s,FILE8 *in);

int putDoubleArray8(double *d,INT8_64 n,FILE8 *nout);
int getDoubleArray8(double *d,INT8_64 n,FILE8 *nout);

int putIntArray8(int *d,INT8_64 n,FILE8 *nout);
int getIntArray8(int *d,INT8_64 n,FILE8 *nout);

int putFloatArray8(float *d,INT8_64 n,FILE8 *nout);
int getFloatArray8(float *d,INT8_64 n,FILE8 *nout);

int putLongLongArray8(INT8_64 *d,INT8_64 n,FILE8 *nout);
int getLongLongArray8(INT8_64 *d,INT8_64 n,FILE8 *nout);

int getString8(unsigned char *s,INT8_64 n,FILE8 *in);
int putString8(unsigned char *s,INT8_64 n,FILE8 *nout);
/*
int putLong8(long n,FILE8 *nout);
int getLong8(long *n,FILE8 *in);
*/

int putInt8(int n,FILE8 *nout);
int getInt8(int *n,FILE8 *in);

int startCRC8(FILE8 *in);
int stopCRC8(unsigned int *crcval,FILE8 *in);
INT8_64 FileLength8(FILE8 *in);
int putLongLong8(INT8_64 n,FILE8 *nout);
int getLongLong8(INT8_64 *n,FILE8 *in);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
