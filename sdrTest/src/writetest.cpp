#include <stdio.h>
#include <stdlib.h>
/*
g++ -o writetest writetest.cpp
*/
int main (int argc, char * argv [])
{	
	int k,n;
	
	n=0;
	for(int k = -10;k < 10;++k){
		printf("./sdrTest -fc 602.%d0e6 -f 602.308440e6 -usb -faudio 12000 -timeout 2 -file test01.raw -device 0 >& test.txt ;",30844+1000); 
		
		printf("/opt/local/bin/sox -t raw -r 12000 -b 16 -c 1 -L -e signed-integer test01.raw -n stat >& test%02d.txt\n",n);
		++n;
	}
	return 0;
}