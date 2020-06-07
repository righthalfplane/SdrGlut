#include <stdio.h>

#include <complex>

#include <iostream>

#include <liquid/liquid.h>

#include <RtAudio.h>

#include <unistd.h>

// c++ -o receive760000 receive760000.cpp -lrtaudio -lliquid

using namespace std;

struct BiQuad{
    double kk;
    double b0;
    double b1;
    double b2;
    double a1;
    double a2;
    double dx1;
    double dx2;
    double dy1;
    double dy2;
};

struct BiQuad biquad[] = {
                            {   1.573576559e-02,    1.000000000e+00,   -0.000000000e+00,   -1.000000000e+00,    1.432218973e+00,    9.937907992e-01},
                            {   1.558147139e-02,    1.000000000e+00,   -0.000000000e+00,   -1.000000000e+00,    1.474419369e+00,    9.939906632e-01},
                            {   1.563561852e-02,    1.000000000e+00,   -0.000000000e+00,   -1.000000000e+00,    1.427357876e+00,    9.824601835e-01},
                            {   1.550789858e-02,    1.000000000e+00,   -0.000000000e+00,   -1.000000000e+00,    1.463160248e+00,    9.829373962e-01},
                            {   1.554850491e-02,    1.000000000e+00,   -0.000000000e+00,   -1.000000000e+00,    1.427242377e+00,    9.739786122e-01},
                            {   1.546468388e-02,    1.000000000e+00,   -0.000000000e+00,   -1.000000000e+00,    1.451178213e+00,    9.744518899e-01},
                            {   1.548637214e-02,    1.000000000e+00,   -0.000000000e+00,   -1.000000000e+00,    1.431775295e+00,    9.695554582e-01},
                            {   1.545721946e-02,    1.000000000e+00,   -0.000000000e+00,   -1.000000000e+00,    1.440182989e+00,    9.697506321e-01}
                         };

double sampleRate=2e+06;

double thetaNorm=2.38756;

double lowCorner=755000;

double highCorner=765000;

int cascaderesponce(struct BiQuad *biquad,long cascade,int steps);

int cascadestart(struct BiQuad *biquad,long cascade);

int cascadesine(struct BiQuad *biquad,long cascade,double frequency,int steps);

int cascadeforce(struct BiQuad *biquad,long cascade,double *input,int npoint,double frequency);

int size;

int cascadego(struct BiQuad *biquad,long cascade,complex<float> *input,complex<float> *output,long steps);



struct playData{
     int wShift;
    int Debug;
    float gain;
    double fc;
    double f;
    double dt;
    double sino;
    double coso;
    double sindt;
    double cosdt;
    double w;
    int channels;
    int size;
    volatile int ibuff;
	complex<float> *output;
	complex<float> *buff;
	float Ratio;
	double fOut;
	msresamp_crcf iqSampler;
    iirfilt_crcf lowpass;
    int samplerate;
    double aminGlobal;
    double amaxGlobal;
};

struct playData play;


int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );



int mix(struct playData *play,float *buf1,float *buf2);

int setFilters(struct playData *rx);

