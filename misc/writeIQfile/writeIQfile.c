#include <stdio.h>
#include <stdlib.h>
#include <liquid/liquid.h>

//cc -o writeIQfile writeIQfile.c  -lliquid

int main(int argc,char *argv[])
{
	FILE *in,*out;
	float buff1[20000],buff2[20000];
	int c;
	msresamp_crcf iqSampler;
	float As = 90.0f;

    
    iqSampler  = msresamp_crcf_create(0.5, As);
    // iqSampler  = msresamp_crcf_create(2.0, As);

	
	in=fopen("SDRplay-Single(0)_IQ_7000000_62500_fc.raw","rb");
	if(!in)exit(1);
	out=fopen("out","wb");
	if(!out)exit(1);
	
	while((c=fread(buff1,4,20000,in)) > 0){
	
	    unsigned int num;

	    msresamp_crcf_execute(iqSampler, (liquid_float_complex *)buff1, 10000, (liquid_float_complex *)buff2, &num);  // decimate

	     c=fwrite(buff2,8,num,out);
	}
	
	if (iqSampler)msresamp_crcf_destroy(iqSampler);

	
	printf("done\n");

}