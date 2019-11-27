#define EXTERN22 extern
#include "firstFile.h"
#include <stdio.h>
#include <math.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct complex2 {
	double x, y;
};

#define MULT4

#define PI 3.1415926535979

int doFFT(double *x,double *y,long length,int direction);

int doFFT2(double *x,double *y,long length,int direction);

int fft(double *data,int nn,int isign);

int fftn(struct complex2 *data, unsigned int *nn,int ndim,int isign);

int window(struct complex2 *rt,int nfft,int num,int direction);

extern int WarningBatch(char *message);
void *cMalloc(unsigned long ,int tag);
extern char WarningBuff[256];
int cFree(char *);

int doFFT3D(double *x,double *y,long xsize,long ysize,long zsize,int direction,int filter);

int doFFT3D(double *x,double *y,long xsize,long ysize,long zsize,int direction,int filter)
{
	struct complex2 *data;
	unsigned int nn[4];
	int nd;
	long length;
	long n,n2;
	int ifound;
	
	if(!x || !y)return 1;
	
	n2=2;
	ifound=FALSE;
	for(n=0;n<29;++n){
		if(xsize == n2){
			ifound=TRUE;
			break;
		}
		n2 *= 2;
	}
	
	if(!ifound){
	    sprintf(WarningBuff,"doFFT3D Did not find power of 2 xsize %ld\n",xsize);
	    WarningBatch(WarningBuff);
	    return 1;
	}
	
	nd=1;
	
	if(ysize > 1){
		n2=2;
		ifound=FALSE;
		for(n=0;n<29;++n){
			if(ysize == n2){
				ifound=TRUE;
				break;
			}
			n2 *= 2;
		}
		
		if(!ifound){
		    sprintf(WarningBuff,"doFFT3D Did not find power of 2 ysize %ld\n",xsize);
		    WarningBatch(WarningBuff);
		    return 1;
		}
		nd=2;
		if(zsize > 1){
			n2=2;
			ifound=FALSE;
			for(n=0;n<29;++n){
				if(zsize == n2){
					ifound=TRUE;
					break;
				}
				n2 *= 2;
			}
			
			if(!ifound){
			    sprintf(WarningBuff,"doFFT3D Did not find power of 2 zsize %ld\n",xsize);
			    WarningBatch(WarningBuff);
			    return 1;
			}
			nd=3;
		}else{
		   zsize=1;
		}
	
	}else{
	   ysize=zsize=1;
	}
	
	
	length=xsize*ysize*zsize;
	data=(struct complex2 *)cMalloc(length*sizeof(struct complex2),9092);
	if(!data){
	    WarningBatch("doFFT3D out of Memory\n");
	    return 1;
	}
	
	for(n=0;n<length;++n){
		data[n].x=x[n];
		data[n].y=y[n];
	}
	
	if(direction == 1){
	     window(data,(int)length,filter,direction);
	}
	
	nn[0]=(unsigned int)xsize;
	nn[1]=(unsigned int)ysize;
	nn[2]=(unsigned int)zsize;
	nn[3]=0;
	
	
	fftn(data, nn,nd,direction);
	
	if(direction == -1){
	    window(data,(int)length,filter,direction);
	}
	
	for(n=0;n<length;++n){
		x[n]=data[n].x;
		y[n]=data[n].y;
	}
	
	
	if(data)cFree((char *)data);
	
	return 0;
	
}
int window(struct complex2 *rt,int nfft,int filter,int direction)
{      
    double scale,p2;
    int i;
    
    if(!rt)return 1;


      if(filter <= 0){
/*

        square window
 
*/
		  if(direction == 1){
	      	scale=2./nfft;
          }else{
	      	scale=0.5;
          }
          for(i=0;i<nfft;++i){
              rt[i].x=scale*rt[i].x;
              rt[i].y=scale*rt[i].y;
          }
      } else if(filter == 1){
/*

        hamming window
 
*/
          p2=8.*atan(1.);
          scale=2./(.54*nfft);
          for(i=0;i<nfft;++i){
              rt[i].x=scale*(.54-.46*cos(p2*i/nfft))*rt[i].x;
          }
      }
      
      return 0;
} 
int fftn(struct complex2 *data, unsigned int *nn,int ndim,int isign)
{
	int idim;
	unsigned i1, i2rev, i3rev, ibit;
	unsigned ip2, ifp1, ifp2, k2, n;
	unsigned nprev = 1, ntot = 1;
	register unsigned i2, i3;
	double theta;
	struct complex2 w, wp;

	double wtemp;
	struct complex2 temp, wt;

#ifndef MULT4
	double t1, t2;
#endif

    if(!data || !nn)return 1;

	for (idim = 0; idim < ndim; ++idim)
		ntot *= nn[idim];
	for (idim = ndim - 1; idim >= 0; --idim) {
		n = nn[idim];

		//nrem = ntot / (n * nprev);
		ip2 = nprev * n;       
		i2rev = 0;             

		for (i2 = 0; i2 < ip2; i2 += nprev) {
			if (i2 < i2rev)
				for (i1 = i2; i1 < i2 + nprev; ++i1)
					for (i3 = i1; i3 < ntot; i3 += ip2) {
						i3rev = i3 + i2rev - i2;
						temp.x = data[i3].x;
						data[i3].x = data[i3rev].x;
						data[i3rev].x = temp.x;
						temp.y = data[i3].y;
						data[i3].y = data[i3rev].y;
						data[i3rev].y = temp.y;
					}

			ibit = ip2;
			do {
				ibit >>= 1;
				i2rev ^= ibit;
			} while (ibit >= nprev && !(ibit & i2rev));
		}

		for (ifp1 = nprev; ifp1 < ip2; ifp1 <<= 1) {
			ifp2 = ifp1 << 1;
			theta = isign * 2.0 * PI / (ifp2 / nprev);
			wp.x = sin(0.5 * theta);
			wp.x *= -2.0 * wp.x;
			wp.y = sin(theta);
			w.x = 1.0;
			w.y = 0.0;

			for (i3 = 0; i3 < ifp1; i3 += nprev) {
				for (i1 = i3; i1 < i3 + nprev; ++i1)
					for (i2 = i1; i2 < ntot; i2 += ifp2) {
						k2 = i2 + ifp1;
						wt.x = data[k2].x;
						wt.y = data[k2].y;

#ifdef MULT4
						data[k2].x = data[i2].x - (temp.x = w.x * wt.x - w.y * wt.y);
						data[k2].y = data[i2].y - (temp.y = w.x * wt.y + w.y * wt.x);
#else
						data[k2].x = data[i2].x - (temp.x =
							(t1 = w.x * wt.x) - (t2 = w.y * wt.y));
						data[k2].y = data[i2].y - (temp.y =
							(w.x + w.y) * (wt.x + wt.y) - t1 - t2);
#endif
						data[i2].x += temp.x;
						data[i2].y += temp.y;
					}
				wtemp = w.x;
#ifdef MULT4
				w.x += w.x * wp.x - w.y * wp.y;
				w.y += wtemp * wp.y + w.y * wp.x;
#else
				w.x += (t1 = w.x * wp.x) - (t2 = w.y * wp.y);
				w.y += (wtemp + w.y) * (wp.x + wp.y) - t1 - t2;
#endif
			}
		}
	nprev *= n;
	}
	return 0;
}
int doFFT2(double *x,double *y,long length,int direction)
{
	double *datar;
	long n,n2;
	int ifound;
	
	if(!x || !y)return 1;
	
	n2=2;
	ifound=FALSE;
	for(n=0;n<29;++n){
		if(length == n2){
			ifound=TRUE;
			break;
		}
		n2 *= 2;
	}
	
	if(!ifound){
	    sprintf(WarningBuff,"doFFT Did not find power of 2 length %ld\n",length);
	    WarningBatch(WarningBuff);
	    return 1;
	}
	
	datar=(double *)cMalloc(2*length*sizeof(double),9092);
	if(!datar){
	    WarningBatch("doFFT out of Memory\n");
	    return 1;
	}
	
	for(n=0;n<length;++n){
		datar[2*n]=x[n];
		datar[2*n+1]=y[n];
	}
	
	fft(datar,(int)length,direction);
	
	for(n=0;n<length;++n){
		x[n]=datar[2*n];
		y[n]=datar[2*n+1];
	}
	
	if(datar)cFree((char *)datar);
	
	return 0;
	
}
int fft(double *data,int nn,int isign)
{
	double twopi,tempr,tempi,wstpr,wstpi;
	double wr,wi,theta,sinth,fni;
	int i,j,n,m,mmax,istep;
	
      data -= 1;
      j=1;
      n=2*nn;
      twopi=8.*atan(1.);
       for(i=1;i<=n;i += 2){
       if(i-j >= 0)goto L200;
       tempr=data[j];
       tempi=data[j+1];
       data[j]=data[i];
       data[j+1]=data[i+1];
       data[i]=tempr;
       data[i+1]=tempi;
L200:    m=n/2;
L300:    if(j-m > 0)goto L400;
		goto L500;
L400:    j=j-m;
       m=m/2;
       if(m-2 >= 0)goto L300;
L500:  j=j+m;
       }
      mmax=2;
L600:   if(mmax-n >= 0)goto L1000;
	  istep=2*mmax;
      theta=twopi/(double)(isign*mmax);
      sinth=sin(theta/2.);
      wstpr=-2.*sinth*sinth;
      wstpi=sin(theta);
      wr=1.;
      wi=0.;
		for(m=1;m<=mmax;m+=2){
			for( i=m;i<=n;i+=istep){
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i]=data[i]+tempr;
				data[i+1]=data[i+1]+tempi;
			}
			tempr=wr;
			wr=wr*wstpr-wi*wstpi+wr;
			wi=wi*wstpr+tempr*wstpi+wi;
		}
      mmax=istep;
      goto L600;
L1000: 

	if(isign > 0){
		fni=2.0/(double)nn;
	}else{
		fni=0.5;
	}
	for( i=1;i<=2*nn;++i){
	    data[i]=data[i]*fni;
	}
	return 0;
}
