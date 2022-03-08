#ifndef _FILE8_

#define _FILE8_ 1

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

int fget8(FILE8 *file,char *data,INT8_64 size,INT8_64 length);


int getFloat8(float *s,FILE8 *in);
int putFloat8(float s,FILE8 *in);

int getDouble8(double *s,FILE8 *in);
int putDouble8(double s,FILE8 *in);

int getString8(unsigned char *s,long n,FILE8 *in);
int putString8(unsigned char *s,long n,FILE8 *nout);
int putLong8(long n,FILE8 *nout);
int getLong8(long *n,FILE8 *in);
int startCRC8(FILE8 *in);
int stopCRC8(unsigned int *crcval,FILE8 *in);
INT8_64 FileLength8(FILE8 *in);

int getDoubleArray8(double *d,long n,FILE8 *in);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
