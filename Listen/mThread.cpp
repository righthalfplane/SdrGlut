#define EXTERN22 extern
//#include "firstFile.h"
#include "mThread.h"
#include <chrono>
#include <thread>


int Sleep2(int ms)

{


    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    

	return 0;

}

int launchThread(void *data,int (*sageThread)(void *data))
{
    
    std::thread(sageThread,data).detach();
    
    return 0;
}


