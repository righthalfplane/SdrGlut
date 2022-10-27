#include <stdio.h>
#include <stdlib.h>
#include <liquid/liquid.h>

//cc -o writeIQfile3 writeIQfile3.c  -lliquid

int main(int argc,char *argv[])
{
	FILE *in,*out;
	float buff1[20000*8],buff2[20000*8];
	int c;
	msresamp_crcf iqSampler;
	float As = 90.0f;
	float dt;

    
    iqSampler  = msresamp_crcf_create(0.5, As);
    // iqSampler  = msresamp_crcf_create(2.0, As);

	
	in=fopen("SDRplay Dev0_IQ_580598_62500_fc.raw","rb");
	if(!in)exit(1);
	out=fopen("out.in","w");
	if(!out)exit(1);
	
	dt=1.0/62500;
	while((c=fread(buff1,8,62500,in)) > 0){
	
	    unsigned int num;

	    //msresamp_crcf_execute(iqSampler, (liquid_float_complex *)buff1, 10000, (liquid_float_complex *)buff2, &num);  // decimate
		fprintf(out,"plot 62500 real\n");
		for(int n=0;n<62500;++n){
		   fprintf(out,"%f %f\n",dt*n,buff1[2*n]);
		}
		
		fprintf(out,"plot 62500 imag\n");
		for(int n=0;n<62500;++n){
		   fprintf(out,"%f %f\n",dt*n,buff1[2*n+1]);
		}
		fprintf(out,"exit\n");
		
		break;
	    
	}
	
	if (iqSampler)msresamp_crcf_destroy(iqSampler);

	
	printf("done c %d\n",c);
	
	exit(0);

}