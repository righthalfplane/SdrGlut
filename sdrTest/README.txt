To build sdrTest on the MacOS -

g++ -O2 -std=c++11 -Wno-deprecated -o sdrTest sdrTest.cpp mThread.cpp cMalloc.c -lrtaudio -lSoapySDR -lliquid -framework OpenAL

To build sdrTest on the Linux systems -

g++ -O2 -std=c++11 -Wno-deprecated -o sdrTest sdrTest.cpp mThread.cpp cMalloc.c -lrtaudio -lSoapySDR -lliquid -lopenal -pthread


If you get an error on the line -

     f->demodAM = ampmodem_create(0.5, mode, iflag);

sdrTest.cpp:1270:50: error: too few arguments to function ‘ampmodem_s*


uncomment the line -

#define LIQUID_VERSION_4 1


Some examples -


./sdrTest -fc 1e6 -f 0.6e6 -gain 1

./sdrTest -fc 1e6 -f 0.76e6 -am -gain 1

./sdrTest -fc 1e6 -f 1.17e6 -gain 1

./sdrTest -fc 10.1e6 -f 10.0e6 -gain 1

./sdrTest -fc 27.1e6 -f 27.185e6 -gain 1

./sdrTest -fc 101.1e6 -f 101.5e6 -fm -gain 1

./sdrTest -fc 103.0e6 -f 103.7e6 -fm -gain 1

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1

./sdrTest -fc 162.0e6 -f 162.4e6 -nbfm -gain 1

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1 -faudio 12000 -file test.raw

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1 -timeout 60 -faudio 12000 -file test.raw

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1 -timeout 601 -dumpbyminute -faudio 12000

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1 -dumpbyminute -faudio 12000

/opt/local/bin/sox -t raw -r 48000 -b 16 -c 1 -L -e signed-integer saveAudio.raw -n stat