int main(int argv,char *argc[])
{
    long cascade=(long)(sizeof(biquad)/sizeof(struct BiQuad));
    
    FILE *infile;

    fprintf(stdout,"#\n#      sampleRate %g cascade %ld\n",sampleRate,cascade);

	fprintf(stdout,"sizeof(complex<double>) %ld\n",(long)sizeof(complex<double>));

	fprintf(stdout,"sizeof(complex<float>) %ld\n",(long)sizeof(complex<float>));

    cascadestart(&biquad[0],cascade);
    
    size=(0.5+0.8533*sampleRate/10);
    
    complex<float> *buff= new complex<float>[size];
    
    complex<float> *output= new complex<float>[size];
    
    fprintf(stdout,"size %d\n",size);
    
    play.fc = 1e6;
    play.f  = 760000;
    play.channels=2;
    play.size=size;
    play.output=buff;
    play.buff=output;
    play.ibuff=0;
    play.fOut=48000;
    play.samplerate=sampleRate;
    play.aminGlobal=0.0;
    play.amaxGlobal=0.0;

    {
        float pi;
        pi=4.0*atan(1.0);
        play.dt=1.0/(double)sampleRate;
        play.sino=0;
        play.coso=1;
        play.w=2.0*pi*(play.fc - play.f);
        play.sindt=sin(play.w*play.dt);
        play.cosdt=cos(play.w*play.dt);
    }
  
    
    setFilters(&play);
    
    
	RtAudio dac;
	if ( dac.getDeviceCount() < 1 ) {
		std::cout << "\nNo audio devices found!\n";
		exit( 0 );
	}
	RtAudio::StreamParameters parameters;
	parameters.deviceId = dac.getDefaultOutputDevice();
	// parameters.deviceId = 7;
	parameters.nChannels = 2;
	parameters.firstChannel = 0;
	unsigned int sampleRate = 48000;
	unsigned int bufferFrames = 4096;


	try {
		dac.openStream( &parameters, NULL, RTAUDIO_SINT16,
						sampleRate, &bufferFrames, &sound, (void *)&play );
		dac.startStream();
	}
	catch ( RtAudioError& e ) {
		e.printMessage();
		exit( 0 );
	}
    
     if ((infile = fopen (argc[1], "rb")) == NULL)
    {
        printf ("Not able to open input file %s.\n", argc[1]) ;
        return 1 ;
    }
    
 	int nb=0;
 	
   	while((fread(output,sizeof(complex<float>),size,infile)) == size){
   	
  //		cascadego(biquad,cascade,buff,output,size);
   	
   		mix(&play,(float *)output,(float *)buff);
   		
   		play.ibuff=1;
   		
   		while(play.ibuff==1)usleep(10);
   		
   		
   		++nb;
   	}
   	
    fprintf(stdout,"nb %d\n",nb);
   
   	if(infile)fclose(infile);
   	
	try {
    // Stop the stream
    dac.stopStream();
  }
  catch (RtAudioError& e) {
    e.printMessage();
  }
  if ( dac.isStreamOpen() ) dac.closeStream();

    return 0;

}
int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i, j;
  short int *buffer = (short int *) outputBuffer;
    
  struct playData *rx=(struct playData *)userData;
  
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;
    
    int nskip=rx->size/nBufferFrames;
  
	if (rx->ibuff >= 0){
	
		fprintf(stderr,"rx->size %d\n",rx->size);
	    unsigned int num;
        msresamp_crcf_execute(rx->iqSampler, (liquid_float_complex *)rx->output, rx->size, (liquid_float_complex *)rx->buff, &num);
    /*
		for(unsigned int k=0;k<num;++k){ 
			liquid_float_complex v;
			v=rx->buff[k];
			iirfilt_crcf_execute(rx->lowpass,v, (liquid_float_complex *)&rx->output[k]);
		}
	*/
        fprintf(stderr,"rx->size %d num %d\n",rx->size,num);
        
 	    double amin =  1e33;
	    double amax = -1e33;
	    
 	    double vmin =  1e33;
	    double vmax = -1e33;
	    
	    
	    int kk=0;
	    
	    for(int n=0;n<num;++n){
	    
	        double value = abs(rx->buff[n]);
	        
	        if(value < amin)amin=value;
	        if(value > amax)amax=value;
	        
			++kk;
			if(kk < nBufferFrames){
				double vv=(32000.0*rx->buff[n].real()/0.02);
				if(vv < vmin)vmin=vv;
	        	if(vv > vmax)vmax=vv;

				short int v=(short int)(vv);
				//short int v=(short int)(2.0*32000.0*value/.002);
				buffer[2*kk] = v;
				buffer[2*kk+1] = v;
			}
	    
	    }
	    
	    rx->ibuff=0;
	    printf("sound rx-size %d amin %g amain %g nBufferFrames %d kk %d vmin %g vmax %g\n",rx->size,amin,amax,nBufferFrames,kk,vmin,vmax);
    
    	rx->ibuff=0;

	}else{
  
	for ( i=0; i<nBufferFrames; i++ ) {
		for ( j=0; j<2; j++ ) {
		  *buffer++ = 0;
		}
	}
	  printf("sound nBufferFrames %d\n",nBufferFrames);

  }
  
    
  return 0;
}

