int doCLowPass(BatchPtr Batch,struct CommandInfo *cp)
{
    complex<double> *poles;
    complex<double> *zeros;

    char *command;
    double value,ripple,fc;
    int order;
    int ret;
    
    struct Poly *pl=Batch->myIcon->pl;
    
    ++(cp->n);
    command=stringCommand(cp);
    if(!command)goto ErrorOut;
    ++(cp->n);
    ret=doubleCommand(&value,cp);
    if(ret)goto ErrorOut;
    ++(cp->n);
    order=value;
    ret=doubleCommand(&value,cp);
    if(ret)goto ErrorOut;
    ++(cp->n);
    ripple=value;
    ret=doubleCommand(&value,cp);
    if(ret)goto ErrorOut;
    ++(cp->n);
    fc=value;
    
    if(!mstrcmp((char *)"chev",command)){
        pl->doChev(order,ripple);
    }else if(!mstrcmp((char *)"butter",command)){
        pl->doButterWorth(order);
    }
    
    poles=pl->poles;
    zeros=pl->zeros;

    pl->poles=(complex<double> *)eMalloc(2*sizeof(complex<double>),20032);
    if(pl->zeros)eFree(pl->zeros);
    pl->zeros=NULL;
    pl->np=2;
    pl->nz=0;
    
    if(order & 1){
        ;
    }else{
        int np=order/2;
        
        pl->cfore=(double **)eMalloc(np*sizeof(double *),20032);
        pl->cback=(double **)eMalloc(np*sizeof(double *),20032);
        
        pl->cascade=np;

        for(int k=0;k<np;++k){
            int nl=order-k-1;
            printf("k %d %g %g %g %g\n",k,poles[k].real(),poles[k].imag(),poles[nl].real(),poles[nl].imag());
            pl->poles[0]=poles[k];
            pl->poles[1]=poles[nl];
            pl->bilinear(fc);
            pl->diff();
            pl->cfore[k]=pl->fore;
            pl->cback[k]=pl->back;
            pl->fore=NULL;
            pl->back=NULL;
        }
    }

ErrorOut:
    return 0;
}

int Poly::forceCascade(double *input,int npoint)
{
    double *xnp = (double *)eMalloc(npoint*sizeof(double),20002);
    double *ynp = (double *)eMalloc(npoint*sizeof(double),20003);

    double **xnn=(double **)eMalloc(cascade*sizeof(double),20003);
    double **ynn=(double **)eMalloc(cascade*sizeof(double),20003);
    
    nfore=3;
    nback=3;
    
    for(int k=0;k<cascade;++k){

        xn = (double *)eMalloc(nfore*sizeof(double),20003);
        
        xnn[k]=xn;
    
        for(int n=0;n<nfore;++n)xn[n]=0;
    
        yn = (double *)eMalloc(nback*sizeof(double),20004);
    
        for(int n=0;n<nback;++n)yn[n]=0;
        
        ynn[k]=yn;

    }

    double scale=1.0;  // normalize at 0 Hz
    
    double t=1.0/sampleRate;

    for(int n=0;n<npoint;++n){
        double y;
        double x;
        
        x=input[n];
        
        y=0;
        
        for(int k=0;k<cascade;++k){
            xn=xnn[k];
            yn=ynn[k];
            fore=cfore[k];
            back=cback[k];
            
            y=x*fore[nfore-1];
            
            for(int k=1;k<nfore;++k){
                y += xn[k-1]*fore[nfore-k-1];
            }
            
            for(int k=1;k<nback;++k){
                y -= yn[k-1]*back[nback-k-1];
            }
            
            for(int k=0;k<nback-1;++k){
                yn[nback-k-1]=yn[nback-k-2];
            }
            yn[0]=y;
            
            for(int k=0;k<nfore-1;++k){
                xn[nfore-k-1]=xn[nfore-k-2];
            }
            xn[0]=x;
            
            x=y;


        }

        xnp[n]=n*t;
        
        ynp[n]=y/scale;
        
        fprintf(stderr,"    %18.9e, %18.9e , %18.9e\n",n*t,y/scale,x);
    }

    
    BatchPlot((char *)"forceCascade",0,xnp,ynp,npoint);
    
    for(int k=0;k<cascade;++k){
        if(xnn[k])eFree(xnn[k]);
        if(ynn[k])eFree(ynn[k]);
    }

    if(xnn)eFree(xnn);
    
    if(ynn)eFree(ynn);

    if(xnp)eFree(xnp);
    
    if(ynp)eFree(ynp);
    
    xn=NULL;
    
    yn=NULL;
    
   return 0;
}