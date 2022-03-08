#include "Utilities.h"
#include "File8.h"

#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <Windows.h>
#include <CommDlg.h>
/*
extern char *cMalloc(long Length,int Tag);
extern int cFree(char *data);
extern void zerol(char *ptr,long length);
*/


int crctab[] =				/* CRC lookup table */
{	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};


INT8_64 FileSeeks8,FileReads8,FileWrite8,FileReadBytes8;

FILE8 *fopen8(char *name,char *mode)
{
	FILE8 *file8;
	FILE *out;
	int open;

	if(!name || !mode)return NULL;

  if (strcmp(mode,"rb")==0) {
	  open=_open(name,_O_RDONLY | _O_BINARY);
	  if(open==-1) return NULL;
  } else if (strcmp(mode,"wb")==0){
	  open=_open(name,_O_TRUNC | _O_RDWR | _O_BINARY);
	  if(open==-1){
		  out=fopen(name,"wb");
		  if(out)fclose(out);
		  open=_open(name,_O_TRUNC | _O_RDWR | _O_BINARY);
	  }
	  if(open==-1) return NULL;
  } else {
    return NULL;
  }

	file8=(FILE8 *)cMalloc(sizeof(FILE8),6476);
	if(!file8){
	    if(open!=-1) _close(open);
	    return NULL;
	}
	zerol((char *)file8,sizeof(FILE8));
	file8->ifile=open;
	return file8;

}

int fclose8(FILE8 *file8)
{
	if(!file8)return 1;
  if(file8->ifile!=-1) {
    _close(file8->ifile);
  } else {
    return 1;
  }
	file8->ifile=-1;
	cFree((char *)file8);
	return 0;
}

INT8_64 ftell8(FILE8 *file8)
{
	if(!file8)return -1;
	if(file8->ifile==-1)return -1;
	return _telli64(file8->ifile);
}

int fseek8(FILE8 *file8,INT8_64 offset,int from)
{
	if(!file8)return -1;
	if(file8->ifile==-1)return -1;
	++FileSeeks8;
  if(_lseeki64(file8->ifile,offset,from)==-1) {
    return -1;
  } else {
    return 0;
  }
}

int fput8(FILE8 *file8,char *data,INT8_64 lengthin)
{
	if(!data)return -1;
	if(!file8)return -1;
	if(file8->ifile==-1)return -1;
	if(_write(file8->ifile,(char *)data,(unsigned int)lengthin)!=(int)lengthin) return -1;
	++FileWrite8;
	return 0;
}

