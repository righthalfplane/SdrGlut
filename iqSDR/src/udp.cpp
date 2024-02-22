int ListenAudio(void *rxv)
{
  unsigned char *data;
  int audioOut;
  int bytes;
  
  
  socklen_t slen=sizeof(sockaddr);


    
    class Listen *rx=(class Listen *)rxv;
    
    
    
    
 	struct sockaddr_in si_me, si_other;
	int s;
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int port=5000;
	int broadcast=1;
	//struct hostent *host;

	setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast, sizeof broadcast);			
	 //host= (struct hostent *) gethostbyname((char *)"192.168.0.7");

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = INADDR_ANY;
//	si_me.sin_addr.s_addr =  *((struct in_addr *)host->h_addr);


	::bind(s, (sockaddr *)&si_me, sizeof(sockaddr));
	
    if(!rx->cs){
    	fprintf(stderr,"ListenAudio Error\n");
    	return 1;
	}
	
    rx->audioOut=0;
    audioOut=0;
    
	rx->mutexo.lock();
	data=(unsigned char *)rx->buffa[rx->audioOut++ % NUM_ABUFF];
	rx->mutexo.unlock();
	
	
	int nn=0;
    int nc=0;
	while(1)
	{
		char buf[10000];
		int ret;
		ret=recvfrom(s, buf, sizeof(buf)-1, 0, (sockaddr *)&si_other, &slen);

		printf("recv ret %d %d\n",ret,nn++);
		bytes=ret;
		for(int k=0;k<bytes;++k){
			data[nc++]=buf[k];
			if(nc >= (int)rx->samplerate*2){
				nc=0;
				rx->cs->pushBuffa(audioOut,rx);
				
				rx->mutexo.lock();
				audioOut=rx->audioOut;
				//printf("audioOut %d rx->samplerate %d\n",audioOut,rx->samplerate);
				data=(unsigned char *)rx->buffa[rx->audioOut++ % NUM_ABUFF];
				rx->mutexo.unlock();
				
			}
		}
		
		if(threadexit){
			//if(out)fclose(out);
			//fprintf(stderr,"ListenAudio Return\n");
			rx->ibuff=0;
			return 0;
		}
		
		
	}
 
 
    return 0;

}