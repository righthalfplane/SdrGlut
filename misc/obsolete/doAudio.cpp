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
		data[k]=(short int)v;
       // if(v < amin)amin=v;
       // if(v > amax)amax=v;
	}
	
   // fprintf(stderr,"doAudio amin %f amax %f \n",amin,amax);

	pushBuffa(audioOut,rx);

	return 0;
}