int fget8(FILE8 *file8,char *data, INT8_64 size,INT8_64 lengthin)
{
	if(!data)return -1;
	if(!file8)return -1;
	if(file8->ifile==-1)return -1;
	if(_read(file8->ifile,(char *)data,(unsigned int)lengthin*size)!=(int)lengthin*size) return -1;
	++FileReads8;
	FileReadBytes8 += lengthin;
	return (int)lengthin;
}
int getString8(unsigned char *s,INT8_64 n,FILE8 *in)
{
	unsigned int crcval,c;
	
	if(!s || !in || (n < 0))return 1;
	
	if(fget8(in,(char *)s,1,(INT8_64)n))return 1;
	
	if(in->saveCRC){
		crcval=in->crcval;
		while(n-- > 0){
		    c=*s++;
		    
		    crcval = ((((crcval>>8)&0x00ff) ^ crctab[(crcval^(c))&0x00ff]) & 0xFFFF);
		}
		in->crcval=crcval;
	}
	
	return 0;
}
int putString8(unsigned char *s,INT8_64 n,FILE8 *nout)
{
	unsigned short int crcval,c;
	unsigned char *ss;
	INT8_64 nn;
	
	if(!s || !nout || (n < 0))return 1;
	
	if(nout->saveCRC){
		ss=s;
		nn=n;
		crcval=nout->crcval;
		while(nn-- > 0){
		    c=*ss++;
		   
		    crcval = ((((crcval>>8)&0x00ff) ^ crctab[(crcval^(c))&0x00ff]) & 0xFFFF);
		}
		nout->crcval=crcval;
	}
		
	return fput8(nout,(char *)s,(INT8_64)n);
}
/*
int putLong8(long d,FILE8 *nout)
{
    
	if(!nout)return 1;
	
	if(nout->convertWrite){
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)&d;
	    c=cp[np];
	    cp[np]=cp[np+3];
	    cp[np+3]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+2];
	    cp[np+2]=c;
	}	
	
    return putString8((unsigned char *)&d,4L,nout);	
}
int getLong8(long *d,FILE8 *in)
{
    
	if(!d || !in)return 1;
	
	if(getString8((unsigned char *)d,4L,in))return 1;

	if(in->convertRead){
	
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)d;
	    c=cp[np];
	    cp[np]=cp[np+3];
	    cp[np+3]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+2];
	    cp[np+2]=c;
	}	

	return 0;	
}
*/
int getFloatArray8(float *d,INT8_64 n,FILE8 *in)
{
	INT8_64 k;
	
	if(!d || !in)return 1;
	
	if(in->convertRead){
		for(k=0;k<n;++k){
			if(getFloat8(&d[k],in))return 1;
		}
	}else{
        return getString8((unsigned char *)d,n*sizeof(float),in);	
	}
	return 0;
}
int putFloatArray8(float *d,INT8_64 n,FILE8 *nout)
{
	INT8_64 k;
	
	if(!d || !nout)return 1;
	
	if(nout->convertWrite){
		for(k=0;k<n;++k) {
			if(putFloat8(d[k],nout))return 1;
		}
	}else{
        return putString8((unsigned char *)d,n*sizeof(float),nout);	
	}
	return 0;
}
int putFloat8(float d,FILE8 *nout)
{
    
	if(!nout)return 1;
	
	if(nout->convertWrite){
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)&d;
	    c=cp[np];
	    cp[np]=cp[np+3];
	    cp[np+3]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+2];
	    cp[np+2]=c;
	}	
	
    return putString8((unsigned char *)&d,4L,nout);	
}
int getFloat8(float *d,FILE8 *in)
{
    
	if(!d || !in)return 1;
	
	if(getString8((unsigned char *)d,4L,in))return 1;

	if(in->convertRead){
	
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)d;
	    c=cp[np];
	    cp[np]=cp[np+3];
	    cp[np+3]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+2];
	    cp[np+2]=c;
	}	

	return 0;	
}
int getIntArray8(int *d,INT8_64 n,FILE8 *in)
{
	INT8_64 k;
	
	if(!d || !in)return 1;
	
	if(in->convertRead){
		for(k=0;k<n;++k){
			if(getInt8(&d[k],in))return 1;
		}
	}else{
        return getString8((unsigned char *)d,n*sizeof(int),in);	
	}
	return 0;
}
int getInt8(int *d,FILE8 *in)
{
    
	if(!d || !in)return 1;
	
	if(getString8((unsigned char *)d,4L,in))return 1;

	if(in->convertRead){
	
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)d;
	    c=cp[np];
	    cp[np]=cp[np+3];
	    cp[np+3]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+2];
	    cp[np+2]=c;
	}	

	return 0;	
}
int putIntArray8(int *d,INT8_64 n,FILE8 *nout)
{
	INT8_64 k;
	
	if(!d || !nout)return 1;
	
	if(nout->convertWrite){
		for(k=0;k<n;++k) {
			if(putInt8(d[k],nout))return 1;
		}
	}else{
        return putString8((unsigned char *)d,n*sizeof(int),nout);	
	}
	return 0;
}
int putInt8(int d,FILE8 *nout)
{
    
	if(!nout)return 1;
	
	if(nout->convertWrite){
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)&d;
	    c=cp[np];
	    cp[np]=cp[np+3];
	    cp[np+3]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+2];
	    cp[np+2]=c;
	}	
	
    return putString8((unsigned char *)&d,4L,nout);	
}

