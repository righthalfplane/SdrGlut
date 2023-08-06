 
// c++ -std=c++11 -o sendAM.x sendAM.cpp -lSoapySDR -lsndfile -lliquid -Wall -Wno-return-type-c-linkage
// sendAM.x "driver=hackrf"
// sendAM.x "driver=bladerf"

#include <iostream>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>
#include <cstdlib>
#include <cstddef>
#include <chrono>
#include <string>
#include <cstdint>
#include <complex>

#include <csignal>
#include <sndfile.h>

#include <liquid/liquid.h>

using namespace std;

#define	BLOCK_SIZE2 200000

#define	BLOCK_SIZE 2048

float buf [BLOCK_SIZE2],r[2*BLOCK_SIZE2];
float buf2 [BLOCK_SIZE2],r2[2*BLOCK_SIZE2];

int loop = 1;


class sTone{
public:
    sTone(double frequency,double sampleRate,double gain);
    int doTone(short int *buffer,int frames);
    int doTone(float *buffer,int frames);
    int gainSet(double gaini);
	~sTone();

	double w;
	double dt;
	double sino;
	double coso;
	double sindt;
	double cosdt;
	double gain;
};
sTone::~sTone()
{
	printf("~sTone\n");
}
int sTone::gainSet(double gaini)
{
	gain=gaini;
	return 0;
}
sTone::sTone(double frequency,double sampleRate,double gaini)
{
	double pi;
    pi=4.0*atan(1.0);
    dt=1.0/((double)sampleRate);
    sino=0;
    coso=1;
    w=2.0*pi*(frequency);
    sindt=sin(w*dt);
    cosdt=cos(w*dt);
    gain=gaini;
}

int sTone::doTone(float *buffer,int frames)
{

	if(frames <= 0)return 1;
	
	for(int k=0;k<frames;++k){
		double sint=sino*cosdt+coso*sindt;
		double cost=coso*cosdt-sino*sindt;
		coso=cost;
		sino=sint;
		buffer[k] += gain*cost;
//		buffer[2*k] += gain*cost;
//		buffer[2*k+1] += gain*sint;
		
	}
	

	double r=sqrt(coso*coso+sino*sino);
	coso /= r;
	sino /= r;

	return 0;
}


int sTone::doTone(short int *buffer,int frames)
{

	if(frames <= 0)return 1;

	for(int k=0;k<frames;++k){
		double sint=sino*cosdt+coso*sindt;
		double cost=coso*cosdt-sino*sindt;
		coso=cost;
		sino=sint;
		double v=32000*cost;
		if(v > 32000)v=32000;
		if(v < -32000)v=-32000;
		buffer[k]=(short)(v);
	}


	double r=sqrt(coso*coso+sino*sino);
	coso /= r;
	sino /= r;

	return 0;
}

class AMmod{
public:	
	AMmod(float mui){
		sample_rate_save=0;
		mu=mui;
		alpha=0.1f;
		amin=0;
		amax=0;
	};
	virtual ~AMmod(){
	    ;
	};
	int modulate(float *in,int sample_rate,float *out){
		float dmin = 1e33;
		float dmax =-1e33;
		for(int i=0;i<sample_rate;++i){
		    float v=in[i];
		    if(v < dmin)dmin=v;
		    if(v > dmax)dmax=v;
		}
		
    	if(amin == 0.0)amin=dmin;
    	amin = (1-alpha)*amin+alpha*dmin;
    
    	if(amax == 0.0)amax=dmax;
    	amax = (1-alpha)*amax+alpha*dmax;
    
 		//fprintf(stderr,"dmin %g dmax %g amin %g amax %g ",dmin,dmax,amin,amax);
 		
 		
 		if(sample_rate != sample_rate_save){
     		double pi;
    		pi=4.0*atan(1.0);
    		dt=1.0/(double)sample_rate;
    	    sino=0;
    	    coso=1;
    	    double w=2.0*pi*(sample_rate);
    	    sindt=sin(w*dt);
    	    cosdt=cos(w*dt);
    	    sample_rate_save=sample_rate;
 		}
   
        double sint,cost;
   
    	dmin=amin;
    	float average=dmax=amax;
		if(fabs(dmin) > average)average=fabs(dmin);
				
		//fprintf(stderr,"average %f\n",average);
				
		for(int i=0;i<sample_rate;++i){
			//float v=(2.0f*average+mu*in[i]);
			float v=(2.0f*mu*in[i]);
			
	        sint=sino*cosdt+coso*sindt;
            cost=coso*cosdt-sino*sindt;
            coso=cost;
            sino=sint;
			out[2*i]=v*coso;
			out[2*i+1]=v*sino;
		}		
		
    	double r=sqrt(coso*coso+sino*sino);
    	coso /= r;
    	sino /= r;		
		
		return 0;
	};
	float sample_rate_save;
	float alpha;
	float amin;
	float amax;
	float mu;
	
