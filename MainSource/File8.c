
#include "File8.h"
#include "Utilities.h"

extern int cFree(char *data);

extern int mstrncpy(char *out,char *in,long n);

INT8_64 FileSeeks8,FileReads8,FileWrite8,FileReadBytes8;

FILE8 *fopen8(char *name,char *mode)
{
	FILE8 *file;
	FILE *open;

	if(!name || !mode)return NULL;

	open=fopen(name,mode);
	if(!open)return NULL;

	file=(FILE8 *)cMalloc(sizeof(FILE8),6476);
	if(!file){
	    if(open)fclose(open);
	    return NULL;
	}
	zerol((char *)file,sizeof(FILE8));
	file->file=open;
	mstrncpy(file->name,name,sizeof(file->name));
	mstrncpy(file->mode,mode,sizeof(file->mode));
	
	file->convertRead=1;
	file->convertWrite=1;

	return file;

}

int fclose8(FILE8 *file)
{
	if(!file)return 1;
	if(file->file)fclose(file->file);
	file->file=NULL;
	cFree((char *)file);
	return 0;
}

INT8_64 ftell8(FILE8 *file)
{
	if(!file)return -1;
	if(!file->file)return -1;
	return (INT8_64)ftello(file->file);
}

int fseek8(FILE8 *file,INT8_64 offset,int from)
{
	if(!file)return -1;
	if(!file->file)return -1;
	++FileSeeks8;
	return fseeko(file->file,offset,from);
}

int fput8(FILE8 *file,char *data,INT8_64 lengthin)
{
    size_t length;

	if(!data)return -1;
	if(!file)return -1;
	if(!file->file)return -1;
	length=(long)lengthin;
	++FileWrite8;
	if(fwrite((char *)data,1,length,file->file) != length){ 
	    return -1;
	}
	return (int)length;
}

int fget8(FILE8 *file,char *data,INT8_64 size,INT8_64 lengthin)
{
    size_t length;

	if(!data)return -1;
	if(!file)return -1;
	if(!file->file)return -1;
	length=(long)lengthin;
	++FileReads8;
	FileReadBytes8 += length;
	if(fread((char *)data,size,length,file->file) != length){ 
	    return -1;
	}

   // float *in=(float *)data;
   // printf("%g %lld %lld\n",*in,size,lengthin);
    
    
	return (int)length;
}
