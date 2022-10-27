#include <stdio.h>
#include <stdlib.h>
#include "SocketDefs.h"
#include <csignal>



//c++ -o pipeUDP pipeUDP.cpp
int main()
{

    FILE *in;
    FILE *out;
    FILE *pipe;
    char data[256];
    int filenum;
    
    
    
    pipe=popen("/Applications/VLC.app/Contents/MacOS/VLC -v udp://@:1234 --program=4","r");
    
    out=fopen("junk.txt","w");
    
    int count;
    
    while(1){
    	count=read(0,data,256);
    	if(count > 0 &&  out){
    		fprintf(out,"count 1 %d\n",count);
    		fclose(out);
    		sleep(1);
     		raise(SIGINT);
    		fprintf(out,"count 2 %d\n",count);
  		}else{
    		sleep(5);
    		fprintf(stderr,"SIGINT %d\n",SIGINT);
    		raise(SIGINT);

    	}
    }
    
    if(out)fclose(out);
    
    if(pipe)fclose(pipe);
    
    exit(1);
    
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