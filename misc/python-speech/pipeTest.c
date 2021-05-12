#include <stdio.h>
#include <stdlib.h>
#include "SocketDefs.h"


//cc -o pipeTest pipeTest.c
int main()
{

    FILE *in;
    char data[256];
    int filenum;
    
    in=popen("python -u speech.py","r");
    if(!in){
        printf("popen failed\n");
        exit(1);
    }
    
    filenum=fileno(in);
    
    printf("filenum %d\n",filenum);
    
    
  //  int count=fread(data,1,256,in);

	while(1){
    	int count=read(filenum,data,256);
    
    	if(count <= 0){
        	printf("read failed");
    	}else{
        	data[count]=0;
        	printf("data = %s end\n",data);
    	}
    }
    
    if(in)pclose(in);

	return 0;
}