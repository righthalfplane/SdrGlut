#include <stdio.h>
#include <stdlib.h>

//cc -o writeIQfile2 writeIQfile2.c

int main(int argc,char *argv[])
{
	FILE *in;
	float buff2[20000*2];
	int n;
	int c;

    

	
	in=fopen("SDRplay_IQ_7000000_20000_fc.raw","wb");
	if(!in)exit(1);
	
	n=0;
	while(n++ < 1000){
	   for(int k=0;k<20000*2;++k){
	       buff2[k]=n;
	   }
	   c=fwrite(buff2,8,20000,in);
	   if(c != 20000)printf("Error\n");
	}
	
	
	printf("done\n");

}