int getLongLong8(INT8_64 *d,FILE8 *in)
{
    
	if(!d || !in)return 1;
	
	if(getString8((unsigned char *)d,8L,in))return 1;

	if(in->convertRead){
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)d;
	    c=cp[np];
	    cp[np]=cp[np+7];
	    cp[np+7]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+6];
	    cp[np+6]=c;
	    c=cp[np+2];
	    cp[np+2]=cp[np+5];
	    cp[np+5]=c;
	    c=cp[np+3];
	    cp[np+3]=cp[np+4];
	    cp[np+4]=c;
	}	

	return 0;	
}
int getLongLongArray8(INT8_64 *d,INT8_64 n,FILE8 *in)
{
	INT8_64 k;
	
	if(!d || !in)return 1;
	
	if(in->convertRead){
		for(k=0;k<n;++k){
			if(getLongLong8(&d[k],in))return 1;
		}
	}else{
        return getString8((unsigned char *)d,n*sizeof(INT8_64),in);	
	}
	return 0;
}

int putLongLong8(INT8_64 d,FILE8 *nout)
{
    
	if(!nout)return 1;
	
	if(nout->convertWrite){
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)&d;
	    c=cp[np];
	    cp[np]=cp[np+7];
	    cp[np+7]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+6];
	    cp[np+6]=c;
	    c=cp[np+2];
	    cp[np+2]=cp[np+5];
	    cp[np+5]=c;
	    c=cp[np+3];
	    cp[np+3]=cp[np+4];
	    cp[np+4]=c;
	}	

    return putString8((unsigned char *)&d,8L,nout);	
}

int putLongLongArray8(INT8_64 *d,INT8_64 n,FILE8 *nout)
{
	INT8_64 k;
	
	if(!d || !nout)return 1;
	
	if(nout->convertWrite){
		for(k=0;k<n;++k) {
			if(putLongLong8(d[k],nout))return 1;
		}
	}else{
        return putString8((unsigned char *)d,n*sizeof(INT8_64),nout);	
	}
	return 0;
}

