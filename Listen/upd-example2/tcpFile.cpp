#include "SocketDefs.h"
#include "Clisten.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//c++ -O2 -std=c++11 -Wno-deprecated -o tcpFile tcpFile.cpp Clisten.cpp -Wall

int main()
{
		
    class Listen *l=new Listen;

    l->Port=3800;

	SOCKET ret = l->waitForService((char *)"192.168.0.2");
	if(ret < 0){
		delete l;
		return 1;
	}
	
	delete l;

    return 0;
}
