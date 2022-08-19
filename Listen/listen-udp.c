/* 
 * listen-udp.c - Illustrate simple TCP connection
 * It opens a blocking socket and
 * listens for messages in a for loop.  It takes the name of the machine
 * that it will be listening on as argument.
 */

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
/*
cc -o listen-udp listen-udp.c
*/

//#define SERVER_PORT 50000
#define SERVER_PORT 5000
//#define SERVER_PORT 123
//#define SERVER_PORT 48321

volatile int threadexit; 

void signalHandler( int signum ) {
	//mprint("signum %d\n",signum);
	threadexit=1;
}


int main(int argc, char *argv[]) {
  unsigned char message[1024*2];
  int sock;
  struct sockaddr_in name;
  struct hostent *gethostbyname();
  int bytes;
  
  	signal(SIGINT, signalHandler);										
	

	threadexit=0;



  printf("Listen activating.\n");

  /* Create socket from which to read */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)   {
    perror("Opening datagram socket");
    exit(1);
  }
  
  /* Bind our local address so that the client can send to us */
  bzero((char *) &name, sizeof(name));
  name.sin_family = AF_INET;
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  name.sin_port = htons(SERVER_PORT);
  
  if (bind(sock, (struct sockaddr *) &name, sizeof(name))) {
    perror("binding datagram socket");
    printf("errno %d\n",errno);
    exit(1);
  }
  
  printf("Socket has port number #%d\n", ntohs(name.sin_port));
  
  static FILE *out=NULL;
	
  if(!out)out=fopen("junk22.raw","wb");

  
  while (threadexit == 0) {
    if((bytes = read(sock, message, 2048)) > 0){
    	message[bytes] = '\0';
    	printf("recv: %d %x %x %d\n", bytes, (unsigned char)message[0], (unsigned char)message[1], (unsigned char)message[2]+256*(unsigned char)message[3]);
    	if(out)fwrite(message,1,bytes,out);
    }
  }

  close(sock);
  
  return 0;
}
