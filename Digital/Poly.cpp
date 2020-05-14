//
//  Poly.cpp
//  digital
//
//  Created by Dale on 12/17/16.
//  Copyright © 2016 Dale. All rights reserved.
//

#include "utilities.h"

#include "Poly.h"

int doFFT2(double *x,double *y,long length,int direction);

Poly::Poly()
{
    np=0;
    nz=0;
    con=1.0;
    iprint=1;
    iangle=0;
    poles=NULL;
    zeros=NULL;
    fore=NULL;
    back=NULL;
    sampleRate=10000;
    delay=NULL;
    coef1=NULL;
    coef2=NULL;
    type=NULL;
    pz=NULL;
    FIRCount=0;
    xn=NULL;
    yn=NULL;
    FIRCoefficients=NULL;

    
}
Poly::~Poly()
{
    if(poles)delete [] poles;
    if(zeros)delete [] zeros;
    poles=NULL;
    zeros=NULL;
    
    if(fore)delete [] fore;
    if(back)delete [] back;
    fore=NULL;
    back=NULL;
    
    if(delay)delete [] delay;
    delay=NULL;
    
    if(coef1)delete [] coef1;
    coef1=NULL;
    
    if(coef2)delete [] coef2;
    coef2=NULL;
    
    if(type)delete [] type;
    type=NULL;
    
    if(pz)delete [] pz;
    pz=NULL;
  
    if(FIRCoefficients)delete [] FIRCoefficients;
    FIRCoefficients=NULL;
    
    if(xn)delete [] xn;
    xn=NULL;
    
    if(yn)delete [] yn;
    yn=NULL;
    
    
}
int Poly::forceFIR(double *input,int npoint)
{
    
    
    double scale=1.0;  // normalize at 0 Hz
    
    double t=1.0/sampleRate;
    
    fprintf(stderr,"forceFIR %d scale %18.9e period %18.9e\n",npoint,scale,t);
    
    fprintf(stderr,"           SECONDS,           AMPLITUDE,             INPUT\n");
    
    
    
    double *xn = new double[FIRCount];
    
    for(int n=0;n<FIRCount;++n)xn[n]=0;
    
    
    for(int n=0;n<npoint;++n){
        double y;
        double x;
        x=input[n];
        y=x*FIRCoefficients[FIRCount-1];
        
        
        for(int k=1;k<FIRCount;++k){
            y += xn[k-1]*FIRCoefficients[FIRCount-k-1];
            //fprintf(stderr,"y1 %g\n",y);
        }
        
        
        for(int k=0;k<FIRCount-1;++k){
            xn[FIRCount-k-1]=xn[FIRCount-k-2];
        }
        xn[0]=x;
        
        fprintf(stderr,"    %18.9e, %18.9e , %18.9e\n",n*t,y/scale,x);
    }
    
    delete [] xn;
    
    return 0;
}
int Poly::force(double *input,int npoint)
{
    
    if(FIRCount > 0){
        return forceFIR(input,npoint);
    }
    
    complex<double> sum;
    
    complex<double> z = 1.0;
    sum = 1;
    if(nz > 0){
        for(int n=0;n<nz;++n){
            sum *= (z-zeros[n]);
        }
    }
    
    if(np > 0){
        for(int n=0;n<np;++n){
            sum /= (z-poles[n]);
        }
    }
    
    double scale=abs(sum);  // normalize at 0 Hz
    
    double t=1.0/sampleRate;
    
    fprintf(stderr,"force %d scale %18.9e period %18.9e\n",npoint,scale,t);
   
    fprintf(stderr,"           SECONDS,           AMPLITUDE,             INPUT\n");
  

    if(xn)delete [] xn;
    
    if(yn)delete [] yn;
    
    xn = new double[nfore];
    
    for(int n=0;n<nfore;++n)xn[n]=0;
    
    yn = new double[nback];
    
    for(int n=0;n<nback;++n)yn[n]=0;
    
    for(int n=0;n<npoint;++n){
        double y;
        double x;
        x=input[n];
        y=x*fore[nfore-1];
  
        
        for(int k=1;k<nfore;++k){
            y += xn[k-1]*fore[nfore-k-1];
            //fprintf(stderr,"y1 %g\n",y);
        }
       
        for(int k=1;k<nback;++k){
            y -= yn[k-1]*back[nback-k-1];
            //fprintf(stderr,"y2 %g\n",y);
        }
        
        for(int k=0;k<nback-1;++k){
            yn[nback-k-1]=yn[nback-k-2];
        }
        yn[0]=y;
       
        for(int k=0;k<nfore-1;++k){
            xn[nfore-k-1]=xn[nfore-k-2];
        }
        xn[0]=x;
            
        fprintf(stderr,"    %18.9e, %18.9e , %18.9e\n",n*t,y/scale,x);
    }
   
    return 0;
}
int Poly::SetPolesAndZeros(int np,int nz)
{
    this->np=np;
    
    this->nz=nz;
    
    if(poles)delete [] poles;
    poles=NULL;
    
    if(zeros)delete [] zeros;
    zeros=NULL;
    
    if(np > 0)poles=new complex<double>[np];
    
    if(nz > 0)zeros=new complex<double>[nz];
    
    
    return 0;
}
int Poly::diff()
{

    nfore=0;
    nback=0;
    
    if(nz > 0){
        if(fore)delete [] fore;
        fore=new double[nz+1];
        complex<double> *t1=new complex<double>[nz+1];
        complex<double> *t2=new complex<double>[nz+1];
        complex<double> *prod=new complex<double>[2*nz+1];
        
        int n1,n2,n3;
        
        t2[0]= -zeros[0];
        t2[1]=1.0;
        n1=2;
        n2=2;
        n3=0;
        
        for(int n=1;n<nz;++n){
            t1[0]= -zeros[n];
            t1[1]=1.0;
            mult(prod,t1,t2,n1,n2,&n3);
            for(int k=0;k<n3;++k){
                t2[k]=prod[k];
            }
            n2=n3;
        }
        
        if(iprint)fprintf(stderr,"\n          Coefficients Fore\n");
        
        nfore=n3;
        for(int n=0;n<nfore;++n){
            fore[n]=t2[n].real();
            if(iprint)fprintf(stderr,"%4d %18.9e, %18.9e \n",n+1,t2[n].real(),t2[n].imag());
        }
        
        delete [] t1;
        delete [] t2;
        delete [] prod;
        
    }
    if(np > 0){
        if(back)delete [] back;
        back=new double[nz+1];
        complex<double> *t1=new complex<double>[nz+1];
        complex<double> *t2=new complex<double>[nz+1];
        complex<double> *prod=new complex<double>[2*nz+1];
        
        int n1,n2,n3;
        
        t2[0]= -poles[0];
        t2[1]=1.0;
        n1=2;
        n2=2;
        n3=0;
        
        for(int n=1;n<np;++n){
            t1[0]= -poles[n];
            t1[1]=1.0;
            mult(prod,t1,t2,n1,n2,&n3);
            for(int k=0;k<n3;++k){
                t2[k]=prod[k];
            }
            n2=n3;
        }
        
        if(iprint)fprintf(stderr,"\n          Coefficients Back\n");
        
        nback=n3;
        for(int n=0;n<nback;++n){
            back[n]=t2[n].real();
            if(iprint)fprintf(stderr,"%4d %18.9e, %18.9e \n",n+1,t2[n].real(),t2[n].imag());
        }
        
        delete [] t1;
        delete [] t2;
        delete [] prod;
        
    }
    
    return 0;
}
int Poly::mult(complex<double> *prod,complex<double> *a,complex<double> *b,int m,int n,int *n3)
{
    for(int i=0;i<m+n-1;++i)prod[i]=0.0;
    
    for (int i=0; i<m; i++)
    {
        for (int j=0; j<n; j++){
            prod[i+j] += a[i]*b[j];
        }
    }
    
    for(int i=0;i<m+n-1;++i){
        if(prod[i] != 0.0)*n3=i+1;
    }
    
    return 1;
}
int Poly::response(double steps)
{
    complex<double> sum;

    complex<double> z = 1.0;
    sum = 1;
    if(nz > 0){
        for(int n=0;n<nz;++n){
            sum *= (z-zeros[n]);
        }
    }
    
    if(np > 0){
        for(int n=0;n<np;++n){
            sum /= (z-poles[n]);
        }
    }

    double scale=abs(sum);  // normalize at 0 Hz

    
    double pi=4.0*atan(1.0);
    
    double dt=(pi)/(steps-1);

    fprintf(stderr,"\n      FREQUENCY,         AMPLITUDE\n");
    for(int k=0;k<steps;++k){
        double theta=k*dt;
        complex<double> z = exp(complex<double>(0,theta));
        sum = 1;
        if(nz > 0){
            for(int n=0;n<nz;++n){
                sum *= (z-zeros[n]);
            }
        }
    
        if(np > 0){
            for(int n=0;n<np;++n){
                sum /= (z-poles[n]);
            }
        }
        fprintf(stderr,"%18.9e, %18.9e\n",theta*sampleRate/(2*pi),abs(sum)/scale);
    }
    
    return 0;
}
int Poly::dft(int npoints)
{
    //fprintf(stderr,"dft npoints %d FIRCount %d\n",npoints,FIRCount);
    
    fprintf(stderr,"\n           N,           COEFFICIENTS\n");
    
    for(int n=0;n<FIRCount;++n){
        printf("         %3d,       %18.9e\n",n, FIRCoefficients[n]);
    }
    
    complex<double> *x = new complex<double>[FIRCount];
    
    complex<double> sum;
    
    double pi=4.0*atan(1.0);
    
    double df=sampleRate/FIRCount;
    
    fprintf(stderr,"\n       FREQUENCY,       AMPLITUDE\n");
    for(int k=0;k<FIRCount;++k){
        sum=0;
        for(int n=0;n<FIRCount;++n){
            sum +=FIRCoefficients[n]*exp(complex<double>(0.0,(-2.0*pi*k*n)/(FIRCount)));
        }
        x[k]=sum;
        fprintf(stderr,"%18.9e,%18.9e\n",df*k,::norm(sum));
    }
    
    
/*
     This is the inverse and it works
 */
    
/*
    fprintf(stderr,"\n       FREQUENCY,    AMPLITUDE\n");
    for(int n=0;n<FIRCount;++n){
        sum=0.0;
        for(int k=0;k<FIRCount;++k){
            sum +=x[k]*exp(complex<double>(0.0,(2.0*pi*k*n)/FIRCount));
        }
        fprintf(stderr,"%16.8g,%16.8g\n",(double)n,sum.real()/FIRCount);
    }
*/
    
    delete [] x;
    
    return 0;
}
int Poly::bilinear(double wT)
{
    warp(wT);
    
    if(nz > 0){
        for(int n=0;n<nz;++n){
            double sig=zeros[n].real();
            double w  =zeros[n].imag();
            double d=(1.0+(sig*sig+w*w)-2.0*sig);
            double x=(1.0-(sig*sig+w*w))/d;
            double y=2.0*w/d;
            zeros[n]=complex<double>(x,y);
        }
    }else{
        nz=np;
        zeros=new complex<double>[nz];
        for(int n=0;n<nz;++n){
            zeros[n]=complex<double>(-1.0,0.0);
        }
    }
    
    if(np > 0){
        for(int n=0;n<np;++n){
            double sig=poles[n].real();
            double w  =poles[n].imag();
            double d=(1.0+(sig*sig+w*w)-2.0*sig);
            double x=(1.0-(sig*sig+w*w))/d;
            double y=2.0*w/d;
            poles[n]=complex<double>(x,y);
        }
    }
    
    char mes[256];
    
    sprintf(mes,"bilinear Coefficients");
    
    writePoly(mes);
    
    
    return 0;
}
int Poly::warp(double f)
{
    double pi=4.0*atan(1.0);
    
    double w=2*pi*f;
    
    double wd=tan(w*0.5/sampleRate);
    
    if(nz > 0){
        for(int n=0;n<nz;++n){
            zeros[n]= -wd*zeros[n];
        }
    }
    
    if(np > 0){
        for(int n=0;n<np;++n){
            poles[n]= -wd*poles[n];
        }
    }
    
    char mes[256];
    
    sprintf(mes,"Warped Coefficients  wd = %18.9e",wd);
    
    writePoly(mes);
    
    return 0;
}

