#include <stdio.h>
#include <stdlib.h>
#include "SocketDefs.h"
#include <csignal>



//c++ -o pipeUDP3 pipeUDP3.cpp
int main()
{

    FILE *in;
    FILE *pipe;
    char data[256];
    int filenum;
    
    
    
    pipe=popen("/Applications/VLC.app/Contents/MacOS/VLC -v udp://@:1234 --program=4 --extraintf rc","w");
    fprintf(stderr,"pipe %p\n",pipe);
    int np=fileno(pipe);
    fprintf(stderr,"pipe %p np %d\n",pipe,np);
    
    
    int count;
    
    sleep(10);
    
   // write(np,"quit\n",5);
    fprintf(pipe,"quit\n");
           
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