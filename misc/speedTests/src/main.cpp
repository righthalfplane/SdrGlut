#include "cReceive.h"

#include <csignal>

volatile int threadexit; 

void signalHandler( int signum ) {
	//mprint("signum %d\n",signum);
	threadexit=1;
}

void checkall();

int test1(class cReceive *r);

int main(int argc, char * argv [])
{	

	signal(SIGINT, signalHandler);										
	

	threadexit=0;

	class cReceive *rec2 = new cReceive(argc, argv);
	

	if(rec2->initPlay(rec2->rx)){
		fprintf(stderr,"initPlay Failed\n");
		return 1;
	};
	
	
	//while(1){
	//	if(threadexit)break;
		test1(rec2);
	//	Sleep2(5);
	//}
	
	exit(1);
		
	rec2->playRadio(rec2->rx);

	rec2->rx->doWhat=Exit;
	
	Sleep2(100);
		
	rec2->stopPlay(rec2->rx);
	
	delete rec2;
	
	checkall();
	
	return 0 ;
} /* main */

int test1(class cReceive *r)
{

	class cDemod d;
	
	double rate=r->rx->device->getSampleRate(SOAPY_SDR_RX, r->rx->channel);
  
	int size=(int)rate/r->rx->ncut;

	r->rx->size=size;
	
	r->rx->cs->setBuff(r->rx);
	
 

	return d.process2(r->rx);
}


int cDemod::process2(struct playData *rxi)
{
	rx=rxi;	
	
	zerol((char *)&f,sizeof(f));
	
	setFilters(rx,&f);
	

	bAudio=(float *)cMalloc((size_t)(2*rx->faudio*4),9837);
    if(!bAudio){
    	rx->r->mprint("3 cMalloc Errror %ld\n",(long)(2*rx->faudio*4));
       	return 1;
    }
    

	bRf=(float *)cMalloc(2*rx->size*4,4567);
    if(!bRf){
    	rx->r->mprint("5 cMalloc Errror %ld\n",(long)(2*rx->size*4));
       	return 1;
    }
    zerol((char *)bRf,2*rx->size*4);
	
	Sleep2(1000);
	
	rx->cutOFFSearch=1;
	for(int n=0;n<rx->r->FFTlength;++n){
		rx->r->frequencies[n]=0;
		rx->r->ampitude[n] = -160;
	}
	
	
	//fprintf(stderr,"rx->faudio %f rx->size %d\n",rx->faudio,rx->size);
	
	printf("process2 2 fc %f f %f dt %g samplerate %d\n",rx->fc,rx->f,rx->dt,rx->samplerate);
	
	long steps=100;
	
	double start=rtime();
	
	for(long k=0;k<steps;++k){
	    if(threadexit)break;
		processAll2();
		//rx->r->updateLine();	
	}
	
	double end=rtime();
	
	fprintf(stderr,"steps = %ld Time = %02f Time/step %f size %d\n",steps,end-start,(end-start)/steps,rx->size);
	
	return 0;
}

void cDemod::processAll2()
{


	//printf("processAll2 fc %f f %f dt %g samplerate %d\n",rx->fc,rx->f,rx->dt,rx->samplerate);

/*

	int ip=-1;
	if(rx->cs){
		ip=rx->cs->popBuff(rx);
	}
	if(ip < 0){
	 	return;
	}
*/
	int ip=0;
	
	rx->r->iplay = 1;
		
	int witch=ip % NUM_DATA_BUFF;
	
	float *buf=rx->cs->buff[witch];
	 
	float *buf2=bRf;
	
	double sint,cost;

    //fprintf(stderr,"rx->r->fmult.size() %ld\n",(long)rx->r->fmult.size());
    
  // fprintf(stderr,"1 rx->size %d rx->dt %g buf %p\n",rx->size,rx->dt,buf);
   

   
   // fprintf(stderr,"2 rx->size %d rx->dt %g\n",rx->size,rx->dt);

	if(rx->r->iplay == 1){
		for (int k = 0 ; k < rx->size ; k++){
			float r = buf[k * 2];
			float i = buf[k * 2 + 1];
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
		if(r){
			rx->coso /= r;
			rx->sino /= r;
		}
    }else{
		for (int k = 0 ; k < rx->size ; k++){
			buf2[k * 2] = 0;
			buf2[k * 2 + 1] = 0;
		}
    }
    
  // fprintf(stderr,"3 rx->size %d rx->dt %g\n",rx->size,rx->dt);

	buf=bAudio;
		
    unsigned int num;
    unsigned int num2;
    
    num=0;
    num2=0;
    
    msresamp_crcf_execute(f.iqSampler, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf, &num);  // decimate
        
    if(rx->decodemode < MODE_AM){

		freqdem_demodulate_block(f.demod, (liquid_float_complex *)buf, (int)num, (float *)buf2);

    }else if(rx->decodemode < MODE_USB){
        #define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

        for(unsigned int n=0;n<num;++n){
            double mag=sqrt(buf[2*n]*buf[2*n]+buf[2*n+1]*buf[2*n+1]);
            double z0=mag + (f.amHistory * DC_ALPHA);
            buf2[n]=(float)(z0-f.amHistory);
            f.amHistory=z0;
        }
    }else{
        ampmodem_demodulate_block(f.demodAM,  (liquid_float_complex *)buf, (int)num, (float *)buf2);
   }
   
   msresamp_rrrf_execute(f.iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
   
/*
	if(rx->decodemode == MODE_NBFM){
		for(unsigned int n=0;n<num2;++n){
			double mag=buf[n];
			double z0=mag + (f.amHistory * DC_ALPHA);
			buf[n]=(float)(z0-f.amHistory);
			f.amHistory=z0;
		}
	}
*/

	float *buff=bAudio;

	int short *data;
	
	int audioOut;


	rx->mutexo.lock();
	audioOut=rx->audioOut;
	//mprint("audioOut %d\n",audioOut);
	data=rx->cs->buffa[rx->audioOut++ % NUM_ABUFF];
	rx->mutexo.unlock();

	double amin=1e30;
	double amax=-1e30;
	double average=0;


	double dnom,gain;

	gain=rx->gain;

	if(gain <= 0.0)gain=1.0;


	for (int i=0; i<rx->faudioCount; i++ ) {
		double v;
		v=buff[i];
		average += v;
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}
	

	average /= rx->faudioCount;
		
	//mprint("1 doAudio faudioCount %f amin %f amax %f audioOut %d ",rx->faudioCount,amin,amax,audioOut);
	//mprint(" aminGlobal %f amaxGlobal %f average %g\n",rx->aminGlobal ,rx->amaxGlobal,average);
	
	double tt=0.8;

/*
	if(rx->averageGlobal == 0.0)rx->averageGlobal=average;
	rx->averageGlobal = tt*rx->averageGlobal+(1.0-tt)*average;
	average=rx->averageGlobal;
*/

	amin -= average;

	amax -= average;

	if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;

	rx->aminGlobal = tt*rx->aminGlobal+(1.0-tt)*amin;

	amin=rx->aminGlobal;

	if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;

	rx->amaxGlobal = tt*rx->amaxGlobal+(1.0-tt)*amax;

	amax=rx->amaxGlobal;


	//mprint("3 doAudio faudioCount %f amin %f amax %f audioOut %d\n",rx->faudioCount,amin,amax,audioOut);

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

	if(rx->cs){
		// rx->cs->pushBuffa(audioOut,rx);
   	}
   
}