int mix(struct playData *play,float *buf1,float *buf2)
{

    double sint,cost;
    
    for (int k = 0 ; k < play->size ; k++){
        float r = buf1[k * play->channels];
        float i = buf1[k * play->channels + 1];
        //r = 0.001*(rand() % 100);
        //i = 0.001*(rand() % 100);
        if(play->dt > 0){
            buf2[k * play->channels] = (float)(r*play->coso - i*play->sino);
            buf2[k * play->channels + 1] = (float)(i*play->coso + r*play->sino);
            sint=play->sino*play->cosdt+play->coso*play->sindt;
            cost=play->coso*play->cosdt-play->sino*play->sindt;
            play->coso=cost;
            play->sino=sint;
        }else{
            buf2[k * play->channels] = r;
            buf2[k * play->channels + 1] = i;
        }
    }
    
    double rr=sqrt(play->coso*play->coso+play->sino*play->sino);
    play->coso /= rr;
    play->sino /= rr;
    
	return 0;
}

int setFilters(struct playData *rx)
{

	float shift=rx->f-rx->fc;

    
    float As = 60.0f;
            
    rx->Ratio = rx->fOut / rx->samplerate;
    
    //rx->Ratio = 40960.0 / rx->samplerate;
        
    rx->iqSampler = msresamp_crcf_create(rx->Ratio, As);
    
	msresamp_crcf_print(rx->iqSampler);
	
    rx->lowpass = iirfilt_crcf_create_lowpass(6,0.104);    // +- 5000 HZ
        
	return 0;
	
}


int cascadego(struct BiQuad *biquad,long cascade,complex<float> *input,complex<float> *output,long steps)
{

    double scale=1.0;  // normalize at 0 Hz

    double t=1.0/sampleRate;
 
    for(int n=0;n<steps;++n){
        double y;
        double x;
 
        x=input[n].real();

        y=0;
 
        for(int k=0;k<cascade;++k){
            x=x*biquad[k].kk;
            y=x*biquad[k].b0+biquad[k].dx1*biquad[k].b1+biquad[k].dx2*biquad[k].b2-
            biquad[k].dy1*biquad[k].a1-biquad[k].dy2*biquad[k].a2;
            biquad[k].dx2=biquad[k].dx1;
            biquad[k].dx1=x;
            biquad[k].dy2=biquad[k].dy1;
            biquad[k].dy1=y;
            x=y;
        }
        
        output[n]=y/scale;
    }

    return 0;
}