int Poly::doButterWorth(int np)
{
    SetPolesAndZeros(np,0);
  
    double zpole=0.5/(double)np;
    
    double pi=4.0*atan(1.0);
    
    con=1.0;
    for(int k=0;k<np;++k){
        double zk=2*(k+1)-1;
        double ang=pi*(0.5+zk*zpole);
        poles[k]=complex<double>(-cos(ang),-sin(ang));
        // poles[k]= -poles[k];
    }
    
    writePoly((char *)"Butterworth Poles");
    
    return 0;
}
int Poly::doChev(int np,double r)
{
    SetPolesAndZeros(np,0);
    
    double pi=4.0*atan(1.0);
    
    double zn=0.1*r;
    
    double e=sqrt(pow(10,zn)-1.0);
    
    zn=np;
    
    double v=log(1.0/e+sqrt(1.0+1.0/(e*e)))/zn;
    
    double cosh=0.5*(exp(v)+exp(-v));
    
    double sinh=0.5*(exp(v)-exp(-v));
    
    zn=0.5*pi/zn;
    
    for(int k=0;k<np;++k){
        v=zn*(2.0*(double)(k+1+np)-1.0);
        poles[k]=complex<double>(-sinh*sin(v),cosh*cos(v));
        //poles[k] = -poles[k];
    }
    
    norm();
    
    if((np % 2) == 0)con=con/sqrt(1.0+e*e);
    
    char mes[256];
    
    sprintf(mes,"Chebyshev Poles ripple = %18.9e db",r);
    
    writePoly(mes);
    
    return 0;
}
int Poly::norm()
{
    con=1.0;
    
    if(nz > 0){
        for(int i=0;i<nz;++i){
            con *= abs(zeros[i]);
        }
    }
    
    if(np > 0){
        for(int i=0;i<np;++i){
            con *= abs(poles[i]);
        }
    }
    return 0;
}

