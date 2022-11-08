 
// c++ -std=c++11 -o sendFFT02 sendFFT02.cpp -lSoapySDR -lsndfile -lliquid -Wall -Wno-return-type-c-linkage
// sendFFT02 "driver=hackrf"
// sendFFT02 "driver=bladerf"
#include <stdio.h>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>
#include <signal.h>
#include <unistd.h>

#include <sndfile.h>
#include <string.h>

#include <liquid/liquid.h>

#define FILTER_RECTANGULAR     0
#define FILTER_HANN            1
#define FILTER_HAMMING         2
#define FILTER_FLATTOP         3
#define FILTER_BLACKMANHARRIS  4
#define FILTER_BLACKMANHARRIS7 5


int doWindow(float *x,long length,int type);

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
	fprintf(stderr,"~sTone\n");
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
		buffer[2*k] += gain*cost;
		buffer[2*k+1] += gain*sint;
		
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

class sourceBase{
public:
    virtual int doSource(short int *buffer,int frames);
};
int sourceBase::doSource(short int *buffer,int frames)
{
	//printf("doSource Base\n");
	return 1;
}
class sourceFile : public sourceBase{
public:
    virtual int doSource(short int *buffer,int frames);
    virtual ~sourceFile(){}
 	SNDFILE *infile;
};

int sourceFile::doSource(short int *buffer,int frames)
{
	//printf("doSource sourceFile\n");
	int readcount = (int)sf_readf_short(infile,(short *)buffer,frames);
	return readcount;
}

class sourceTone : public sourceBase{
public:
    virtual int doSource(short int *buffer,int frames);
    virtual int gainSet(double gain);
    virtual ~sourceTone(){}
    int doSource(float *buffer,int frames);
    sTone *tone;
};

int sourceTone::gainSet(double gain)
{
	//printf("doSource sourceTone\n");
	tone->gainSet(gain);
	return 0;
}
int sourceTone::doSource(float *buffer,int frames)
{
	//printf("doSource sourceTone\n");
	tone->doTone(buffer,frames);
	return frames;
}
int sourceTone::doSource(short int *buffer,int frames)
{
	//printf("doSource sourceTone\n");
	tone->doTone(buffer,frames);
	return frames;
}

class sourceImpulse : public sourceBase{
public:
    virtual int doSource(short int *buffer,int frames);
    virtual ~sourceImpulse();
    sourceImpulse(int nfft,double frequency);
    int doFFT(float *buffer,int frames);
    int forward_fft(float *buffer,long frames,int type);
    int doSource(float *buffer,int frames);
    int set(double freqSet,double value);
    int fft(float *data,int nn,int isign);
    int d_nfft;
    int d_nend;
    int nrun;
    double frequency;
    double *freq;
    float *out;
    int nfft;

};

sourceImpulse::sourceImpulse(int nffti,double frequencyi)
{
	fprintf(stderr,"sourceImpulse\n");
	nfft=nffti;
	frequency=frequencyi;
	freq=new double[nfft*2];
	for(int n=0;n<nfft;++n){
		freq[2*n]=0.0;
		freq[2*n+1]=0.0;
	}
	
	out=new float[nfft*2];

}

int sourceImpulse::set(double freqSet,double value)
{
	int np=(nfft-1)*freqSet/frequency;
	if(np < 0)np=0;
	if(np >= nfft)np=nfft-1;
	freq[2*np]=value;
	freq[2*np+1]=value;
	//fprintf(stderr,"np %d freqSet %f value %f nfft %d frequency %f\n",np,freqSet,value,nfft,frequency);
	return 0;
}

sourceImpulse::~sourceImpulse()
{
	fprintf(stderr,"~sourceImpulse\n");
	if(freq)delete freq;
	if(out)delete out;
}

int sourceImpulse::forward_fft(float *buffer,long length,int type)
{
	
	doWindow(buffer,length,type);
/*
   for(int n=0;n<length;++n){
        buffer[n*2] *= pow(-1.0,n);
        buffer[n*2+1] *= pow(-1.0,n);
	}
*/
	fft((float *)buffer,length,1);
	 	  			
	return length;
}

int sourceImpulse::doFFT(float *buffer,int frames)
{
         
     if(frames != nfft){
     	fprintf(stderr,"Error frames %d nfft %d\n",frames,nfft);
     }
 
 	for(int n=0;n<nfft*2;++n){
 	    out[n]=freq[n];
	}    
	
/*
   for(int n=0;n<nfft;++n){
        out[n*2] *= pow(-1.0,n);
        out[n*2+1] *= pow(-1.0,n);
	}
*/	
     
	 fft((float *)out,nfft,-1);
	 
	 
	double amin=1e33;
	double amax=-1e33;
		
	for(int n=0;n<nfft;++n){
	    double v=out[2*n];
	    if(v > amax)amax=v;
	    if(v < amin)amin=v;
		buffer[2*n] = out[2*n];
		buffer[2*n+1] = out[2*n+1];
	}

	//printf("amin %g amax %g\n",amin,amax);
	
	return frames;
}


