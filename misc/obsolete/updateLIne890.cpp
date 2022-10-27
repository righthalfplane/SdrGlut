int SdrFile::updateLine()
{
    
    double *real,*imag;
    double amin,amax,v;
    
    if(rtime() < lineTime)return 0;
    
    lineTime=rtime()+lineDumpInterval;
    
    if(play.FFTcount > FFTlength){
        printf(" FFTcount %d error\n",play.FFTcount);
        return 1;
    }
    
    real=play.real;
    imag=play.imag;
    
    double average=0;
    for(int k=0;k<play.FFTcount;++k){
        average += sqrt(real[k]*real[k]+imag[k]*imag[k]);
    }
    average /= play.FFTcount;
    
    if(play.averageGlobal == 0)play.averageGlobal=average;
    play.averageGlobal = 0.9*play.averageGlobal+0.1*average;
    if(average < 0.1*play.averageGlobal){
        //fprintf(stderr,"Drop out %g \n",average);
        return 0;
    }
    // fprintf(stderr,"average %g averageGlobal %g\n",average,rx->averageGlobal);
    
    int length=play.FFTcount;
    
    doWindow(real,imag,length,play.FFTfilter);
    
    for(int n=0;n<length;++n){
        real[n] *= pow(-1.0,n);
        imag[n] *= pow(-1.0,n);
    }
    
    doFFT2(real,imag,length,1);
    
    amin =  0.0;
    int nn=0;
    for(int n=10;n<length-10;++n){
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        double mag=(1.0-lineAlpha)*limag[length-n-1]+v*lineAlpha;
        amin +=  mag;
        ++nn;
    }
    
    amin /= nn;
    
    double shift=-90-amin;
    
    if(play.aminGlobal3 == 0.0)play.aminGlobal3=shift;
    play.aminGlobal3 = 0.9*play.aminGlobal3+0.1*shift;
    //shift=play.aminGlobal3;
    
    //printf("shift %g amin %g ",shift,amin);
    
    amin =  1e33;
    amax = -1e33;
    
    double rmin=  1e33;
    double rmax= -1e33;
    
    float dx=play.samplerate;
    long nf=0;
    for(int n=0;n<length;++n){
        double r;
        r=play.fc-0.5*play.samplerate+n*(double)play.samplerate/(double)(length-1);
        if(r < rmin)rmin=r;
        if(r > rmax)rmax=r;
        range[n]=r;
        if(abs(range[n]-play.f) < dx)
        {
            dx=abs(range[n]-play.f);
            nf=n;
        }
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        limag[length-n-1]=(1.0-lineAlpha)*limag[length-n-1]+v*lineAlpha+shift;
        v=limag[length-n-1];
        lreal[length-n-1]=v+play.scaleFactor;
        if(v < amin)amin=v;
        if(v > amax)amax=v;
    }
    
    if(FindScene(scenel)){
        lines->plotPutData(scenel,range,lreal,length,0L);
        
        uGridPlotPtr Plot;
        Plot=lines->lines->Plot;
        if(!Plot)return 1;
        
        Plot->xAutoMaximum=TRUE;
        Plot->xAutoMinimum=TRUE;
        
        GridPlotScale(Plot);
        
        Plot->xAutoMaximum=FALSE;
        Plot->xAutoMinimum=FALSE;
        Plot->xSetMaximum=rmax;
        Plot->xSetMinimum=rmin;
    }
    
    long ns,ne,nsub;
    nsub=length/20;
    ns=nf-nsub;
    if(ns < 0)ns=0;
    ne=nf+nsub;
    if(ne >= length){
        ne=length-1-nf+nsub;
    }else{
        ne=2*nsub+1;
    }
    
    double meterMax=lreal[nf];
    
    for(int n=0;n<length;++n){
        if(n > nf-5 && n < nf+5){
            if(lreal[n] > meterMax)meterMax=lreal[n];
        }
    }
    
    setDialogPower(meterMax);
    
    play.meterMax=meterMax;
    
    
    if(FindScene(scenel2))lines2->plotPutData(scenel2,&range[ns],&lreal[ns],ne,0L);
    
    if(water.data == NULL)return 0;
    
    pd.dmin=amin;
    pd.dmax=amax;
    
    
    setDialogRange(amin,amax);
    
    amin=water.amin;
    amax=water.amax;
    
    if(water.nline >= water.ysize)water.nline=0;
    
    FloatToImage(lreal,length,&pd,water.ic);
    
    int ns1=3*water.xsize*(water.ysize-water.nline-1);
    int ns2=3*water.xsize*water.ysize+3*water.xsize*(water.ysize-1-water.nline++);
    
    
    // fprintf(stderr,"water length %ld\n",length);
    
    for(int n=2;n<length-2;++n){
        int ic;
        
        ic=water.ic[n];
        if(water.ic[n-1] > ic)ic=water.ic[n-1];
        if(water.ic[n-2] > ic)ic=water.ic[n-2];
        if(water.ic[n+1] > ic)ic=water.ic[n+1];
        if(water.ic[n+2] > ic)ic=water.ic[n+2];
        
        water.data[ns1+3*n]=pd.palette[3*ic];
        water.data[ns1+3*n+1]=pd.palette[3*ic+1];
        water.data[ns1+3*n+2]=pd.palette[3*ic+2];
        
        
        
        water.data[ns2+3*n]=pd.palette[3*ic];
        water.data[ns2+3*n+1]=pd.palette[3*ic+1];
        water.data[ns2+3*n+2]=pd.palette[3*ic+2];
        
    }
    
    InvalRectMyWindow(scene);
    
    return 0;
}
