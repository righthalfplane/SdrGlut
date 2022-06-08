#include "cReceive.h"

volatile int threadexit; 

void signalHandler( int signum ) {
	//mprint("signum %d\n",signum);
	threadexit=1;
}

void checkall(void);

int main (int argc, char * argv [])
{	

	signal(SIGINT, signalHandler);  

	threadexit=0;

	class cReceive rec(argc,argv);
	
	if(rec.initPlay(rec.rx)){
		fprintf(stderr,"initPlay Failed\n");
		return 1;
	};
		
	rec.playRadio(rec.rx);

	rec.rx->doWhat=Exit;
	
	Sleep2(100);
		
	rec.stopPlay(rec.rx);
	
	checkall();
	
	return 0 ;
} /* main */


