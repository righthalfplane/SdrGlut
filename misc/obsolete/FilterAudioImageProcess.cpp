static int doFilter(struct playData *rx,float *wBuff,float *aBuff,struct Filters *f)
{
 	int ip=popBuff(rx,f);

 	if(ip < 0){

 	     return 1;

 	}

 	 	

 	int witch=ip % NUM_DATA_BUFF5;

 	

 	//fprintf(stderr,"doFilter witch %d ip %d start \n",witch,ip);

	

    float *buf=rx->buff[witch];

    float *buf2=wBuff;

    
    doMix(rx,buf,buf2,f);
    

	buf=aBuff;

    
    int div=4800;
    int step=1+rx->size/div;
    int nn;
    nn=0;
    for(int n=0;n<rx->size;n += step){
        int end=n+step;
        if(end >= rx->size)end=rx->size;
        double sumr=buf2[2*n];
        double sumi=buf2[2*n+1];
        int kk=1;
        for(int k=n+1;k<end;++k){
            sumr += buf2[2*n];
            sumi += buf2[2*n+1];
            ++kk;
        }
        buf[nn++]=sumr/kk;
        buf[nn++]=sumi/kk;
    }

    nn /= 2;
    
    fprintf(stderr,"nn = %ld ",(long)nn);

#define DC_ALPHA 0.99/4.0    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

    for(int n=0;n<nn;++n){
        double mag=sqrt(buf[2*n]*buf[2*n]+buf[2*n+1]*buf[2*n+1]);
        double z0=mag + (f->amHistory * DC_ALPHA);
        buf2[n]=(float)(z0-f->amHistory);
        buf2[n]=mag;
        f->amHistory=z0;
    }
    
    for(int n=0;n<nn;++n){
        buf[n]=buf2[n];
    }
    
    
    return 0;



    unsigned int num;

    unsigned int num2;

    

    num=0;

    num2=0;

    

    msresamp_crcf_execute(f->iqSampler, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf, &num);  // decimate

    

    rx->m_SMeter.ProcessData(num, (TYPECPX *)buf, (TYPEREAL)rx->bw);

        

    if(rx->decodemode < MODE_AM){

		freqdem_demodulate_block(f->demod, (liquid_float_complex *)buf, (int)num, (float *)buf2);

        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate

        //printf("2 rx->size %d num %u num2 %u\n",rx->size,num,num2);

    }else if(rx->decodemode < MODE_USB){
        #define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

        for(unsigned int n=0;n<num;++n){
            double mag=sqrt(buf[2*n]*buf[2*n]+buf[2*n+1]*buf[2*n+1]);
            double z0=mag + (f->amHistory * DC_ALPHA);
            buf2[n]=(float)(z0-f->amHistory);
            f->amHistory=z0;
        }
        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
    }else{
        ampmodem_demodulate_block(f->demodAM,  (liquid_float_complex *)buf, (int)num, (float *)buf2);
        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
   }
    
    fprintf(stderr,"rx->size %ld num %ld bw %ld num2 %ld\n",(long)rx->size,(long)num,(long)rx->bw,(long)num2);
    

	//iirfilt_crcf_execute_block(f->dcFilter, (liquid_float_complex *)buf, num, (liquid_float_complex *)buf);

	

	//iirfilt_crcf_execute_block(f->lowpass, (liquid_float_complex *)buf, num2, (liquid_float_complex *)buf);



	//fprintf(stderr,"doFilter witch %d end num %d Ratio %f size %d num2 %d \n",witch,num,rx->Ratio,rx->size,num2);

    

	return 0;

}