int Poly::high(double f,int inorm)
{
    double pi=4.0*atan(1.0);
    double w0=2.0*pi*f;
    
    if(nz > 0){
        for(int i=0;i<nz;++i){
            double dnom=abs(zeros[i]);
            dnom *= dnom;
            if(dnom > 0){
                con *= sqrt(dnom);
                dnom=w0/dnom;
                zeros[i]=complex<double>(dnom*zeros[i].real(),-dnom*zeros[i].imag());
            }
        }
    }
    
    if(np > 0){
        for(int i=0;i<np;++i){
            double dnom=abs(poles[i]);
            dnom *= dnom;
            if(dnom > 0){
                con /= sqrt(dnom);
                dnom=w0/dnom;
                poles[i]=complex<double>(dnom*poles[i].real(),-dnom*poles[i].imag());
            }
        }
    }
    
    int nd=np-nz;
    
    if(nd > 0){
        complex<double> *zeros2=new complex<double>[nz+nd];
        if(nz > 0){
            for(int n=0;n<nz;++n){
                zeros2[n]=zeros[n];
            }
            delete [] zeros;
        }
        for(int n=0;n<nd;++n){
            zeros2[n+nz]=complex<double>(0,0);
        }
        
        zeros=zeros2;
        
        nz=nz+nd;
    }
    
    char mes[256];
    
    sprintf(mes,"High Pass Transformation f = %18.9e",f);
    
    writePoly(mes);
    
    return 0;
}
int Poly::low(double f,int inorm)
{
    double pi=4.0*atan(1.0);
    double w0=2.0*pi*f;
    
    if(nz > 0){
        for(int i=0;i<nz;++i){
            con /= w0;
            zeros[i] *= w0;
        }
    }
    if(np > 0){
        for(int i=0;i<np;++i){
            con *= w0;
            poles[i] *= w0;
        }
    }
    
    if(inorm > 0)norm();
    
    char mes[256];
    
    sprintf(mes,"Low Pass Transformation f = %18.9e",f);
    
    writePoly(mes);
    
    return 0;
}
int Poly::march(int nstep,double step,int flag)
{
    fprintf(stderr,"%d %g %d\n",nstep,step,flag);
    
    fprintf(stderr,"          t,              rsr,               rsi \n");
    double t = -step;
    for(int n=0;n<nstep+1;++n){
        t += step;
        double rsr=0.0;
        double rsi=0.0;
        for(int nf=0;nf<nforces;++nf){
            double del=delay[nf];
            if(t < del)continue;
            double rsrr=0.0;
            double rsii=0.0;
            for(int nr=0;nr<pz[nf].np;++nr){
                double exc=pz[nf].rs[nr]*exp(-pz[nf].poles[nr].real()*(t-del));
                double csc=pz[nf].ts[nr]-pz[nf].poles[nr].imag()*(t-del);
                rsrr=rsrr+exc*cos(csc);
                rsii=rsii+exc*sin(csc);
            }
            rsi=rsi+rsii*pz[nf].con;
            rsr=rsr+rsrr*pz[nf].con;
        }
        
        fprintf(stderr,"%18.9e,%18.9e,%18.9e\n",t,rsr,rsi);
    }
    return 1;
}
int Poly::band(double f,int flag,double iter)
{
    double pi=4.0*atan(1.0);
    
    double w0=2.0*pi*f;
    
    double w2=w0*w0;
    
    int np2=2*np;
    int nz2=2*nz+np-nz;
    
    complex<double> *zeros2=new complex<double>[nz2];
    
    complex<double> *poles2=new complex<double>[np2];
    
    for(int n=0;n<np;++n){
        double zr=2.0*poles[n].real();
        double zi=2.0*poles[n].imag();
        double fr=zr*zr-zi*zi-4.0*w2;
        double fi=2.0*zr*zi;
        double r=sqrt(fr*fr+fi*fi);
        double theta=0.0;
        if(r > 0){
            theta=0.5*atan2(fi,fr);
            r=sqrt(r);
        }
        poles2[2*n+1]   = complex<double>(0.5*(zr-r*cos(theta)),0.5*(zi-r*sin(theta)));
        poles2[2*n] = complex<double>(0.5*(zr+r*cos(theta)),0.5*(zi+r*sin(theta)));
    }
    
    for(int n=0;n<nz;++n){
        double zr=2.0*zeros[n].real();
        double zi=2.0*zeros[n].imag();
        double fr=zr*zr-zi*zi-4.0*w2;
        double fi=2.0*zr*zi;
        double r=sqrt(fr*fr+fi*fi);
        double theta=0.0;
        if(r > 0){
            theta=0.5*atan2(fi,fr);
            r=sqrt(r);
        }
        zeros2[2*n+1]   = complex<double>(0.5*(zr-r*cos(theta)),0.5*(zi-r*sin(theta)));
        zeros2[2*n] = complex<double>(0.5*(zr+r*cos(theta)),0.5*(zi+r*sin(theta)));
    }
    
    int nd=np-nz;
    
    if(nd > 0){
        for(int n=0;n<nd;++n){
            con = 2*con;
            zeros2[2*nz+n]=complex<double>(0,0);
        }
    }
    
    np=np2;
    nz=nz2;
    
    delete [] zeros;
    
    zeros=zeros2;
    
    delete [] poles;
    
    poles=poles2;
    
    char mes[256];
    
    sprintf(mes,"Band Transformation f = %18.9e",f);
    
    writePoly(mes);
    
    return 0;
}

