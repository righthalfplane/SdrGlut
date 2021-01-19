int doAudio(float *aBuff,struct playData *rx)
{
	int short *data;
	int audioOut;

	
	pthread_mutex_lock(&rx->mutexo);
	audioOut=rx->audioOut;
	//fprintf(stderr,"audioOut %d\n",audioOut);
	data=rx->buffa[rx->audioOut++ % NUM_ABUFF];
	pthread_mutex_unlock(&rx->mutexo);
	

	double amin=1e30;
	double amax=-1e30;
		
	float *buff=aBuff;

	
	double dmin,dnom,gain;
	
	gain=rx->gain;
	
	if(gain <= 0.0)gain=1.0;
	
	for (int i=0; i<rx->faudio; i++ ) {
		double v;
		v=buff[i];
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}
	
	//fprintf(stderr,"doAudio size %d amin %f amax %f audioOut %d\n",BLOCK_SIZE,amin,amax,audioOut);
	
	//amin=0.0;
	
	dnom=64000.0/(amax-amin);
		
	dmin=amin;

	for(int k=0;k<rx->faudio;++k){
		double v;
		v=gain*buff[k];
		v=(v-dmin)*dnom-32000;
		data[k]=(short int)v;
	}	

	pushBuffa(audioOut,rx);


	return 0;
}