static int doAudio(float *aBuff,struct playData *rx)
{
	int short *data;
	int audioOut;

	
	rx->mutexo.lock();
	audioOut=rx->witchAudioBuffer;
	//fprintf(stderr,"witchAudioBuffer %d\n",audioOut);
	data=rx->buffa[rx->witchAudioBuffer++ % NUM_ABUFF5];
	rx->mutexo.unlock();
		
	float *buff=aBuff;

	
	double dmin,dnom,gain;
    
/*

	for (int i=0; i<BLOCK_SIZE; i++) {
        agc_rrrf_execute(rx->agc, buff[i], &buff[i]);
    }
*/
    
	gain=rx->gain;
	
	if(gain <= 0.0)gain=1.0;
    
    double amin=1e30;
    double amax=-1e30;
    
	for (int i=0; i<BLOCK_SIZE5; i++ ) {
		double v;
        v=buff[i];
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}

	//fprintf(stderr,"doAudio amin %f amax %f \n",amin,amax);

    if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;
    rx->aminGlobal = 0.9*rx->aminGlobal+0.1*amin;
    amin=rx->aminGlobal;
    
    if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;
    rx->amaxGlobal = 0.9*rx->amaxGlobal+0.1*amax;
    amax=rx->amaxGlobal;

   // fprintf(stderr,"amax %g amaxg %g amin %g aming %g\n",amax,rx->amaxGlobal,amin,rx->aminGlobal);

    if((amax-amin) > 0){
        dnom=65535.0/(amax-amin);
    }else{
        dnom=65535.0;
    }
		
	dmin=amin;
    //amin=1e30;
   // amax=-1e30;
#define B_DIV 10
#define B_XSIZE BLOCK_SIZE5/B_DIV
#define B_YSIZE 258

    double *b = new double[B_YSIZE*B_XSIZE];
    
    double *bpmax = new double[B_XSIZE];
    double *bnmax = new double[B_XSIZE];

    for(int k=0;k<B_YSIZE*B_XSIZE;++k){
        b[k]=0;
    }
    int nnnp=0;
    int nnnn=0;
	for(int k=0;k<BLOCK_SIZE5;++k){
		double v;
        v=buff[k];
		v=gain*((v-dmin)*dnom-32768);
        if(rx->mute)v=0.0;
        if(v < -32765){
            v = -32765;
        }else if(v > 32765){
            v=32765;
        }
        short int s=(short int)v;
        data[k]=(short int)s;
        int nn=k/B_DIV;
        int ne;
        if(s > 0){
            ne=(int)(s*127.0/32765.0);
            nnnp += ne;
           if(nn >= B_XSIZE)fprintf(stderr,"1 too big nn %d\n",nn);
            if(ne >= 128)fprintf(stderr,"1 too big ne %d\n",ne);
            for(int n=0;n<ne;++n){
                b[nn*B_YSIZE+(n+128)]++;
            }
        }else{
            ne=(int)(s*127.0/32765.0);
            ne = -ne;
            nnnn += ne;
            if(nn >= B_XSIZE)fprintf(stderr,"2 too big nn %d\n",nn);
            if(ne >= 128)fprintf(stderr,"2 too big ne %d\n",ne);
            for(int n=0;n<ne;++n){
                b[nn*B_YSIZE+n]++;
            }

        }
       // if(v < amin)amin=v;
       // if(v > amax)amax=v;
	}
    
    fprintf(stderr,"nnnp %d nnnn %d ",nnnp,nnnn);

    int n5p=0;
    int n5n=0;

    for(int k=0;k<B_XSIZE;++k){
        bpmax[k] = -1e33;
        bnmax[k] =  1e33;
    }
    
    for(int k=0;k<B_XSIZE;++k){
        for(int n=0;n<128;++n){
            if(b[k*B_YSIZE+(n+128)] > 5){
                n5p++;
                if(n*32765.0/127.0 > bpmax[k]){
                    bpmax[k]=n*32765.0/127.0;
                }
            }
        }
        
        for(int n=0;n<128;++n){
            if(b[k*B_YSIZE+n] > 5){
                n5n++;
                if(-n*32765.0/127.0 < bnmax[k]){
                    bnmax[k]= -n*32765.0/127.0;
                }
            }
        }
    }
    
    for(int k=0;k<BLOCK_SIZE5;++k){
        int nn=k/B_DIV;
        if(data[k] > 0.0){
            if(data[k] > bpmax[nn])data[k]=bpmax[nn];
        }else{
            if(data[k] < bnmax[nn])data[k]=bnmax[nn];
       }

    }
    fprintf(stderr,"n5p %d n5n %d\n",n5p,n5n);
	
   // fprintf(stderr,"doAudio amin %f amax %f \n",amin,amax);
    
    delete [] b;

	pushBuffa(audioOut,rx);

	return 0;
}