int sourceImpulse::doSource(float *buffer,int frames)
{
	float out[2*32768];

	d_nfft=8192;
	d_nend=800;
	
	int nout=d_nfft;
		  
      	      
	int nw=0;
	int nend=d_nend;
	for(int n = 0; n < nout; n++)
	 {
		out[2*n]=0;
		out[2*n+1]=0;
		if(nw++ >= nend){
			nw=0;
			out[2*n]=1;
			out[2*n+1]=1;
		}
	 }
         
	 fft((float *)out,d_nfft,-1);
	 
	 
	double amin=1e33;
	double amax=-1e33;
		
	for(int n=0;n<frames;++n){
	    double v=out[2*n];
	    if(v > amax)amax=v;
	    if(v < amin)amin=v;
		buffer[n] = v;
	}

	//printf("amin %g amax %g\n",amin,amax);
	
	return frames;
}

int sourceImpulse::doSource(short int *buffer,int frames)
{
	float out[2*32768];

	d_nfft=32768;
	d_nend=0;
	
	int nout=d_nfft;
		  
      	      
	int nw=0;
	int nend=d_nend;
	for(int n = 0; n < nout; n++)
	 {
		out[2*n]=0;
		out[2*n+1]=0;
		if(nw++ >= nend){
			nw=0;
			out[2*n]=1;
			out[2*n+1]=1;
		}
	 }
         
	 fft((float *)out,d_nfft,-1);
	 
	 
	double amin=1e33;
	double amax=-1e33;
		
	for(int n=0;n<frames;++n){
	    double v=out[2*n];
	    if(v > amax)amax=v;
	    if(v < amin)amin=v;
		buffer[n]=(short int)v;
	}

	//printf("amin %g amax %g\n",amin,amax);
	
	return frames;
}

int sourceImpulse::fft(float *data,int nn,int isign)
{
	double twopi,tempr,tempi,wstpr,wstpi;
	double wr,wi,theta,sinth,fni;
	int i,j,n,m,mmax,istep;

	  data -= 1;
	  j=1;
	  n=2*nn;
	  twopi=8.*atan(1.);
	   for(i=1;i<=n;i += 2){
	   if(i-j >= 0)goto L200;
	   tempr=data[j];
	   tempi=data[j+1];
	   data[j]=data[i];
	   data[j+1]=data[i+1];
	   data[i]=tempr;
	   data[i+1]=tempi;
L200:    m=n/2;
L300:    if(j-m > 0)goto L400;
		goto L500;
L400:    j=j-m;
	   m=m/2;
	   if(m-2 >= 0)goto L300;
L500:  j=j+m;
	   }
	  mmax=2;
L600:   if(mmax-n >= 0)goto L1000;
	  istep=2*mmax;
	  theta=twopi/(double)(isign*mmax);
	  sinth=sin(theta/2.);
	  wstpr=-2.*sinth*sinth;
	  wstpi=sin(theta);
	  wr=1.;
	  wi=0.;
		for(m=1;m<=mmax;m+=2){
			for( i=m;i<=n;i+=istep){
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i]=data[i]+tempr;
				data[i+1]=data[i+1]+tempi;
			}
			tempr=wr;
			wr=wr*wstpr-wi*wstpi+wr;
			wi=wi*wstpr+tempr*wstpi+wi;
		}
	  mmax=istep;
	  goto L600;
L1000: 

	if(isign > 0){
		fni=2.0/(double)nn;
	}else{
		fni=0.5;
	}
	double amax=-1e33;
	double amin=1e33;
	for( i=1;i<=2*nn;++i){
		data[i]=data[i]*fni;
		double v=data[i];
		if(v > amax)amax=v;
		if(v < amin)amin=v;
	}
/*		
	static int p=0;
	
	if(++p > 20){
	   fprintf(stderr,"amin %g amax %g\n",amin,amax);
	   p=0;
	}
*/		
	
	return 0;
}

class sourceImpulse *si;

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

#define	BLOCK_SIZE2 200000

#define	BLOCK_SIZE 10000

float buf [BLOCK_SIZE2],r[2*BLOCK_SIZE2];
float r2[2*BLOCK_SIZE2];
float rsave[2*BLOCK_SIZE2],out2save[2*BLOCK_SIZE2];
float *buffp,*out2;

int Sleep2(int ms);

int loop = 1;
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
	
	
	//FILE *out11;
	//out11=fopen("test1_IQ_300000000_2000000_fc.raw","wb");
	//if(!out11)exit(1);

	

	
    const double frequency = 310e6;  //center frequency to 500 MHz
    const double sample_rate = 2e6;    //sample rate to 5 MHz
    //double freqSSB = 16384;
    const double freqSSB = 8192;
    //const double freqSSB = 4096;
    const double freqAudio=8192;
    float As = 60.0f;
    const double level=0.125;
    
 	si=new sourceImpulse(freqSSB,freqAudio);
	
	si->nrun=0;
