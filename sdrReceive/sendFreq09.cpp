 
// c++ -std=c++11 -o sendFreq09 sendFreq09.cpp -lSoapySDR -lsndfile -lliquid -Wall -Wno-return-type-c-linkage
// sendFreq09 "driver=hackrf"
// sendFreq09 "driver=bladerf"
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
    virtual ~sourceImpulse(){}
    int doSource(float *buffer,int frames);
    int fft(float *data,int nn,int isign);
    int d_nfft;
    int d_nend;
    int nrun;

};
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
			float v=(2.0f*average+mu*in[i]);
			
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
float buf2 [BLOCK_SIZE2],r2[2*BLOCK_SIZE2];
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

	
	si=new sourceImpulse;
	
	si->nrun=0;
	

	
    //const double frequency = 462.7125e6;
    const double frequency = 310.0e6;
    //const double frequency = 85.5e6;
    const double sample_rate = 2.0e6;
    float As = 60.0f;
    
    //float *buf2=(float *)malloc(2*sample_rate);
    
    
    class sourceTone *st1=new sourceTone;
 	
 	st1->tone= new sTone(-2000,sample_rate,0.125);

	class sourceTone *st2=new sourceTone;
 	
 	st2->tone= new sTone(-1500,sample_rate,0.125);
 	
	class sourceTone *st3=new sourceTone;
 	
 	st3->tone= new sTone(-1000,sample_rate,0.125);
 	
	class sourceTone *st4=new sourceTone;
 	
 	st4->tone= new sTone(-500,sample_rate,0.125);
 	
	class sourceTone *st5=new sourceTone;
 	
 	st5->tone= new sTone(500,sample_rate,0.125);
 	
	class sourceTone *st6=new sourceTone;
 	
 	st6->tone= new sTone(1000,sample_rate,0.125);
 	
	class sourceTone *st7=new sourceTone;
 	
 	st7->tone= new sTone(1500,sample_rate,0.125);
 	
	class sourceTone *st8=new sourceTone;
 	
 	st8->tone= new sTone(2000,sample_rate,0.125);
 	
 	    
    std::string argStr(argv[1]);
    
    std::vector<size_t> channels;

	channels = {0};

    SoapySDR::Device *device = SoapySDR::Device::make(argStr);
    if (device == NULL)
    {
        printf("No device! Found\n");
        return EXIT_FAILURE;
    }


   device->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);

    fprintf(stderr,"Sample rate: %g MHZ\n",sample_rate/1e6);

     device->setFrequency(SOAPY_SDR_TX, 0, frequency);
    
       device->setGain(SOAPY_SDR_TX, 0, 55.0);  // BladeRF
    // device->setGain(SOAPY_SDR_TX, 0, 60.0);    // Hackrf

    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    
    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    
    size_t MTU=device->getStreamMTU(txStream);
    fprintf(stderr,"MTU: %ld\n",(long)MTU);


	freqmod mod = freqmod_create(0.5); 
	

	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	
		printf ("Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		return 1 ;
	} ;
	
	fprintf(stderr, "Read from file %s.\n", infilename) ;
	fprintf(stderr,"Number of Channels %d, Sample rate %d\n", sfinfo.channels, sfinfo.samplerate) ;
		
	float Ratio1 = (float)(12500/ (float)sfinfo.samplerate);
	
	float Ratio2 = (float)(sample_rate/(float)12500);
	
	fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
	
	msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
	
	msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
	
	ampmodem demodAM = ampmodem_create(0.5, LIQUID_AMPMODEM_DSB, 0);
	
	AMmod modulation(0.5);
	
	std::vector<void *> buffs(2);
	
    int flags(0);


    signal(SIGINT, sigIntHandler);
    
    //unsigned char *letters=(unsigned char *)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    unsigned char letters[256];
    
    for(int n=0;n<256;++n)letters[n]=n;
    
    int nc=0;
    int ncc=0;
    
    int tick=1;
    loop = 1;
	while(loop){
		if ((readcount = sf_readf_float (infile, buf, BLOCK_SIZE)) > 0){
		
				
			float *buffp;
			
			readcount=10000;
			
			for(int n=0;n<readcount*2;++n){
				buf[n]=0;
			}

			if(count == 0){
			    unsigned char l;
			    
			    //if(nc++ >= strlen((char *)letters)){
			    if(ncc++ >= 255){
			    	ncc=1;
			    }
			    
			    nc = ncc;
			    
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
				
			}else if(count > 9){
			    count=-1;
			}
			
				st2->doSource(buf,readcount);
				st1->doSource(buf,readcount);
				st3->doSource(buf,readcount);
				st4->doSource(buf,readcount);
				st5->doSource(buf,readcount);
				st6->doSource(buf,readcount);
				st7->doSource(buf,readcount);
				st8->doSource(buf,readcount);
			
			
			
			//fwrite(buf,8,readcount,out11);

			if(tick == 0){
			    double dt=1.0/sample_rate;
			    printf("plot %d signal\n",readcount);
			    for(int n=0;n<readcount;++n){
			        printf("%f %f\n",n*dt,buf[2*n]);
				}
				tick=1;
			}

			int numcount=readcount;
			
			buffp=buf;
			
			while(numcount > 0){
				buffs[0] = buffp;
				int numsend;
				
				numsend=numcount;
				
				//if(numsend > MTU)numsend=MTU;
		
				int ret = device->writeStream(txStream,  &buffs[0], numsend, flags);
				if (ret <= 0){
				     printf("writeStream Error ret %d\n",ret);
					 continue;
				}
							
				numcount -= ret;
				buffp += 2*ret;

			}
			//printf("count %ld\n",(long)count);
			count++;
		}else{
		     sf_seek(infile,(sf_count_t)0,SEEK_SET);
		}
	}
	
	fprintf(stderr,"count %ld\n",count);

	sf_close (infile) ;
	
    freqmod_destroy(mod);
    
    if(demodAM)ampmodem_destroy(demodAM);

    if(iqSampler)msresamp_rrrf_destroy(iqSampler);
    
    if(iqSampler2)msresamp_crcf_destroy(iqSampler2);

    device->deactivateStream(txStream);

    device->closeStream(txStream);
    
    SoapySDR::Device::unmake(device);
    
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

