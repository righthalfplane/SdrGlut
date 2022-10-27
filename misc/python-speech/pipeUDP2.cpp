#include <stdio.h>
#include <stdlib.h>
#include "SocketDefs.h"

#include <cstdio>
#include <chrono>
#include <thread>

#include <csignal>



//c++  -std=c++11 -o pipeUDP2 pipeUDP2.cpp

struct info{
        volatile int istop;
}datai;


int Sleep2(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return 0;
}

int launchThread(void *data,int (*sageThread)(void *data))
{
    std::thread(sageThread,data).detach();
    return 0;
}


int ListenSocket(void *rxv)
{
	struct info *l=(struct info *)rxv;
    FILE *pipe;
    
    pipe=popen("/Applications/VLC.app/Contents/MacOS/VLC -v udp://@:1234 --program=4","r");
    
     while(!l->istop){
    	sleep(1);
    }
    
    fprintf(stderr,"SIGINT 1 %d\n",SIGINT);
    raise(SIGINT);
    fprintf(stderr,"SIGINT 2 %d\n",SIGINT);
  
    
    if(pipe)fclose(pipe);
   
   	fprintf(stderr,"ListenSocket exit\n");
   
	return 0;
}

int main()
{

    FILE *in;
    FILE *out;
    FILE *pipe;
    char data[256];
    int filenum;
    
    datai.istop = 0;
    launchThread((void *)&datai,ListenSocket);
    
    sleep(5);
    
    fprintf(stderr,"SIGINT 3 %d\n",SIGINT);
    //raise(SIGINT);
    fprintf(stderr,"SIGINT 4 %d\n",SIGINT);

    datai.istop = 1;
    
    sleep(5);
    
    exit(1);
    
    pipe=popen("/Applications/VLC.app/Contents/MacOS/VLC -v udp://@:1234 --program=4","r");
    
    out=fopen("junk.txt","w");
    
    int count;
    
    while(1){
    	count=read(0,data,256);
    	if(count > 0 &&  out){
    		fprintf(out,"count %d\n",count);
    	}else{
    		sleep(5);
    		throw std::runtime_error( "setBandwidth()" );
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