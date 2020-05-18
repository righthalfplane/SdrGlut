#ifndef __EMEMORY__

#define __EMEMORY__

#include <iostream>

#include "Utilities2.h"

void *eMalloc(unsigned long r,int tag);

int eFree(void *r);

#endif

