# SdrGult

SdrGlut is a simple software defined radio player. Using glut for drawing and glui for its dialogs makes it tiny compared to programs that use QT5 or wxWidgets. SdrGlut uses liquid-sdr and SoapySDR to preform the SDR calculations. The audio is handled by OpenAL and RtAudio.

The latest release extends the audio recording capabilities of SdrGlut. It can be programed to record any frequency in any mode at any time. The 48,000 hz 16 bit raw audio files can be imported into Audacity and played or exported as another type of audio file. For information see video - https://youtu.be/oGKcOFbus_U . On MacOS Big Sur, you need to start xquartz before starting SdrGlut. 

SdrGlut now has a send option where it uses tcp/ip to transfer the I/Q stream to other programs. Listen is an example of a program that receives the data. Listen can decode AM, FM, NBFM, USB, LSB and output the audio to the speakers or another program. SdrGlut now has digital filter design mode. It can do highpass, lowpass, bandpass and bandstop filters. Best of all - it generates a c++ program that implements and tests the filter - so there is no question about how to make it work. It adds Voltage bias for powered antennas and Direct Sample Mode for RTL devices. It adds a window that can use the transmit capable SDRs as a signal generator for testing AM, NBFM, USB, and LSB devices. It has a prebuild exe for macOS, a zipped folder for Windows and a zipped folder for Ubuntu 18.04 to unpack and run from

https://github.com/righthalfplane/SdrGlut/releases/tag/v1.21

