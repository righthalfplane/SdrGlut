#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

//cc -o serverFile serverFile.c -Wall

int main()
{
		FILE *out;
        int sock;
        socklen_t addr_len;
        int bytes_read;
        char recv_data[4096*8+8];
        struct sockaddr_in server_addr , client_addr;
        
        out=fopen("Data.raw","wb");
        if(!out){
             printf("Could Not open file to write\n");
             exit(1);
        }


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

		long count=0;
		
        while (1)
        {

          bytes_read = recvfrom(sock,recv_data,sizeof(recv_data),0,
                            (struct sockaddr *)&client_addr, &addr_len);
                            
          if(bytes_read == 0 || bytes_read == -1){
               printf("bytes_read %d\n",bytes_read);
               continue;
          }


          recv_data[bytes_read] = '\0';

		  //printf("bytes_read %d count %ld\n",bytes_read,count);
		  
		  ++count;
		  
		  int nn=fwrite(recv_data,1,bytes_read,out);
		  if(nn != bytes_read){
		      printf("fwrite error\n");
		      exit(1);
		  }

          fflush(stdout);

        }
        return 0;
}
