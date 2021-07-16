#include <stdio.h>
#include <stdlib.h>
//cc -o writeIQfile writeIQfile.c
int main(int argc,char *argv[])
{
	FILE *in,*out;
	int c;
	
	in=fopen("SDRplay Dev0 RSPduo- Single(0)_IQ_7051236_62500_fc.raw","rb");
	if(!in)exit(1);
	out=fopen("out","wb");
	if(!out)exit(1);
	
	while((c=fgetc(in)) != EOF){
	     fputc(c,out);
	     c=fgetc(in);
	     fputc(c,out);
	     c=fgetc(in);
	     fputc(c,out);
	     c=fgetc(in);
	     fputc(c,out);
	     c=fgetc(in);
	     fputc(c,out);
	     c=fgetc(in);
	     fputc(c,out);
	     c=fgetc(in);
	     fputc(c,out);
	     c=fgetc(in);
	     fputc(c,out);
	     
	     c=fgetc(in);
	     c=fgetc(in);
	     c=fgetc(in);
	     c=fgetc(in);
	     c=fgetc(in);
	     c=fgetc(in);
	     c=fgetc(in);
	     c=fgetc(in);

	}
	
	printf("done\n");

}