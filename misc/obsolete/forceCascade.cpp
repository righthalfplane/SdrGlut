int Poly::forceCascade(double *input,int npoint)
{
    double *xnp = (double *)eMalloc(npoint*sizeof(double),20002);
    double *ynp = (double *)eMalloc(npoint*sizeof(double),20003);
    
    for(int k=0;k<cascade;++k){
        biquad[k].d1=0.0;
        biquad[k].d2=0.0;
    }
    
    double scale=1.0;  // normalize at 0 Hz
    
    double t=1.0/sampleRate;
    
    for(int n=0;n<npoint;++n){
        double y;
        double x;
        
        x=input[n];
        
        y=0;
        
        for(int k=0;k<cascade;++k){
            x = x*biquad[k].kk-biquad[k].a1*biquad[k].d1-biquad[k].a2*biquad[k].d2;
            biquad[k].d2=biquad[k].d1;
            biquad[k].d1=x;
            y = x*biquad[k].b0+biquad[k].b1*biquad[k].d1+biquad[k].b2*biquad[k].d2;
            x=y;
        }
        
        xnp[n]=n*t;
        
        ynp[n]=y/scale;
        
        fprintf(stderr,"    %18.9e, %18.9e , %18.9e\n",n*t,y/scale,x);
    }
    
    BatchPlot((char *)"forceCascade",0,xnp,ynp,npoint);
    
    if(xnp)eFree(xnp);
    
    if(ynp)eFree(ynp);
    
    xn=NULL;
    
    yn=NULL;
    
    return 0;
}