int Poly::writePoly(char *name)
{
    if(iprint < 1)return 0;
    
    fprintf(stderr,"\n       %s\n",name);
    
    fprintf(stderr,"            Constant\n");
    fprintf(stderr,"     %18.9e\n",con);
    if(nz > 0){
        fprintf(stderr,"               ZERO COEFFICIENTS\n");
        for(int n=0;n<nz;++n){
            fprintf(stderr,"    %4d, %18.9e,%18.9e\n",n+1,zeros[n].real(),zeros[n].imag());
        }
    }
    if(np > 0){
        fprintf(stderr,"               POLE COEFFICIENTS\n");
        for(int n=0;n<np;++n){
            fprintf(stderr,"    %4d, %18.9e,%18.9e\n",n+1,poles[n].real(),poles[n].imag());
        }
    }
    
    return 0;
}
int Poly::forces(BatchPtr Batch,int nforces)
{
    struct CommandInfo cp;
    char line[256],*command;
    int ret;
    
    zerol((char *)&cp,sizeof(struct CommandInfo));

   fprintf(stderr,"          forces %d\n",nforces);
    
    if(nforces < 1)return 0;
    
    this->nforces=nforces;
    
    if(delay)delete [] delay;
    delay=NULL;
    
    if(coef1)delete [] coef1;
    coef1=NULL;
    
    if(coef2)delete [] coef2;
    coef2=NULL;
    
    if(type)delete [] type;
    type=NULL;

 
    delay=new double(nforces);
    coef1=new double(nforces);
    coef2=new double(nforces);
    type=new int(nforces);
    
    for(int n=0;n<nforces;++n)delay[n]=0;
    
    for(int n=0;n<nforces;++n){
        double v1,v2;
        int n1;
        if(BatchNextLine(Batch,line,sizeof(line)))break;
        if(getCommand(line,&cp))break;
        command=stringCommand(&cp);
        if(!command)goto ErrorOut;

        if(!mstrcmp((char *)"sin",command)){
            type[n]=SIN;
        }else if(!mstrcmp((char *)"cos",command)){
            type[n]=COS;
        }else if(!mstrcmp((char *)"exp",command)){
            type[n]=EXP;
        }else if(!mstrcmp((char *)"impulse",command)){
            type[n]=IMPULSE;
        }else if(!mstrcmp((char *)"step",command)){
            type[n]=STEP;
        }else if(!mstrcmp((char *)"ic",command)){
            type[n]=IC;
        }else if(!mstrcmp((char *)"delay",command)){
            ++(cp.n);
            ret=doubleCommand(&v1,&cp);
            if(ret)goto ErrorOut;
            ++(cp.n);
            ret=doubleCommand(&v2,&cp);
            n1=v1;
            if(n1 >= 0 && n1 < nforces)delay[n]=v2;
            continue;
        }else if(!mstrcmp((char *)"end",command)){
            break;
        }
        ++(cp.n);
        ret=doubleCommand(&coef1[n],&cp);
        if(ret)goto ErrorOut;
        ++(cp.n);
        ret=doubleCommand(&coef2[n],&cp);
        if(ret)coef2[n]=0;
        fprintf(stderr,"     %s   %g    %g\n",command,coef1[n],coef2[n]);
    }
    
    if(BatchNextLine(Batch,line,sizeof(line)))goto ErrorOut;
    if(getCommand(line,&cp))goto ErrorOut;
    command=stringCommand(&cp);
    //fprintf(stderr,"last command %s\n",command);

ErrorOut:
    getCommand(NULL,&cp);

    return 0;
}
int Poly::trans(BatchPtr Batch,int nzero,int npole)
{
    struct CommandInfo cp;
    double value1,value2;
    char line[4096];
    int ret;

    SetPolesAndZeros(npole,nzero);
    
    zerol((char *)&cp,sizeof(struct CommandInfo));

    if(BatchNextLine(Batch,line,sizeof(line)))goto Error;
    if(getCommand(line,&cp))goto Error;
    ret=doubleCommand(&value1,&cp);
    con=value1;

    if(nz > 0){
        for(int n=0;n<nz;++n){
            if(BatchNextLine(Batch,line,sizeof(line)))break;
            if(getCommand(line,&cp))break;
            ret=doubleCommand(&value1,&cp);
            ++(cp.n);
            ret=doubleCommand(&value1,&cp);
            ++(cp.n);
            ret=doubleCommand(&value2,&cp);
            ++(cp.n);
            zeros[n]=complex<double>(value1,value2);
        }
        
    }
    
    if(np > 0){
        for(int n=0;n<np;++n){
            if(BatchNextLine(Batch,line,sizeof(line)))break;
            if(getCommand(line,&cp))break;
            ret=doubleCommand(&value1,&cp);
            ++(cp.n);
            ret=doubleCommand(&value1,&cp);
            ++(cp.n);
            ret=doubleCommand(&value2,&cp);
            ++(cp.n);
            poles[n]=complex<double>(value1,value2);
        }

    }
    
    char *command;
    if(BatchNextLine(Batch,line,sizeof(line)))goto Error;
    if(getCommand(line,&cp))goto Error;
    command=stringCommand(&cp);
    fprintf(stderr,"command %s\n",command);


    char mes[256];
    
    sprintf(mes,"Trans");

    writePoly(mes);
    
    getCommand(NULL,&cp);

    return 0;
Error:
    return 1;
}
int Poly::invert(int flag)
{
    double b;
    int nt;
    
    fprintf(stderr,"invert \n");
    
    if(pz)delete [] pz;

    pz=new poleszeros[nforces];
    
    for(int nf=0;nf < nforces;++nf){
        nt=type[nf];
        b=coef1[nf];
        if(nt == 1)b=b*coef2[nf];
        pz[nf].con=b;
        forcepoleszeros(nf);
        for(int nr=0;nr<pz[nf].np;++nr){
            double r=con;
            double theta=0;
            double rm;
            for(int np=0;np<pz[nf].np;++np){
                if(nr == np)continue;
                complex<double> sig=pz[nf].poles[np]-pz[nf].poles[nr];
                rm=std::abs(sig);
                if(rm > 0){
                    r=r/rm;
                    theta=theta-atan2(sig.imag(),sig.real());
                }
            }
            for(int np=0;np<pz[nf].nz;++np){
                complex<double> sig=pz[nf].zeros[np]-pz[nf].poles[nr];
                r *= std::abs(sig);
                theta=theta+atan2(sig.imag(),sig.real());
            }
            pz[nf].ts[nr]=theta;
            pz[nf].rs[nr]=r;
        }
        
        if(flag == 0)continue;
        
        fprintf(stderr," nf %d con %g delay %g\n",nf,pz[nf].con,delay[nf]);
        for(int nr=0;nr<pz[nf].np;++nr){
            fprintf(stderr,"%4d %18.9e,%18.9e,%18.9e,%18.9e\n",nr,pz[nf].rs[nr],pz[nf].poles[nr].real(),pz[nf].ts[nr],pz[nf].poles[nr].imag());
        }
    }
    
    return 1;
}
int Poly::forcepoleszeros(int nf)
{
    int nt=type[nf];
    
    int npp1=2;
    int nzp=1;
    if(nt == IC)nzp += coef2[nf];
    

    pz[nf].poles=new complex<double>[np+npp1];
    pz[nf].markpole=new int[np+npp1];
    pz[nf].zeros=new complex<double>[nz+nzp];
    pz[nf].markzero=new int[nz+nzp];
    pz[nf].ts=new double[np+npp1];
    pz[nf].rs=new double[np+npp1];

    for(int n=0;n<np;++n){
        pz[nf].poles[n]=poles[n];
    }
    
    for(int n=0;n<nz;++n){
        pz[nf].zeros[n]=zeros[n];
    }
    
    pz[nf].np=np;
    pz[nf].nz=nz;

    
    switch (nt) {
        case SIN:
            pz[nf].poles[pz[nf].np++]=complex<double>(0.0,coef2[nf]);
            pz[nf].poles[pz[nf].np++]=complex<double>(0.0,-coef2[nf]);
            break;
            
        case COS:
            pz[nf].poles[pz[nf].np++]=complex<double>(0.0,coef2[nf]);
            pz[nf].poles[pz[nf].np++]=complex<double>(0.0,-coef2[nf]);
            pz[nf].zeros[pz[nf].nz++]=complex<double>(0.0,0.0);
            break;
            
        case EXP:
            pz[nf].poles[pz[nf].np++]=complex<double>(coef2[nf],0.0);
            break;
            
        case IMPULSE:
            break;
            
        case STEP:
            pz[nf].poles[pz[nf].np++]=complex<double>(0.0,0.0);
            break;
            
        case IC:
        {
            int nn=coef2[nf];
            for(int n=0;n<nn;++n){
               pz[nf].zeros[pz[nf].nz++]=complex<double>(0.0,0.0);
            }
        }
            break;

        default:
            break;
    }
    
    int npp=pz[nf].np;
    int nzz=pz[nf].nz;
    
    
    for(int n=0;n<npp;++n)pz[nf].markpole[n]=0;
    for(int k=0;k<nzz;++k)pz[nf].markzero[k]=0;
    
    for(int n=0;n<npp;++n){
        for(int k=0;k<nzz;++k){
            if((pz[nf].markzero[k] == 0) && (pz[nf].poles[n] == pz[nf].zeros[k])){
                pz[nf].markzero[k]=1;
                pz[nf].markpole[n]=1;
                break;
            }
        }
    }

    pz[nf].np=0;
    for(int n=0;n<npp;++n){
        if(pz[nf].markpole[n] == 1)continue;
        pz[nf].poles[pz[nf].np++]=pz[nf].poles[n];
    }
    
    pz[nf].nz=0;
    for(int k=0;k<nzz;++k){
        if(pz[nf].markzero[k] == 1)continue;
        pz[nf].zeros[pz[nf].nz++]=pz[nf].zeros[k];
    }
    
    return 0;
}
int Poly::wpz()
{
    fprintf(stderr,"con = %18.9e\n",con);
    
    if(nz > 0){
        ;
    }
    
    if(np > 0){
        ;
    }
    
    return 0;
}
int Poly::sweep(double f1,double f2,int ns,int npass,int ilog)
{
    
    double zns=1./(double)(ns);
    
    double df=zns*(f2-f1);
    
    double pi=4.0*atan(1.0);
    
    if(ilog > 0)df=zns*(log10(f2)-log10(f1));
    
    double df10=pow(10,df);
    
    double pi2=2*pi;
    
    double cont=180.0/pi;
    
    npass=1;
    
    ns=ns+1;
    
    int ncount=max(np,nz);
    
    for(int kpass=1;kpass<=npass;++kpass){
        double f=f1;
        int nn=0;
        if(iangle > 0){
            fprintf(stderr,"      FREQUENCY,         ANGLE\n");
        }else{
            fprintf(stderr,"      FREQUENCY,         AMPLITUDE\n");
        }
        for(int ks=1;ks<=ns;++ks){
            nn=nn+1;
            double w=pi2*f;
            double r=con;
            double theta=0.0;
            for(int k=1;k<=ncount;++k){
                if(k <= np){
                    double rp=poles[k-1].real();
                    double wp=poles[k-1].imag();
                    double ws=w+wp;
                    double rl=sqrt(ws*ws+rp*rp);
                    if(rl > 0.0){
                        r=r/rl;
                       theta=theta-atan2(ws,rp);
                    }
                }
                if(k <= nz){
                    double rp=zeros[k-1].real();
                    double wp=zeros[k-1].imag();
                    double ws=w+wp;
                    double rl=sqrt(ws*ws+rp*rp);
                    r=r*rl;
                    if(rl > 0.0)theta=theta+atan2(ws,rp);
            
                }
            }
            if(iangle > 0){
                fprintf(stderr,"%18.9e,%18.9e\n",f,theta*cont);
            }else{
                fprintf(stderr,"%18.9e,%18.9e\n",f,r);
            }
            if(ilog <= 0){
                f=f+df;
            }else{
                f=f*df10;
            }
        }
    }
    return 0;
}

/*
 int Poly::bilinear(double wT)
 {
 warp(wT);
 
 if(nz > 0){
 for(int n=0;n<nz;++n){
 complex<double> sk=zeros[n];
 
 zeros[n]=(1.0+sk)/(1.0-sk);
 }
 }
 
 if(np > 0){
 for(int n=0;n<np;++n){
 complex<double> sk=poles[n];
 poles[n]=(1.0+sk)/(1.0-sk);
 }
 }
 
 char mes[256];
 
 sprintf(mes,"bilinear Coefficients");
 
 writePoly(mes);
 
 
 return 0;
 }
 */





