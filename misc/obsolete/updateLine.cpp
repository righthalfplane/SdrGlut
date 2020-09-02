int Radio::updateLine()
{
    
    double *real,*imag;
    double amin,amax,v;
    
    if(rtime() < lineTime)return 0;

    lineTime=rtime()+lineDumpInterval;
    
    if(rx->controlRF != 2)return 0;
    
    if(rx->FFTcount > FFTlength){
        printf(" FFTlength %ld\n",FFTlength);
        return 1;
    }
    
    int length=rx->FFTcount;
    
    setDialogPower(rx->m_SMeter.GetAve());
    
    real=rx->real;
    imag=rx->imag;
    
    double average=0;
    for(int k=0;k<length;++k){
        average += sqrt(real[k]*real[k]+imag[k]*imag[k]);
    }
    average /= length;
    
   //static int drops=0;
   //if(rx->averageGlobal == 0)drops=0;
    if(rx->averageGlobal == 0)rx->averageGlobal=average;
    rx->averageGlobal = 0.9*rx->averageGlobal+0.1*average;
    if(average < 0.1*rx->averageGlobal){
       // printf("Device '%s' Drop out average %g averageGlobal %g drops %d\n",rx->driveName,average,rx->averageGlobal,++drops);
        return 0;
    }
    
    amin =  1e33;
    amax = -1e33;
    for(int n=0;n<length;++n){
        double v=real[n];
        if(v > amax)amax=v;
        if(v < amin)amin=v;
        v=imag[n];
        if(v > amax)amax=v;
        if(v < amin)amin=v;
    }
    printf("r amin %g amax %g ",amin,amax);

    if(rx->aminGlobal3 == 0.0)rx->aminGlobal3=amin;
    rx->aminGlobal3 = 0.9*rx->aminGlobal3+0.1*amin;
    amin=rx->aminGlobal3;
    
    if(rx->amaxGlobal3 == 0.0)rx->amaxGlobal3=amax;
    rx->amaxGlobal3 = 0.9*rx->amaxGlobal3+0.1*amax;
    amax=rx->amaxGlobal3;
    
    //printf("a amin %g amax %g ",amin,amax);

    double dnom=0.0;
    if((amax-amin) > 0){
        dnom=2.0e-2/(amax-amin);
    }else{
        dnom=2.0e-2;
    }
    
    double dmin=amin;
    
    double gain=0.9;
    
    amin =  1e33;
    amax = -1e33;
    
    double scale=pow(10.0,rx->scaleFactor/20.0);
    
    long int count=0;
    
    for(int n=0;n<length;++n){
        double v;
        v=real[n];
        v=gain*((v-dmin)*dnom-1e-2);
        if(v < amin)amin=v;
        if(v > amax)amax=v;
        if(v < -1e-2){
            v = -1e-2;
            ++count;
        }else if(v > 1e-2){
            v=1e-2;
            ++count;
        }
        real[n]=v*scale;
        v=imag[n];
        v=gain*((v-dmin)*dnom-1e-2);
        if(v < amin)amin=v;
        if(v > amax)amax=v;
        if(v < -1e-2){
            v = -1e-2;
            ++count;
        }else if(v > 1e-2){
            v=1e-2;
            ++count;
        }
        imag[n]=v*scale;
    }
    
    printf("a amin %g amax %g ",amin,amax);
    
    doWindow(real,imag,length,rx->FFTfilter);

    for(int n=0;n<length;++n){
        real[n] *= pow(-1.0,n);
        imag[n] *= pow(-1.0,n);
    }
    
    doFFT2(real,imag,length,1);
    
    amin =  1e33;
    amax = -1e33;

    double rmin=  1e33;
    double rmax= -1e33;
    
    float dx=rx->samplerate;
    double ddx=(double)rx->samplerate/(double)(length);
    long nf=0;
    for(int n=0;n<length;++n){
        double r;
        r=rx->fc-0.5*rx->samplerate+(n+0.5)*ddx;
        if(r < rmin)rmin=r;
        if(r > rmax)rmax=r;
        range[n]=r;
        if(abs(range[n]-rx->f) < dx)
        {
            dx=abs(range[n]-rx->f);
            nf=n;
        }
        v=(real[n]*real[n]+imag[n]*imag[n]);
        if(v > 0.0)v=10*log10(v)+5;
        magnitude[length-n-1]=(1.0-lineAlpha)*magnitude[length-n-1]+v*lineAlpha;
        v=magnitude[length-n-1];
        if(v < amin)amin=v;
        if(v > amax)amax=v;
    }
    
    printf("a amin %g amax %g \n",amin,amax);

    
    if(FindScene(scenel)){
        lines->plotPutData(scenel,range,magnitude,length,0L);
        
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
        
        if(rx->cutOFFSearch){
            for(int k=0;k<length;++k){
                frequencies[k]=range[k];
                if(magnitude[k] > ampitude[k]){
                    ampitude[k]=magnitude[k];
                }
            }
        }
        
        if(scanRun == 1 && scanWait != 1){
            int ifound=0;
            for(vector<double>::size_type k=0;k<scanFrequencies.size();++k){
               scanFound[k]=0;
                int n1=fftIndex(scanFrequencies[k]-0.3*rx->bw);
                int n2=fftIndex(scanFrequencies[k]+0.3*rx->bw);
                if(n1 < 0 || n2 < 0)continue;
                for(int m=n1;m<=n2;++m){
                    if(magnitude[m] > rx->cutOFF){
                        scanFound[k]=1;
                        ifound=1;
                        break;
                    }
                }
            }
            if(ifound){
                if(scanFound[pauseChannel]){
                    if(rtime() < pauseTime)goto FoundTime;
                }
                
                pauseTime=rtime()+pauseTimeDelta;
                if(++pauseChannel >= (int)scanFrequencies.size())pauseChannel=0;
                for(vector<double>::size_type k=pauseChannel;k<scanFrequencies.size();++k){
                    if(scanFound[k]){
                        pauseChannel=(int)k;
                        //fprintf(stderr,"Select channel %d\n",pauseChannel);
                        rx->f=scanFrequencies[k];
                        //fprintf(stderr,"k %d 1 Frequency Selected %.4f\n",(int)k,scanFrequencies[k]/1e6);
                        setFrequency3(rx);
                        goto FoundTime;
                    }
                }
                for(vector<double>::size_type k=0;(int)k<pauseChannel;++k){
                    if(scanFound[k]){
                        pauseChannel=(int)k;
                        //fprintf(stderr,"Select channel %d\n",pauseChannel);
                        rx->f=scanFrequencies[k];
                        //fprintf(stderr,"k %d  2 Frequency Selected %.4f\n",(int)k,scanFrequencies[k]/1e6);
                        setFrequency3(rx);
                        goto FoundTime;
                    }
                }
            }
        }
    }
FoundTime:
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
    
    if(FindScene(scenel2))lines2->plotPutData(scenel2,&range[ns],&magnitude[ns],ne,0L);

    if(water.data == NULL)return 0;
    
    pd.dmin=amin;
    pd.dmax=amax;
    
    
    setDialogRange(amin,amax);
        
    amin=water.amin;
    amax=water.amax;
    
    if(water.nline >= water.ysize)water.nline=0;
    
    FloatToImage(magnitude,length,&pd,water.ic);
    
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