## Demo videos
- [How to Install SdrGlut (A software defined radio) on the Raspberry PI](https://youtu.be/xD9_6c3OisM)
- [SdrGlut - How to set the Time, Frequency and Mode for Recordings.](https://youtu.be/oGKcOFbus_U)
- [The Improved Search and Scan Features of SdrGlut](https://youtu.be/4oTv2DinCRk)
- [sdrTest is a test program for software defined radios.](https://youtu.be/hxxhsxCiPlY)
- [How to Install RtAudio on the Raspberry pi](https://youtu.be/YKyYXtv4aGw)
- [How to Install RtAudio on MacOS](https://youtu.be/HlnC5K5qQ98)
- [How to Install RtAudio on Windows 10](https://youtu.be/DyYIKiu2zVI)
- [How to Install RtAudio on Ubuntu18.04](https://youtu.be/mOGWG1K52wA)
- [How to Install RtAudio on Ubuntu20.04](https://youtu.be/tJ72-yEJgmU)
- [SdrGlut uses WSJT-X and GridTracker to capture digital Amateur traffic.](https://youtu.be/poebQhvDJ5k)
- [SoapyRemote generates distortion in the signal when sending.](https://youtu.be/_F1SvWRZwxg)
- [Sdrglut vs Gqrx on the raspberry Pi 4 (a speed comparison)](https://youtu.be/_UA0gFtRorI)
- [OpenGL speed tests of the Raspberry PI-4](https://youtu.be/pGWB5a5rb-s)
- [SDRplay RSPduo demonstrates the independent RX mode on the Raspberry Pi 4](https://youtu.be/17eJI8KIkYg)
- [SdrGlut vs CubicSDR on the Raspberry pi 4 with a Lime Mini SDR](https://www.youtube.com/watch?v=C18bGxbXNsA)
- [Raspberry PI 4 using SdrGlut to control the NetSDR](https://www.youtube.com/watch?v=jV-LxQS_acY)
- [SdrGlut uses 4 software defined radios to scan 5 different frequency ranges](https://youtu.be/XA8q5nJEWpc)
- [SdrGlut Simultaneously Running Five SDRs](https://www.youtube.com/watch?v=BmUNrAn1Llk)
- [SDRplay RSPduo running in Duel Tuner mode with SdrGlut](https://youtu.be/ShLUdb3Rdvs)
- [LimeSDR mini displays air traffic information (ADS-B) using dump1090](https://youtu.be/ZuXhtdZZ25Y)
- [SdrGlut tracks marine traffic using gnuais.](https://youtu.be/d1VfGnIB0xI)
- [SdrGlut decodes digital transmissions with FLdigi](https://youtu.be/8sQrpdutwgI)
- [SdrGlut sends I/Q data over the internet to the Listen program](https://youtu.be/xxTIfCEYSug)
- [Designing Digital Filters using SdrGlut - Part 1](https://youtu.be/MM7lI3RyWxg)
- [SdrGlut scans SDRs to look for and save stations for playback](https://youtu.be/HGv9suKBJU8)
- [SdrGlut Looks at signals around 450MHZ - What Are They ?](https://youtu.be/zt7G_6yNRcE)
- [SdrGlut captures GPS data for playback and analysis.](https://youtu.be/gAJw4ciOgQY)
- [Installing  gnss-sdr a software radio for processing satellite signal data](https://youtu.be/qfOcMfVFw3k)
- [SdrGlut uses SoapyRemote to play a remote Software Defined Radio](https://youtu.be/Z6yNH2zKDRo)
- [Viewing Over-The-Air Digital Broadcasts with a Digital Converter Box](https://youtu.be/ZJWdiFJ4HQ4)
- [Using SdrGlut to capture local TV stations with a SdrPlay RSP2](https://youtu.be/HwxXSgmRJbI)
- [Using SdrGlut to capture local TV stations with a LimeSDR Mini](https://youtu.be/x3yyEteT6qY)
- [Using a LimeSDR Mini as a signal source with SdrGlut](https://youtu.be/0jWvTtuV94Y)
- [Using a HackRF One as a signal source.](https://youtu.be/17qGxFdgvMY)
- [SdrGlut exploring Single Side Band on 80 meters](https://youtu.be/Up48SK5BGSw)
- [SdrGlut - The New Capabilities of Version 8](https://youtu.be/Vet6VFHjQ2E)
- [SdrGlut views CW signals on 80 meters](https://youtu.be/-WTC74p6WtY)
- [SdrGlut a Software Defined Radio player - Test drive #1](https://www.youtube.com/watch?v=zxgcRjTahcA)
- [SdrGlut a Software Defined Radio controller - Test drive #2](https://youtu.be/VRcvR2V-WFU)
- [SdrGlut a Software Defined Radio controller - Test drive #3](https://youtu.be/K3jH6BMdKkQ)
- [LimeSDR Mini using SdrGlut on Several Systems](https://youtu.be/yhxqu5VcO3w)
- [Gqrx vs SdrGlut at CB frequencies](https://youtu.be/P7zBRdvK6Jg)
- [Gqrx vs SdrGlut at FRS frequencies](https://youtu.be/g_v3TBrCw4o)
- [Icom IC-R30 vs a Cobra 38WXST at CB frequencies](https://youtu.be/FBa0GskAYCI)
- [Icom IC-R30 vs a Eton Elite Executive at CB frequencies](https://youtu.be/jJe3iPdgmLY)
- [Icom IC-R6800 vs a ALINCO DX-R8 at CB frequencies](https://youtu.be/H_mMCzuW_aI)
- [Icom IC-R6800 vs a AOR AR8200 MK3 at CB frequencies.](https://youtu.be/xeJGhNRZTTM)
- [Icom IC-R6800 vs a AOR AR8200 MK3 at FRS frequencies.](https://youtu.be/PPTMccFdxdk)
- [Icom IC-R30 vs a AOR AR8200 MK3 at FRS frequencies.](https://youtu.be/IpzZd8qS4g8)
- [SdrPlay RSP2 vs NetSDR+ using SdrGlut Part 1. ](https://youtu.be/LdIaFVuAICw)
- [SdrPlay RSP2 vs NetSDR+ using SdrGlut Part 2. ](https://youtu.be/1QJromJ76Lo)
- [SdrPlay RSP2 vs NetSDR+ using SdrGlut Part 3. ](https://youtu.be/NXLYYvLQN_k)
- [The SDRplay RSPduo vs. a RFSPACE NetSDR+ at FRS Frequencies](https://youtu.be/Bda-XdaxeW4)
- [The SDRplay RSPduo vs. a RTL-Stick at FRS Frequencies](https://youtu.be/hi_dumxZ9s0)
- [HackRf One vs. a Sdrplay RSP2 using SdrGlut at CB Frequencies](https://youtu.be/0dzZtHT1u2M)
- [The HackrRF One vs. the SDRplay RSPduo at FRS Frequencies.](https://youtu.be/YTmOm55_80Q)
- [SDRPlay RSP2 vs NetSDR+ at CB frequencies](https://youtu.be/BpmVxCTGsQQ)
- [Cobra 148 GTL ST AM vs USB at CB frequencies](https://youtu.be/40ih6kHTxSA)
- [SDRPlay RSP2 vs Grundig Satellit-750 at CB Frequencies](https://youtu.be/s3AlTwK_APg)
- [SRPlay RSP2 vs Realistic DX-440 at CB Frequencies](https://youtu.be/36ug6qZUKBI)
- [SdrPlay RSP2 vs Grundig G6 Aviator at CB frequencies](https://youtu.be/bUyQk_1ANHo)
- [Alinco DX-R8 vs a NetSDR+ using SdrGlut at CB frequencies](https://youtu.be/AxeiXFVd1x0)
- [Icom IC-R75 vs a NetSDR+ using SdrGlut at CB frequencies](https://youtu.be/bPzqTMoz9vc)
- [Icom IC-R20 vs a NetSDR+ using SdrGlut at CB frequencies](https://youtu.be/lAphTfKnU6M)
- [Icom IC-R30 vs a Icom IC-R20 at FRS frequencies](https://youtu.be/_ANaNVrqPz8)
- [Icom IC-R30 vs a NetSDR+ using SdrGlut at CB Frequencies](https://youtu.be/J4iJYugcG_Q)
- [Icom IC-R8600 vs a NetSDR+ using SdrGlut at CB Frequencies](https://youtu.be/rl7geOwf7Oc)
- [Icom IC-R30 vs a Icom IC-R8600 at FRS frequencies](https://youtu.be/nJeMPKXaWy4)
- [Icom IC-R8600 vs a SDRPlay RSP1 using SdrGlut at CB Frequencies](https://youtu.be/YfC5wsm0xyk)
- [Icom IC-R8600 vs a SDRPlay RSP1 using SdrGlut at FRS frequencies](https://youtu.be/fDBAtwQ26GQ)
- [Icom IC-R20 vs a SDRPlay RSP1 using SdrGlut at Family Radio Service frequencies](https://youtu.be/_SXy9OXZxAM)
- [Icom IC-R30 vs a SDRPlay RSP1 using SdrGlut at Family Radio Service frequencies](https://youtu.be/mEg04f57UGc)
- [NetSDR+ vs a HackRF One using SdrGlut at FRS frequencies](https://www.youtube.com/watch?v=5ubHUx9Naio)
- [LimeSDR Mini vs a HackRF One using SdrGlut at FRS frequencies](https://youtu.be/1EQcBYiyWAA)
- [Cobra 38 WXST testing an inverted T antenna using SdrGlut](https://www.youtube.com/watch?v=s-Rg2XC7bUY)
- [SDRPlay RSP2 vs a HackRF One using SdrGlut at FRS frequencies](https://www.youtube.com/watch?v=42uEKTncpbQ)
- [Corbra 38WXST vs Radio Shack TRC-236](https://www.youtube.com/watch?v=O9MQbf1ArQk)
- [Corbra 38WXST using three different antennas](https://www.youtube.com/watch?v=JDmxGQFwX-Y)
- [SdrPlay RSP2 vs a simple RTL stick using SdrGlut at FRS frequencies](https://www.youtube.com/watch?v=eiuTO5yIR7w)
- [SdrPlay RSP2 vs a LimeSDR Mini using SdrGlut at FRS frequencies](https://www.youtube.com/watch?v=PEfrJ4ZuN9g)
- [Lime Mini vs a RTL stick using SdrGlut at FRS frequencies](https://www.youtube.com/watch?v=pDqNCe6GxnA)
- [The Ghosts that I have seen](https://youtu.be/yusmsRdVW1Q)
- [Mail scatter on the street and sidewalk](https://youtu.be/V2LRiLIExjI)
- [A UFO flys over San Diego](https://youtu.be/O054bYnZmFw)
- [The Vanishing Jet and Templet Based Vision](https://youtu.be/caeDZnL3oPg)
- [DISCON the Ultimate computer system of 1982](https://youtu.be/a3yK93VcKm4)
- [Using gnuradio-companion to capture over the air ATSC video with the LimeSDR Mini](https://youtu.be/guPmP1VPqKk)
- [Using gnuradio-companion to capture over the air ATSC video](https://youtu.be/jQGk9dORKrc)
- [LimeSDR Mini using CubicSDR 0.2.5 on Several Systems](https://youtu.be/zCTIK82YW9w)
- [A Power Meter for CubicSDR 0.2.5](https://youtu.be/07vbIeXHukI)



## Installation of RtAudio

The lastest version of SdrGlut needs to have RtAudio installed. Here are videos showing how to do the install on various systems. The Raspberry Pi can follows the instructions that were given for Ubuntu 18.04.

Video of RtAudio installation on:
- [MacOS](https://youtu.be/HlnC5K5qQ98)
- [Windows 10](https://youtu.be/DyYIKiu2zVI)
- [Ubuntu18.04](https://youtu.be/mOGWG1K52wA)
- [Raspberry pi](https://youtu.be/YKyYXtv4aGw)

## Problems With libliquid

There are recent versions of libliquid that have the same version number and need different function calls. In MainSource/firstFile.h the defines -

#define LIQUID_VERSION_4 1

#define WINDOWS_LONG_NAMES 1

can to be turned on or off as needed. The LIQUID_VERSION_NUMBER logic works correctly about 80 percent of the time.
  
## Installation steps on Ubuntu 18.04

This is what I did to get SdrGlut running on a fresh install of Ubuntu 18.04.3

```
bash
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
```

## Add SDRPlay to Ubuntu 18.04

1. Go to https://www.sdrplay.com/downloads/
2. Select to download: API/HW DRIVER – V2.13 (20TH JUN 2018)
3. This will download SDRplay_RSP_API-Linux-2.13.1.run
4. In the bash shell, install of the drivers by running ```./SDRplay_RSP_API-Linux-2.13.1.run```
5. Append to the blacklist file (/etc/modprobe.d/blacklist.conf) the following 3 lines
      ```txt
        blacklist sdr_msi3101
        blacklist msi001
        blacklist msi2500
      ```
6. Reboot and you should be ready to go

## Install on Raspberry pi

SdrGlut runs nicely on the Raspberry pi 4 -  Do not know if slower version have enough horsepower for it

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

brew install cubicsdr

cd ~/Desktop

git clone https://github.com/righthalfplane/SdrGlut.git

cd SdrGlut

make -f makefileMACINTOSH

./sdrglut.x

Do not do the "brew install soapyuhd" suggested by the wiki as it generates an exception error when it is used.

The install of cubicsdr fails, but it installs the needed liquid-sdr.

To compile with the GUI envoke - SdrGlut.xcodeproj

## added support for NetSDR -

cd ~/Desktop

git clone https://github.com/pothosware/SoapyNetSDR.git

cd SoapyNetSDR

mkidr build

cd build

cmake ../

make -j 4

sudo make install

## added support for SDRPlay -

Get the sdrplay driver installer and install it -

https://www.sdrplay.com/macdl.php

SDRplay_RSP_API-MacOSX-2.13.2.pkg




# Install SdrGlut on Windows 7

## Install OpenAL -

OpenAL must be loaded on to window. Get the two packages -

OpenAL1.1.CoreSDK

OpenAL 1.1 windows install

from "https://www.openal.org/downloads"

and install them.

## Install PothosSDR and SdrGlut -

The easiest way to install SdrGlut is to install the PothosSDR package first -

https://downloads.myriadrf.org/builds/PothosSDR/

Run PothosSDR-2019.06.09-vc14-x64.exe and select

add PothosSDR to system path for all users

full install

There is a problem with the "Device.h" the following section needs to be commented out -

///@cond INTERNAL
//! Old setupStream invoked by compatibility macro

//static inline int SoapySDRDevice_setupStream_(SoapySDRDevice *device,

//    SoapySDRStream **stream,

//    const int direction,

//    const char *format,

//    const size_t *channels,

//    const size_t numChans,

//    const SoapySDRKwargs *args)

//{////

//    *stream = SoapySDRDevice_setupStream(device, direction, format, channels, numChans, args);

//    return (*stream == NULL)?-1:0;

//}


//#define SoapySDR_getMacro(_1,_2,_3,_4,_5,_6,_7,NAME,...) NAME

//#define SoapySDRDevice_setupStream(...) SoapySDR_getMacro(__VA_ARGS__, SoapySDRDevice_setupStream_, SoapySDRDevice_setupStream)(__VA_ARGS__)

///@endcond


git clone https://github.com/righthalfplane/SdrGlut.git

Then use the GUI and go to the directory SdrGlut/Sdrglut-windows and click the project SdrGlut.sln.

Select "Release" and "X64" and then find and switch the comments on the lines -

           // SoapySDRDevice_setupStream(rx->device,&rx->rxStream,SOAPY_SDR_RX, SOAPY_SDR_CF32, NULL,0,NULL);
            
           rx->rxStream=SoapySDRDevice_setupStream(rx->device,SOAPY_SDR_RX, SOAPY_SDR_CF32, NULL,0,NULL);


Then select "Start without Debugging", SdrGlut should compile and start running.
