#include "SocketDefs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//c++ -o clientSoapy clientSoapy.cpp -Wall

int main()
{
	SOCKET sock;
	int bytes_recv;
	socklen_t sin_size;
	struct sockaddr_in server_addr;
	struct hostent *host;
	char send_data[1024],recv_data[1024];

	host= (struct hostent *) gethostbyname((char *)"127.0.0.1");


	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8);
	sin_size = sizeof(struct sockaddr);

   while (1)
   {

    	printf("Type Something (q or Q to quit):");
    	gets(send_data);

    	if ((strcmp(send_data , "q") == 0) || strcmp(send_data , "Q") == 0)
       		break;

    	else
       		sendto(sock, send_data, strlen(send_data), 0,
              	(struct sockaddr *)&server_addr, sizeof(struct sockaddr));

         	bytes_recv = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,&sin_size);
          	recv_data[bytes_recv]= '\0';
    		printf("Received :%s\n",recv_data);
   	}
   	
   	closesocket(sock);

}

