#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/param.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <unistd.h>

#include <signal.h>

#include <assert.h>

/*
c++ -o udp udp.cpp
*/

int main()
{

	struct sockaddr_in si_me, si_other;
	int s;
	assert((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))!=-1);
	int port=5000;
	int broadcast=1;
	struct hostent *host;

	setsockopt(s, SOL_SOCKET, SO_BROADCAST,
				&broadcast, sizeof broadcast);
				
	 host= (struct hostent *) gethostbyname((char *)"192.168.0.7");

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = INADDR_ANY;
//	si_me.sin_addr.s_addr =  *((struct in_addr *)host->h_addr);


	assert(::bind(s, (sockaddr *)&si_me, sizeof(sockaddr))!=-1);
	
	
	long nn;
	
	nn=0;

	while(1)
	{
		char buf[10000];
		int ret;
		unsigned slen=sizeof(sockaddr);
		ret=recvfrom(s, buf, sizeof(buf)-1, 0, (sockaddr *)&si_other, &slen);

		printf("recv ret %d %ld\n",ret,nn++);
	}

}