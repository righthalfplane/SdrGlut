#ifndef __LIMITS___
#define __LIMITS___

struct LIMITS{
    unsigned char *command;
    unsigned char *pioName;
    long CurrentFrame;
    long ImageCount;
    long pioIndex;
    double time;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double zmin;
    double zmax;
    double vmin;
    double vmax;
    int dolog;
    int iGot;
    int iGotTime;
    int iGotData;
    int type;
    int Reflect_x;
};

#define LIMIT_TYPE2D 0
#define LIMIT_TYPE3D 1

#endif

