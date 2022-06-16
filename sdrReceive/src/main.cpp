#include "cReceive.h"

#include <csignal>

volatile int threadexit; 

void signalHandler( int signum ) {
	//mprint("signum %d\n",signum);
	threadexit=1;
}

void checkall();


int main(int argc, char * argv [])
{	

	signal(SIGINT, signalHandler);										
	

	threadexit=0;

	class cReceive *rec2 = new cReceive(argc, argv);
	

	if(rec2->initPlay(rec2->rx)){
		fprintf(stderr,"initPlay Failed\n");
		return 1;
	};
		
	rec2->playRadio(rec2->rx);

	rec2->rx->doWhat=Exit;
	
	Sleep2(100);
		
	rec2->stopPlay(rec2->rx);
	
	delete rec2;
	
	checkall();
	
	return 0 ;
} /* main */


