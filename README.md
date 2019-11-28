# SdrGult
SdrGlut is a simple software defined radio - using glut and glui for its interface.
SdrGlut uses liquid-sdr and SoapySDR to preform the SDR calculations. The audio is handled by OpenAL.

# Install on Ubuntu 18.04.3


This is what I did to get SdrGlut running on a fresh install of Ubuntu 18.04.3

sudo apt-get update

sudo apt-get upgrade

sudo apt-get install build-essential

sudo apt-get install libsoapysdr0.6

sudo apt-get install libsoapysdr-dev

sudo apt-get install libopenal-dev

sudo apt-get install libliquid-dev

sudo apt-get install freeglut3

sudo apt-get install freeglut3-dev

sudo apt-get install libalut0

sudo apt-get install libalut-dev

sudo apt-get install git

cd ~/Desktop

git clone https://github.com/righthalfplane/SdrGlut.git

cd SdrGlut

make -f makefileUbunta

./sdrglut.x




