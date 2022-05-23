void cDemod::processAll()
{
	int ip=popBuff(rx);
	if(ip < 0){
		 //printf("process1 wait for data count %ld\n",count++);
		 return;
	}

	//printf("process1 work doWhat2 %d\n",doWhat2);

	int witch=ip % NUM_DATA_BUFF;

	float *buf1=rx->buff[witch];
	float *buf2=b2in;

	double sint,cost;

	for (int k = 0 ; k < rx->size ; k++){
		float r = buf1[k * 2];
		float i = buf1[k * 2 + 1];
		if(rx->dt > 0){
			buf2[k * 2] = (float)(r*rx->coso - i*rx->sino);
			buf2[k * 2 + 1] = (float)(i*rx->coso + r*rx->sino);
			sint=rx->sino*rx->cosdt+rx->coso*rx->sindt;
			cost=rx->coso*rx->cosdt-rx->sino*rx->sindt;
			rx->coso=cost;
			rx->sino=sint;
		 }else{
			buf2[k * 2] = r;
			buf2[k * 2 + 1] = i;
		}


	}
	
	double r=sqrt(rx->coso*rx->coso+rx->sino*rx->sino);
	rx->coso /= r;
	rx->sino /= r;      
	
	num=0;
	num2=0;
	
	msresamp_crcf_execute(f.iqSampler, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf1, &num);  // decimate
 
	if(rx->decodemode < MODE_AM){

		freqdem_demodulate_block(f.demod, (liquid_float_complex *)buf1, (int)num, (float *)buf2);

	}else if(rx->decodemode < MODE_USB){
		#define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

		for(unsigned int n=0;n<num;++n){
			double mag=sqrt(buf1[2*n]*buf1[2*n]+buf1[2*n+1]*buf1[2*n+1]);
			double z0=mag + (f.amHistory * DC_ALPHA);
			buf2[n]=(float)(z0-f.amHistory);
			f.amHistory=z0;
		}
	}else{
		ampmodem_demodulate_block(f.demodAM,  (liquid_float_complex *)buf1, (int)num, (float *)buf2);
	}

	msresamp_rrrf_execute(f.iqSampler2, (float *)buf2, num, (float *)buf1, &num2);  // interpolate
				
	
	float *buff=buf1;

	int short *data;
	int audioOut;


	mutexo.lock();
	audioOut=rx->audioOut;
	//mprint("audioOut %d\n",audioOut);
	data=rx->buffa[rx->audioOut++ % NUM_ABUFF];
	mutexo.unlock();

	double amin=1e30;
	double amax=-1e30;


	double dnom,gain;

	gain=rx->gain;

	if(gain <= 0.0)gain=1.0;

	double average=0;

	for (int i=0; i<rx->faudioCount; i++ ) {
		double v;
		v=buff[i];
		average += v;
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}

	average /= rx->faudio;

	amin -= average;

	amax -= average;


	if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;

	rx->aminGlobal = 0.8*rx->aminGlobal+0.2*amin;

	amin=rx->aminGlobal;

	if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;

	rx->amaxGlobal = 0.8*rx->amaxGlobal+0.2*amax;

	amax=rx->amaxGlobal;


	//mprint("doAudio size %d amin %f amax %f audioOut %d\n",BLOCK_SIZE,amin,amax,audioOut);


	if((amax-amin) > 0){

		dnom=65535.0/(amax-amin);
	}else{

		dnom=65535.0;
	}


	for(int k=0;k<rx->faudioCount;++k){
		double v;

		v=buff[k];

		v=gain*((v-average)*dnom);

		if(v < -32765){
			v = -32765;
		}else if(v > 32765){
			v=32765;
		}

		data[k]=(short int)v;
	}	

	pushBuffa(audioOut,rx);

}
