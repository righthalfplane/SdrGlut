#ifndef __TOOLS__
#define __TOOLS__

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
	

struct pushpopStruct{
    unsigned char *stackData;
    long stackDataSize;
    long stackCount;
    long stackMax;
};

typedef struct pushpopStruct *pushpopPtr;

pushpopPtr pushpopStartR(long n);

int pushpopPushR(void *v,long n,pushpopPtr pop);

int pushpopPopAllR(void *v,long n,pushpopPtr pop);

int pushpopPopR(void *v,long n,pushpopPtr pop);

int pushpopPeekR(void *v,long n,long nr,pushpopPtr pop);

int pushpopEND(pushpopPtr pop);

int pushpopDepth(pushpopPtr pop);

#define pushpopStart(p1) pushpopStartR(sizeof(*(p1)))

#define pushpopPush(p1,p2) pushpopPushR(p1,sizeof(*(p1)),p2)

#define pushpopPop(p1,p2) pushpopPopR(p1,sizeof(*(p1)),p2)

#define pushpopPeek(p1,p2,p3) pushpopPeekR(p1,sizeof(*(p1)),p2,p3)

#define pushpopPopAll(p1,p2) pushpopPopAllR(p1,sizeof(*(p1)),p2)


struct dataStackStruct{
    unsigned char *stackData;
    long stackDataSize;
    long stackCount;
    long stackMax;
};

typedef struct dataStackStruct *dataStackPtr;

#define dataStackStart(p1) dataStackStartR(sizeof(*(p1)))

dataStackPtr dataStackStartR(long n);

int dataStackEND(dataStackPtr pop);

void *dataStackNextR(dataStackPtr pop);


#define dataStackPopAll(p1) ((p1) ? (p1->stackCount=0,0) : 1)

#define dataStackDepth(p1) ((p1) ? (p1->stackCount) : 0)

#define dataStackNext(p2) (((p2)->stackCount+1 < (p2)->stackMax) ? ((p2)->stackCount++,(void *)&(p2)->stackData[((p2)->stackCount-1)*(p2)->stackDataSize]) : dataStackNextR(p2))

#define dataStackPop(p2) ((p2->stackCount <= 0) ? (NULL) : ((p2)->stackCount--,(void *)&(p2)->stackData[((p2)->stackCount)*(p2)->stackDataSize]))


#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */


#endif
