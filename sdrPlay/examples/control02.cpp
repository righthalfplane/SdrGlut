#include <stdio.h>
#include <stdarg.h>
#include <chrono>
#include <thread>

/*
g++ -O2 -std=c++11 -Wno-deprecated  -o control02 ./examples/control02.cpp
*/

int Sleep2(int ms);

int main(int argc,char *argv[])
{
	printf("a 192.168.0.2:3700\n");
	printf("#a 192.168.0.3:3700\n");
	printf("fc 1e6\n");
	printf("f 1e6\n");
	printf("m am\n");
	for(int k=-10;k<=10;++k){
	   double f=1e6+k*10000;
	   printf("f %ld\n",(long)f);
	   fflush(stdout);
	   Sleep2(2000);
	}

	printf("q\n");
}

int Sleep2(int ms)

{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return 0;
}
