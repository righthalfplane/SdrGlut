# SdrGult
SdrGlut is a simple software defined radio - using glut for drawing and glui for its dialogs.
SdrGlut uses liquid-sdr and SoapySDR to preform the SDR calculations. The audio is handled by OpenAL.

https://github.com/righthalfplane/SdrGlut/wiki/Installation-of-SdrGlut

# Installation of libliquid

There are several recent versions of libliquid. The call to The ampmodem_create differs. If a complier error happens,  switch the line that is commented out. It occurs in two places.

      f->demodAM = ampmodem_create(0.5, mode, iflag);
    
  //  f->demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
  
# Install on Ubuntu 18.04


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

# Install on Raspberry pi

To do the normal edition of Raspbian - follow the uBuntu instructions with -

make -f makefileRaspbian

instead of

make -f makefileUbunta

## The SDRPlay Edition


This is what I did to get SdrGlut running on the SDRPlay verison of Raspbian.

The SDRPlay verison Raspbian has the SDRPlay libraries already installed.


sudo apt-get update

sudo apt-get upgrade

sudo apt-get install build-essential

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

make -f makefileRaspbian

./sdrglut.x


## added support for NetSDR -

cd ~/Desktop

git clone https://github.com/pothosware/SoapyNetSDR.git

cd SoapyNetSDR

mkidr build

cd build

cmake ../

make -j 4

sudo make install

## added support for LimeSDR  -

cd ~/Desktop

git clone https://github.com/myriadrf/LimeSuite.git

cd LimeSuite

mkidr build

cd build

cmake ../

make -j 4

sudo make install

# Install on MacOS 10.4 with brew

This is a copy of some of the instruction from -

https://github.com/pothosware/homebrew-pothos/wiki


brew tap pothosware/homebrew-pothos
brew tap dholm/homebrew-sdr #other sdr apps
brew update

brew install soapyremote
brew install soapyaudio
brew install soapyrtlsdr
brew install soapyhackrf
brew install soapybladerf
brew install soapyairspy
brew install soapyairspyhf
brew install soapyosmo
brew install soapyredpitaya
brew install soapyiris
brew install limesuite


Do not do the "brew install soapyuhd" suggested by the wiki as it generates an exception error when it is used.