	float sino;
	float coso;
	float sindt;
	float cosdt;
	float dt;
};

void sigIntHandler(const int)
{
    loop = 0;
}

int error()
{
    exit(-1);
}

int main(int argc, char** argv)
{

	char 		infilename[]="/Users/dir/images/saveVoice.wav";
	SNDFILE	 	*infile = NULL ;
	SF_INFO	 	sfinfo ;
	long int count=0;
	int readcount;
	
    //const double frequency = 85.6e6;  //center frequency to 500 MHz
    const double frequency = 430e6;  //center frequency to 500 MHz
    const double sample_rate = 2e6;    //sample rate to 5 MHz
    const double freqSSB = 10000;
    const double freqAudio=48000;
    float As = 60.0f;
    
  	sTone *st1 = new sTone(-2000,freqAudio,0.125);
 	
 	st1->gainSet(0.125);

    sTone *st2 = new sTone(-1750,freqAudio,0.125);
 	
 	st2->gainSet(0.125/4.0);

 	sTone *st3 = new sTone(-1500,freqAudio,0.125);
 	
 	st3->gainSet(0.125);
 	
    sTone *st4 = new sTone(-1250,freqAudio,0.125);
 	
 	st4->gainSet(0.125);
 	
 	sTone *st5 = new sTone(-1000,freqAudio,0.125);
 	
 	st5->gainSet(0.125);
 	
    sTone *st6 = new sTone(-750,freqAudio,0.125);
 	
 	st6->gainSet(0.125);
 
    sTone *st7 = new sTone(-500,freqAudio,0.125);
 	
 	st7->gainSet(0.125);
 
    sTone *st8 = new sTone(-250,freqAudio,0.125);
 	
 	st8->gainSet(0.125);
    
    float *buf2=(float *)malloc(2*sample_rate);
	
    
    std::string argStr(argv[1]);
    
    std::vector<size_t> channels;

	channels = {0};

    auto device = SoapySDR::Device::make(argStr);
    if (device == NULL)
    {
        std::cerr << "No device!" << std::endl;
        return EXIT_FAILURE;
    }


   device->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);

    cout << "Sample rate: " << sample_rate/1e6 << " MHz" << endl;

    //Set center frequency
    
     device->setFrequency(SOAPY_SDR_TX, 0, frequency);
    
     

    auto txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
    
    
    size_t MTU=device->getStreamMTU(txStream);
    cout << "MTU: " << MTU << endl;
   
    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    
    device->setGain(SOAPY_SDR_TX, 0, 50.0);


	freqmod mod = freqmod_create(0.5); 
	

	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	
		printf ("Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		return 1 ;
	} ;
	
	printf ( "# Converted from file %s.\n", infilename) ;
	printf ( "# Channels %d, Sample rate %d\n", sfinfo.channels, sfinfo.samplerate) ;
		
	float Ratio1 = (float)(freqSSB/ (float)sfinfo.samplerate);
	
	float Ratio2 = (float)(sample_rate/(float)freqSSB);
	
	fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
	
	msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
	
	msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
	
	 ampmodem demodAM = ampmodem_create(0.5, LIQUID_AMPMODEM_DSB, 1);
	 //ampmodem demodAM = ampmodem_create(0.5, LIQUID_AMPMODEM_USB, 1);
	   //ampmodem demodAM = ampmodem_create(0.5, LIQUID_AMPMODEM_LSB, 1);
	 
	 AMmod modulation(0.5);
	 
	
	std::vector<void *> buffs(2);
	
    int flags(0);

    signal(SIGINT, sigIntHandler);
    
     unsigned char letters[256];
    
    for(int n=0;n<256;++n)letters[n]=n;
    
    int nc=0;
    int ncc=0;
   
    
    
    loop = 1;
	while(loop){
			
		if ((readcount = sf_readf_float (infile, buf, BLOCK_SIZE)) > 0){
		
			unsigned int num,num2;
			
			for(int n=0;n<readcount;++n){
			    buf[2*n]=0;
			    buf[2*n+1]=0;
			}
			
			if(1){
			//if(count == 0){
			    unsigned char l;
			
			    if(ncc++ >= 255){
			    	ncc=1;
			    }
			    
			    nc = ncc;
			    
			    //nc=1;
			    
			    l=letters[nc];
			    
			    printf("nc %d l %d\n",ncc,l);
			    
			    double ton=0.125;
			    double toff=ton/4.0;
			    
			    if(l & 1){
			        st1->gainSet(ton);
			    }else{
			        st1->gainSet(toff);			    
			    }
			    
			    if(l & 2){
			        st2->gainSet(ton);
			    }else{
			        st2->gainSet(toff);			    
			    }
				
			    if(l & 4){
			        st3->gainSet(ton);
			    }else{
			        st3->gainSet(toff);			    
			    }
				
			    if(l & 8){
			        st4->gainSet(ton);
			    }else{
			        st4->gainSet(toff);			    
			    }
				
			    if(l & 16){
			        st5->gainSet(ton);
			    }else{
			        st5->gainSet(toff);			    
			    }
				
			    if(l & 32){
			        st6->gainSet(ton);
			    }else{
			        st6->gainSet(toff);			    
			    }
				
			    if(l & 64){
			        st7->gainSet(ton);
			    }else{
			        st7->gainSet(toff);			    
			    }
				
			    if(l & 128){
			        st8->gainSet(ton);
			    }else{
			        st8->gainSet(toff);			    
			    }
				
			}else if(count > 0){
			    count=-1;
			}
						
			st1->doTone(buf,readcount);
			st2->doTone(buf,readcount);
			st3->doTone(buf,readcount);
			st4->doTone(buf,readcount);
			st5->doTone(buf,readcount);
			st6->doTone(buf,readcount);
			st7->doTone(buf,readcount);
			st8->doTone(buf,readcount);
			    
	    	msresamp_rrrf_execute(iqSampler, (float *)buf, readcount, (float *)buf2, &num);  // decimate
	    	
	    	//ampmodem_modulate_block(demodAM,buf2, num, (liquid_float_complex *)r2);
	    	
	    	modulation.modulate(buf2,num,r2);

		    
		   // freqmod_modulate_block(mod, buf2, num, (liquid_float_complex *)r2);
		    
		    msresamp_crcf_execute(iqSampler2, (liquid_float_complex *)r2, num, (liquid_float_complex *)buf2, &num2);  // decimate
			
			//fprintf(stderr,"num %d num2 %d\n",num,num2);
			
			int numcount=num2;
			
			float *buffp=buf2;
			
			while(numcount > 0){
				buffs[0] = buffp;
		
				int ret = device->writeStream(txStream,  &buffs[0], numcount, flags);
				if (ret <= 0){
					 printf("writeStream Error ret %d\n",ret);
				}
			
				numcount -= ret;
				buffp += 2*ret;

			}
			count++;
		}else{
		     sf_seek(infile,(sf_count_t)0,SEEK_SET);
		}
	}
	fprintf(stderr,"count %ld\n",count);

	sf_close (infile) ;
	
    freqmod_destroy(mod);
    
    if(demodAM)ampmodem_destroy(demodAM);

    
    device->deactivateStream(txStream);

    device->closeStream(txStream);
    
    SoapySDR::Device::unmake(device);
    
    return 0;
}
