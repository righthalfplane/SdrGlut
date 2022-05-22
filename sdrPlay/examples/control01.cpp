#include <stdio.h>

/*
g++ -O2 -std=c++11 -Wno-deprecated  -o control01 ./examples/control01.cpp
*/
int main(int argc,char *argv[])
{
	printf("a 192.168.0.2:3700\n");
	printf("#a 192.168.0.3:3700\n");
	printf("fc 162e6\n");
	printf("f 162.4e6\n");
	printf("m nbfm\n");
	printf("q\n");
}