/*
	si->set(250,level);
	si->set(500,level);
	si->set(750,level);
	si->set(1000,level);
	si->set(1250,level);
	si->set(1500,level);
	si->set(1750,level);
	si->set(2000,level);
*/
     
    float *buf2=(float *)malloc(2*sample_rate*10);
	
    
    std::string argStr(argv[1]);
    
    std::vector<size_t> channels;

	channels = {0};

    SoapySDR::Device *device = SoapySDR::Device::make(argStr);
    if (device == NULL)
    {
        fprintf(stderr,"No device!\n");
        return EXIT_FAILURE;
    }


   	device->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);
   
    fprintf(stderr,"Sample rate: %f MHz\n",sample_rate/1e6 );

    //Set center frequency
    
	device->setFrequency(SOAPY_SDR_TX, 0, frequency);
    
    
    device->setGain(SOAPY_SDR_TX, 0, 55.0);  // BladeRF
    // device->setGain(SOAPY_SDR_TX, 0, 60.0);    // Hackrf

     
    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
    
    
    size_t MTU=device->getStreamMTU(txStream);
    
      fprintf(stderr,"MTU: %lld\n",(long long)MTU);
 
    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    

	freqmod mod = freqmod_create(0.5); 
	

	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	
		fprintf(stderr,"Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		return 1 ;
	} ;
	
	fprintf(stderr, "# Converted from file %s.\n", infilename) ;
	fprintf(stderr, "# Channels %d, Sample rate %d\n", sfinfo.channels, sfinfo.samplerate) ;
		
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
			
			readcount=freqSSB;
			
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
			    
			    fprintf(stderr,"nc %d l %d\n",ncc,l);
			    
			    double ton=0.125;
			    double toff=ton/4.0;
			    
			    if(l & 1){
				    si->set(250,ton);
			    }else{
			    	si->set(250,toff);
			    }
			    
			    if(l & 2){
				    si->set(500,ton);
			    }else{
			    	si->set(500,toff);
			    }
			    				
			    if(l & 4){
				    si->set(750,ton);
			    }else{
			    	si->set(750,toff);
			    }
			    				
			    if(l & 8){
				    si->set(1000,ton);
			    }else{
			    	si->set(1000,toff);
			    }
			    				
			    if(l & 16){
				    si->set(1250,ton);
			    }else{
			    	si->set(1250,toff);
			    }
			    				
			    if(l & 32){
				    si->set(1500,ton);
			    }else{
			    	si->set(1500,toff);
			    }
			    				
			    if(l & 64){
				    si->set(1750,ton);
			    }else{
			    	si->set(1750,toff);
			    }
			    				
			    if(l & 128){
				    si->set(2000,ton);
			    }else{
			    	si->set(2000,toff);
			    }
			    								
			}else if(count > 0){
			    count=-1;
			}
						
		
			si->doFFT(buf,readcount);
			
			num=readcount;
					    
		    msresamp_crcf_execute(iqSampler2, (liquid_float_complex *)buf, num, (liquid_float_complex *)buf2, &num2);  // decimate
			
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
    

    
    device->deactivateStream(txStream);

    device->closeStream(txStream);
    
    SoapySDR::Device::unmake(device);
    
    if(demodAM)ampmodem_destroy(demodAM);
    
    if(iqSampler)msresamp_rrrf_destroy(iqSampler);
    
    if(iqSampler2)msresamp_crcf_destroy(iqSampler2);

    if(si)delete si;
    
    return 0;
}

int doWindow(float *x,long length,int type)
{
    double w[length];
    //double w[32768];
    int i;
    
    if(!x)return 1;
    
    switch(type){
            
        case FILTER_RECTANGULAR:

            for(i=0; i<length; i++)
                w[i] = 1.0;
            
            break;
            

            
        case FILTER_HANN:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hann(i, (int)length);
#else
                w[i]=hann(i, (int)length);
#endif
            }
            break;

            
            
        case FILTER_HAMMING:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hamming(i, (int)length);
#else
                w[i]=hamming(i, (int)length);
#endif
            }
            break;
            
        case FILTER_FLATTOP:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_flattop(i, (int)length);
#else
                w[i]=flattop(i, (int)length);
#endif
            }
            break;
            
            
        case FILTER_BLACKMANHARRIS:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris(i, (int)length);
#else
                w[i]=blackmanharris(i, (int)length);
#endif
            }
            break;
            
        case FILTER_BLACKMANHARRIS7:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris7(i, (int)length);
#else
                w[i]=blackmanharris7(i, (int)length);
#endif
            }
            break;
    }
    
    for(i=0; i<length; i++){
        double amp;
        amp=w[i];
        x[2*i]=amp*x[2*i];
        x[2*i+1]=amp*x[2*i+1];
    }
    
    return 0;
    
}

int Sleep2(int ms)

{
#ifdef _MSC_VER
        Sleep(ms);
#else
    usleep(ms*1000);
#endif
        return 0;
}

