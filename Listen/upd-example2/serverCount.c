#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

//cc -o serverCount serverCount.c

int main()
{
        int sock;
        socklen_t addr_len;
        int bytes_read;
        char recv_data[1024],send_data[1024];
        struct sockaddr_in server_addr , client_addr;


        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("Socket");
            exit(1);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(5000);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        memset(&(server_addr.sin_zero),0,sizeof(server_addr.sin_zero));


        if (bind(sock,(struct sockaddr *)&server_addr,
            sizeof(struct sockaddr)) == -1)
        {
            perror("Bind");
            exit(1);
        }

        addr_len = sizeof(struct sockaddr);

        printf("\nUDPServer Waiting for client on port 5000");
        fflush(stdout);

        char a[20][10],d[20][10];
        strcpy(a[0],"A");
        strcpy(a[1],"B");
        strcpy(a[2],"C");
        strcpy(a[3],"D");
        strcpy(d[0],"123");
        strcpy(d[1],"124");
        strcpy(d[2],"100");
        strcpy(d[3],"99");

		long count=0;
		
        while (1)
        {

          bytes_read = recvfrom(sock,recv_data,1024,0,
                            (struct sockaddr *)&client_addr, &addr_len);


          recv_data[bytes_read] = '\0';

		  printf("bytes_read %d count %ld\n",bytes_read,++count);

          fflush(stdout);

        }
        return 0;
}
