#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#define UNIX
#ifdef UNIX
#include <sys/time.h>
int GetTime(long *Seconds,long *milliseconds);
#endif

int putString(char *s,long n,FILE *in);

int getString(unsigned char *data,unsigned long length,FILE *out);

double rtime(void);

int timeTest(char *name);

long FileLength(FILE *in);

int main(int argc,char *argv[])
{
	double start,total;
	/*
	extern void *malloc(int length);
	*/
	char *name,*block;
	FILE *out;
	long int length;
	long long count;
	int n;

	name="ioTest.jnk";
	out=fopen(name,"w");
	if(!out){
	    printf("Could Not Open %s to Write\n",name);
	    exit(1);
	}
	length=40000000;
	block=(char *)malloc(length);
	if(!out){
	    printf("Could Not Allocate %ld Bytes\n",length);
	    exit(1);
	}
	start=rtime();

	count=0;
	for(n=0;n<10;++n){
	    if(putString(block,length,out)){
		    printf("Error Writting %s\n",name);
		    exit(1);
	    }
	    count += length;
	}
	
	if(out)fclose(out);
       
    total=rtime()-start;
    printf("%.2f Seconds To Write %lld Bytes  %ld Blocksize %.2f MBytes/Sec\n",
          total,count,length,(double)count/(1e6*total));
    
        
    return 0;

}
#ifdef UNIX
double rtime(void)
{
        long milliseconds;
        long Seconds;
        double ret;


        GetTime(&Seconds,&milliseconds);

        ret=(double)Seconds+(double)milliseconds/1000.;

        return ret;

}
int GetTime(long *Seconds,long *milliseconds)
{
        struct timeval curTime;

        gettimeofday(&curTime,(struct timezone *) NULL);
        *Seconds=curTime.tv_sec;
        *milliseconds=curTime.tv_usec/1000;
        return 0;
}
#else
double rtime(void)
{
	unsigned long start;
	time_t ship;
	
	start=time(&ship);
	
	return (double)start;

}
#endif
int getString(unsigned char *data,unsigned long length,FILE *out)
{
        if(!data || !out)return 1;
        if(fread(data,1,length,out) != length){
            return 1;
        }
        return 0;
}
int putString(char *s,long n,FILE *in)
{
        unsigned int nn;
        long last;
        
        nn=n;
        if(nn == n){
            if(fwrite(s,1,nn,in) != nn)return 1;     
        }else{
            last=n;
            while(last > 0){
                nn=32000;
                if(nn > last)nn = last;
                if(fwrite(s,1,nn,in) != nn)return 1;
                s += nn;
                last -= nn;
            }
        }
        return 0;
        
}
long FileLength(FILE *in)
{
                long length;

                if(!in)goto ErrorOut;

            if(fseek(in,0L,2)){
                goto ErrorOut;
            }

            if((length=ftell(in)) == -1L){
                goto ErrorOut;
            }

                if(fseek(in,0L,0)){
                    goto ErrorOut;
                }
                return length;
ErrorOut:
        return -1L;
}
