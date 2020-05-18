#include "eMemory.h"
void *eMalloc(unsigned long r,int tag)
{
    void *ret=cMalloc(r,tag);
    
    if(!ret)throw std::runtime_error("Could not create UDP socket");;
    
    return ret;
}
int eFree(void *r)
{
    int ret=cFree((char *)r);
    if(ret)throw std::runtime_error("eFree free failed\n");
    return ret;
}