int cascadestart(struct BiQuad *biquad,long cascade)
{
    for(int k=0;k<cascade;++k){
        biquad[k].dx1=0.0;
        biquad[k].dx2=0.0;
        biquad[k].dy1=0.0;
        biquad[k].dy2=0.0;
    }
    
    return 0;
}
int cascadesine(struct BiQuad *biquad,long cascade,double frequency,int steps)
{
    double *force = new double[steps];

    double pi=4.0*atan(1.0);

    for(int n=0;n<steps;++n){

        force[n]=sin(2*pi*frequency*n/(double)sampleRate);
    }

    cascadeforce(biquad,cascade,force,steps,frequency);

    delete [] force;

    return 0;

}
int cascadeforce(struct BiQuad *biquad,long cascade,double *input,int steps,double frequency)
{

    double scale=1.0;  // normalize at 0 Hz

    double t=1.0/sampleRate;
 
    fprintf(stdout,"plot %d \"Sine Responce Frequency %g\"\n",steps,frequency);
    fprintf(stdout,"           SECONDS,           AMPLITUDE,             INPUT\n");
  
    for(int n=0;n<steps;++n){
        double y;
        double x;
 
        x=input[n];

        y=0;
 
        for(int k=0;k<cascade;++k){
            x=x*biquad[k].kk;
            y=x*biquad[k].b0+biquad[k].dx1*biquad[k].b1+biquad[k].dx2*biquad[k].b2-
            biquad[k].dy1*biquad[k].a1-biquad[k].dy2*biquad[k].a2;
            biquad[k].dx2=biquad[k].dx1;
            biquad[k].dx1=x;
            biquad[k].dy2=biquad[k].dy1;
            biquad[k].dy1=y;
            x=y;
        }

        fprintf(stdout,"    %18.9e, %18.9e , %18.9e\n",n*t,y/scale,x);
    }

    return 0;
}
int cascaderesponce(struct BiQuad *biquad,long cascade,int steps)
{
 
    complex<double> sum;

    double pi=4.0*atan(1.0);
 
    double dt=(pi)/(steps-1);

    fprintf(stdout,"#\n#      cresponse thetaNorm %g \n#\n",thetaNorm);
 
    fprintf(stdout,"plot %d \"Frequency Sweep\" \n",steps);
    fprintf(stdout,"      FREQUENCY,         AMPLITUDE\n");
 
    for(int k=0;k<steps;++k){
        double theta=k*dt;
        complex<double> z = exp(complex<double>(0,theta));
        sum = 1.0;
        for(int k=0;k<cascade;++k){
            sum=sum*biquad[k].kk*(z*z*biquad[k].b0+z*biquad[k].b1+biquad[k].b2)/(z*z+z*biquad[k].a1+biquad[k].a2);
        }
        fprintf(stdout,"%18.9e, %18.9e %18.9e\n",theta*sampleRate/(2*pi),abs(sum),theta/pi);
    }

    return 0;
}
int main2(int argv,char *argc[])
{
    long cascade=(long)(sizeof(biquad)/sizeof(struct BiQuad));

    fprintf(stdout,"#\n#      sampleRate %g cascade %ld\n",sampleRate,cascade);

	fprintf(stdout,"sizeof(complex<double>) %ld\n",(long)sizeof(complex<double>));

	fprintf(stdout,"sizeof(complex<float>) %ld\n",(long)sizeof(complex<float>));

    cascaderesponce(&biquad[0],cascade,1101);

    cascadestart(&biquad[0],cascade);

    cascadesine(&biquad[0],cascade,lowCorner*0.5,1501);

    cascadesine(&biquad[0],cascade,0.5*(lowCorner+highCorner),1501);

    cascadesine(&biquad[0],cascade,0.5*(highCorner+0.5*sampleRate),1501);

    return 0;

}
int soundold( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i, j;
  short int *buffer = (short int *) outputBuffer;
    
  struct playData *rx=(struct playData *)userData;
  
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;
    
    int nskip=rx->size/nBufferFrames;
  
	if (rx->ibuff >= 0){
	    double amin =  1e33;
	    double amax = -1e33;
	    
	    int kk=0;
	    
	    for(int n=0;n<rx->size;++n){
	    
	        double value = abs(rx->output[n]);
	        
	        if(value < amin)amin=value;
	        if(value > amax)amax=value;
	        
	        if(!(n % nskip)){
	        	++kk;
	        	if(kk < nBufferFrames){
		    		//short int v=(short int)(2.0*32000.0*rx->output[n].real()/.002);
		    		short int v=(short int)(2.0*32000.0*value/.002);
		    		buffer[2*kk] = v;
		    		buffer[2*kk+1] = v;
	        	}
	        }
	    }
	    
	    rx->ibuff=0;
	    printf("sound rx-size %d amin %g amain %g nBufferFrames %d kk %d\n",rx->size,amin,amax,nBufferFrames,kk);

	}else{
  
	for ( i=0; i<nBufferFrames; i++ ) {
		for ( j=0; j<2; j++ ) {
		  *buffer++ = 0;
		}
	}
	  printf("sound nBufferFrames %d\n",nBufferFrames);

  }
  
    
  return 0;
}
int sound3Old( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i, j;
  short int *buffer = (short int *) outputBuffer;
    
  struct playData *rx=(struct playData *)userData;
  
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;
    
    int nskip=rx->size/nBufferFrames;
  
	if (rx->ibuff >= 0){
	
	    double amin =  1e33;
	    double amax = -1e33;
	    
	    int kk=0;
	    
	    for(int n=0;n<rx->size;++n){
	    
	        double value = abs(rx->output[n]);
	        
	        if(value < amin)amin=value;
	        if(value > amax)amax=value;
	        
	        if(!(n % nskip)){
	        	++kk;
	        	if(kk < nBufferFrames){
	        	     rx->buff[kk]=rx->output[n];
	        	}else{
	        		kk=nBufferFrames;
	        	}
	        }
	    }
	    
	    printf("sound rx-size %d amin %g amain %g nBufferFrames %d kk %d\n",rx->size,amin,amax,nBufferFrames,kk);
	
	
	      
		for(unsigned int k=0;k<kk;++k){  // Lowpass filter
			liquid_float_complex v;
			v=rx->buff[k];
			iirfilt_crcf_execute(rx->lowpass,v, (liquid_float_complex *)&rx->output[k]);
		}
        //printf("rx->size %d num %d\n",rx->size,num);
        
 	    amin =  1e33;
	    amax = -1e33;
	    
 	    double vmin =  1e33;
	    double vmax = -1e33;
	    
	    for(int n=0;n<kk;++n){
	    
	        double value = abs(rx->output[n]);
	        
	        if(value < amin)amin=value;
	        if(value > amax)amax=value;
	        
				double vv=(32000.0*rx->output[n].real()/0.04);
				if(vv < vmin)vmin=vv;
	        	if(vv > vmax)vmax=vv;

				short int v=(short int)(vv);
				//short int v=(short int)(2.0*32000.0*value/.002);
				buffer[2*n] = v;
				buffer[2*n+1] = v;

	    
	    }
	    
	    rx->ibuff=0;
	    printf("sound rx-size %d amin %g amain %g nBufferFrames %d kk %d vmin %g vmax %g\n",rx->size,amin,amax,nBufferFrames,kk,vmin,vmax);
    
    	rx->ibuff=0;

	}else{
  
	for ( i=0; i<nBufferFrames; i++ ) {
		for ( j=0; j<2; j++ ) {
		  *buffer++ = 0;
		}
	}
	  printf("sound nBufferFrames %d\n",nBufferFrames);

  }
  
    
  return 0;
}
int soundOld5( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i, j;
  short int *buffer = (short int *) outputBuffer;
    
  struct playData *rx=(struct playData *)userData;
  
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;
    
    int nskip=rx->size/nBufferFrames;
  
	if (rx->ibuff >= 0){
	    unsigned int num;
        msresamp_crcf_execute(rx->iqSampler, (liquid_float_complex *)rx->output, rx->size, (liquid_float_complex *)rx->buff, &num);
    /*
		for(unsigned int k=0;k<num;++k){ 
			liquid_float_complex v;
			v=rx->buff[k];
			iirfilt_crcf_execute(rx->lowpass,v, (liquid_float_complex *)&rx->output[k]);
		}
	*/
        //printf("rx->size %d num %d\n",rx->size,num);
        
 	    double amin =  1e33;
	    double amax = -1e33;
	    
	    double gain=0.5;
	    
	    
		for (size_t i=0; i<num; i++ ) {
			double v;
			v=rx->buff[i].real();
			if(v < amin)amin=v;
			if(v > amax)amax=v;
		}

    if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;
    rx->aminGlobal = 0.9*rx->aminGlobal+0.1*amin;
    amin=rx->aminGlobal;
    
    if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;
    rx->amaxGlobal = 0.9*rx->amaxGlobal+0.1*amax;
    amax=rx->amaxGlobal;

	double dnom,dmin;

    if((amax-amin) > 0){
        dnom=65535.0/(amax-amin);
    }else{
        dnom=65535.0;
    }
    
    dmin=amin;

	    	    
	    
	    int kk=0;
	    
	    for(int n=0;n<num;++n){
	    
	        double value = abs(rx->buff[n]);
	        
			++kk;
			if(kk < nBufferFrames){
				double vv=rx->buff[n].real();
				
				vv=gain*((vv-dmin)*dnom-32768);

				short int v=(short int)(vv);

				buffer[2*kk] = v;
				buffer[2*kk+1] = v;
			}
	    
	    }
	    
	    rx->ibuff=0;
//	    printf("sound rx-size %d amin %g amain %g nBufferFrames %d kk %d vmin %g vmax %g\n",rx->size,amin,amax,nBufferFrames,kk,vmin,vmax);
    
    	rx->ibuff=0;

	}else{
  
	for ( i=0; i<nBufferFrames; i++ ) {
		for ( j=0; j<2; j++ ) {
		  *buffer++ = 0;
		}
	}
	  printf("sound nBufferFrames %d\n",nBufferFrames);

  }
  
    
  return 0;
}


