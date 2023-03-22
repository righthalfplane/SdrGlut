#ifndef SDRRECEIVE_h
#define SDRRECEIVE_h

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include "CWindow.h"

#include "RaceFastsdr.h"


class SdrReceive;

class SdrReceive: public CWindow{
public:
    SdrReceive(struct Scene *scene,SoapySDR::Kwargs deviceArgs);
    virtual ~SdrReceive();
    virtual int OpenWindows(struct Scene *scene);
    int getMouse(int button, int state, int x, int y);
    int Reshape(int wscr, int hscr);
    int BackGroundEvents(struct Scene *scene);
    int moveMouse(int x, int y);
    int drawAxis();
    int display();
    int backGroundEvents;
    int width,height;
    uRect boxFrequency;
    uRect xAxis;
    uRect yAxis;
    
    double fcdown;
    
    double fdown;
    
    int fcount;

    struct playData *rx;
    struct playData rxs;

};

typedef SdrReceive *SdrReceivePtr;

#endif
