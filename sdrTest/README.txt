To build sdrTest on the MacOS -

g++ -O2 -std=c++11 -Wno-deprecated -o sdrTest sdrTest.cpp mThread.cpp cMalloc.c -lrtaudio -lSoapySDR -lliquid -lpthread -framework OpenAL

To build sdrTest on the Linux systems -

g++ -O2 -std=c++11 -Wno-deprecated -o sdrTest sdrTest.cpp mThread.cpp cMalloc.c -lrtaudio -lSoapySDR -lliquid -lpthread -lopenal


If you get an error on the line -

     f->demodAM = ampmodem_create(0.5, mode, iflag);

sdrTest.cpp:1270:50: error: too few arguments to function ‘ampmodem_s*


uncomment the line -

#define LIQUID_VERSION_4 1