int putDoubleArray8(double *d,INT8_64 n,FILE8 *nout)
{
	INT8_64 k;
	
	if(!d || !nout)return 1;
	
	if(nout->convertWrite){
		for(k=0;k<n;++k) {
			if(putDouble8(d[k],nout))return 1;
		}
	}else{
        return putString8((unsigned char *)d,n*sizeof(double),nout);	
	}
	return 0;
}
int getDoubleArray8(double *d,INT8_64 n,FILE8 *in)
{
	INT8_64 k;
	
	if(!d || !in)return 1;
	
	if(in->convertRead){
		for(k=0;k<n;++k){
			if(getDouble8(&d[k],in))return 1;
		}
	}else{
        return getString8((unsigned char *)d,n*sizeof(double),in);	
	}
	return 0;
}
int putDouble8(double d,FILE8 *nout)
{
    
	if(!nout)return 1;
	
	if(nout->convertWrite){
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)&d;
	    c=cp[np];
	    cp[np]=cp[np+7];
	    cp[np+7]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+6];
	    cp[np+6]=c;
	    c=cp[np+2];
	    cp[np+2]=cp[np+5];
	    cp[np+5]=c;
	    c=cp[np+3];
	    cp[np+3]=cp[np+4];
	    cp[np+4]=c;
	}	

    return putString8((unsigned char *)&d,8L,nout);	
}
int getDouble8(double *d,FILE8 *in)
{
    
	if(!d || !in)return 1;
	
	if(getString8((unsigned char *)d,8L,in))return 1;

	if(in->convertRead){
   	 	unsigned char c,*cp;
   	 	int np;
   	 	
   	 	np=0;
		cp=(unsigned char *)d;
	    c=cp[np];
	    cp[np]=cp[np+7];
	    cp[np+7]=c;
	    c=cp[np+1];
	    cp[np+1]=cp[np+6];
	    cp[np+6]=c;
	    c=cp[np+2];
	    cp[np+2]=cp[np+5];
	    cp[np+5]=c;
	    c=cp[np+3];
	    cp[np+3]=cp[np+4];
	    cp[np+4]=c;
	}	

	return 0;	
}
int startCRC8(FILE8 *in)
{
	if(!in)return 1;
	in->saveCRC = TRUE;
	in->crcval=0;
	return 0;
}
int stopCRC8(unsigned int *crcval,FILE8 *in)
{
	if(!in || !crcval)return 1;
	
	in->saveCRC = FALSE;
	*crcval=in->crcval;
	return 0;
}
INT8_64 FileLength8(FILE8 *in)
{
	INT8_64 length;

	if(!in)goto ErrorOut;

    if(fseek8(in,0L,2)){
        goto ErrorOut;
    }

    if((length=ftell8(in)) == -1L){
        goto ErrorOut;
    }
    
	if(fseek8(in,0L,0)){
	    goto ErrorOut;
	}
	return length;
ErrorOut:
	return (INT8_64)(-1);
}
int rewind8(FILE8 *in)
{

	if(!in)return 1;
	
	rewind(in->file);

	return 0;
}
static int intcmp(const void *xx, const void *yy)
{
  char **x;
  char **y;
  int n;

  x=(char **)xx;
  y=(char **)yy;
  n=strcmp(*x,*y);
  return n;
}
/*
struct FileList *FileDialog3(char *name)
{
	struct FileList *Files;
	OPENFILENAME file;
	char filename[256];
	char *buffin;
	int ret,count,n;
	char *p;
	char *pc;


	buffin=cMalloc(250000,928);
	if(!buffin)return NULL;

	memset(&file,0,sizeof(OPENFILENAME));
	file.lStructSize=sizeof(OPENFILENAME);
	file.hwndOwner=HWND_DESKTOP;
	file.lpstrFilter="all\0*.*\0\0\0";
	file.nFilterIndex=1;
	file.lpstrFile=buffin;
	file.nMaxFile=250000;
	strcpy(filename,"This is Filename");
	file.lpstrFileTitle=filename;
	file.nMaxFileTitle=sizeof(filename)-1;
	file.lpstrTitle=name;
	file.Flags=OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST;
	ret=GetOpenFileName(&file);
	if(!ret)return NULL;

	Files=(struct FileList *)cMalloc(sizeof(struct FileList),9002);
	if(!Files){
		Warning("FileDialog Out Of Memeory");
		return NULL;
	}

	zerol((char *)Files,sizeof(struct FileList));

	count=0;
	{
		p=buffin;
		while(strlen(p)){
			++count;
			p += strlen(p)+1;
		}
	}

	p=buffin;

	Files->name=(char **)cMalloc(count*sizeof(char *),9003);
	if(!Files->name){
	    goto ErrorOut;
	}
	zerol((char *)Files->name,sizeof(count*sizeof(char *)));


	Files->directory=strsave(p,9315);
	if(!Files->directory){
		goto ErrorOut;
  }
  if(count == 1){
    pc=strrchr(Files->directory,'\\');
    if(!pc){
      Warning("Directory Not found\n");
      goto ErrorOut;
    }
    Files->name[0]=strsave(pc+1,9314);
    if(!Files->name[0]){
		    goto ErrorOut;
    }
    *pc=0;
  }else{
    n=0;
    {
      
		p=buffin;
        while(strlen(p)){
          if(n > 0){
            Files->name[n-1]=strsave(p,9313);
            if(!Files->name[n-1]){
              goto ErrorOut;
            }	
          }
          ++n;
          p += strlen(p)+1;
        }
    }
    --count;
	}
	Files->count=count;

	goCD(Files->directory);

	   qsort((void *)Files->name,(size_t)count,(size_t)sizeof(char*),
		      (int (__cdecl *)(const void *, const void *))intcmp);

	   {
		   char buff[2000];
			sprintf(buff,"directory %s\n",Files->directory);
			Warning(buff);
			for(n=0;n<count;++n){
				sprintf(buff,"name %s\n",Files->name[n]);
				Warning(buff);
			}
	   }
    return Files;
ErrorOut:
	FileListFree(Files);
	return NULL;
}
*/
int Warning(char *Message)
{
	if (!Message)return 1;
	printf("%s", Message);
	return 0;
}
