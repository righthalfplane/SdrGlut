# SdrGlut

Forced to move to gitlab - go there for lastest updates

SdrGlut V1.32 - Mostly Improvements and Clean up.

What is new ?

```
SdrGlut now has a device sync option where all open device windows track to the same frequency.
SdeGlut can now use more points in the FFT display
```

https://github.com/righthalfplane/SdrGlut/releases/tag/v1.32

If you are new to SdrGlut, please view the introduction video -
- [An Introduction to SdrGlut (A software defined radio)](https://youtu.be/JEXK-r6PvmA)

A users group is just starting - you can ask questions there -

https://groups.google.com/g/sdrglut-users

SdrGlut is a simple software defined radio player. Using glut for drawing and glui for its dialogs makes it tiny compared to programs that use QT5 or wxWidgets. SdrGlut uses liquid-sdr and SoapySDR to preform the SDR calculations. The audio is handled by OpenAL and RtAudio.


SdrGlut has a new Voice Command Module. It can be programed to record any frequency in any mode at any time. Listen is an example of a program that receives the data. Listen can decode AM, FM, NBFM, USB, LSB and output the audio to the speakers or another program. SdrGlut now has digital filter design mode. It can do highpass, lowpass, bandpass and bandstop filters. Best of all - it generates a c++ program that implements and tests the filter - so there is no question about how to make it work. It adds Voltage bias for powered antennas and Direct Sample Mode for RTL devices. It adds a window that can use the transmit capable SDRs as a signal generator for testing AM, NBFM, USB, and LSB devices. It has a prebuild exe for macOS, a zipped folder for Windows.

## Demo videos
- [An Introduction to SdrGlut (A software defined radio)](https://youtu.be/JEXK-r6PvmA)
- [Installing SdrGlut and iqSDR on Ubuntu24.10.](https://youtu.be/7ydgAz1LFfw)
- [Using the ANTSDR E200 on Windows with WSL Ubuntu 24.04.](https://youtu.be/rckekxm25bg)
- [Using The ANTSDR E200 on Ubuntu 24.04.](https://youtu.be/nyC2BJI9kSA)
- [Installing SdrGlut and iqSDR on Ubuntu 24.04.](https://youtu.be/cJnNznZeYSs)
- [Real Time TV on Windows using WSL Ubuntu.](https://youtu.be/focr8OPggzE)
- [Using The ANSDR E200 To Display Real Time TV.](https://youtu.be/Vg-0zkBW1gk)
- [Installing iqSDR on DragonOS.](https://youtu.be/yJ9pYlZZ1VI)
- [Using iqSDR To Enhance Gnuradio.](https://youtu.be/kvwMYOMpF4o)
- [How to Install iqSDR on Ubuntu 23.10.](https://youtu.be/_8FA2wanckQ)
- [Using the ANTSDR E200 on Windows with WSL Ubuntu 22.04.](https://youtu.be/sYnqp-skjEo)
- [HackRF One vs. CaribouLite at FRS Frequencies.](https://youtu.be/fzVpYNdPsi8)
- [CaribouLite SDR - Demo of CubicSDR 0.2.7 vs. SdrGlut-1182](https://youtu.be/sKcm6edLB-U)
- [Using the MicroPhase ANTSDR E200 on Ubuntu 23.10](https://youtu.be/76u6hPhku5E)
- [HackRF One vs. ANTSDR E200 at FRS Frequencies.](https://youtu.be/39tmEcldqsA)
- [BladeRF xA5 vs. ANTSDR E200 at FRS Frequencies.](https://youtu.be/LHv6Z11Tf9c)
- [Times LMR-240 vs. ABR Industries 25400f and 24240 cables.](https://youtu.be/LYvo1DxKcuI)
- [Ham It Up (Plus) using BladeRF-XA5 vs SDRplay RSP2 reception tests.](https://youtu.be/PRhD0N9icLw)
- [Ham It Up (Plus) vs SDRplay RSP2 reception tests.](https://youtu.be/ZATC0DXrhYg)
- [Receiving Two Or More Data Streams With gnuradio-companion.](https://youtu.be/7G7D5hoC7dI)
- [Ham It Up (Plus) vs RTL-SDR reception tests.](https://youtu.be/30DvmHJUL78)
- [Ham It Up (Plus) vs HackRF One reception tests.](https://youtu.be/jZtyk8VPHDA)
- [How to Install SdrGlut on Ubuntu 23.04.](https://youtu.be/lxZ4seB3ss0)
- [The Fastest wxWidgets OpenGL Graphics for uBuntu 23.04](https://youtu.be/aNU_7ST0lMw)
- [The Fastest wxWidgets OpenGL Graphics for MacOS Monterey](https://youtu.be/U51RqlkZRKQ)
- [The Fastest OpenGL Graphic for Ubuntu 22.04.](https://youtu.be/M0-jAKARte0)
- [A three way carpet cleaner shoot out with a big time winner.](https://youtu.be/HHVo2jPRg9M)
- [The World's Best Fly Swatter.](https://youtu.be/cStW4t8zLOk)
- [Sending Two Or More Data Streams With gnuradio-companion.](https://youtu.be/g0wN2VcWnA4)
- [Music from the ASCII Character Set and a Challenge.](https://youtu.be/RjL23ykfkYo)
- [The Secret Songs of the Great Pyramid.](https://youtu.be/0uk-aIEWGmQ)
- [A Small Black Hole passing low over Manhattan.](https://youtu.be/XjadKcJrCeE)
- [The Secret to Time Travel.](https://youtu.be/TrnJNo5-SJ8)
- [SdrGlut V1.31 - The New Remote Speaker Option.](https://youtu.be/UwEVdCotV5I)
- [Icom IC-R8600 - Creating an all Frequency Power Spectrum](https://youtu.be/AHhqNU7o7PM)
- [BladeRF xa5 - Creating an all Frequency Power Spectrum](https://youtu.be/Z2Fa7qzTfFQ)
- [BladeRF xA5 Simultaneously Receiving 14 Channels](https://youtu.be/GpmKlCI4L9E)
- [Receiving and Transmitting NBFM using gnuradio-companion.](https://youtu.be/L1x244p2eZo)
- [Transmitting and Receiving OFDM Data with the gnuradio-companion](https://youtu.be/A3aJDPcvrHg)
- [A P25 reception contest - dsd-louis vs. dsd-dme](https://youtu.be/DVqu4Q2ILPM)
- [sdrReceive vs. rx_tools(rx_power)](https://youtu.be/k5QfcGDs_sU)
- [sdrReceive vs. rx_tools(rx_fm)](https://youtu.be/aYcU28W96lU)
- [wxEqualizer a 10 channel audio equalizer using wxWidgets.](https://youtu.be/wcsD3ahMplg)
- [SdrGlut V1.30 - The  New Audio Window with Audio Equalization](https://youtu.be/xaySmML95l8)
- [CubicSDR v0.2.5 Vs. CubicSDR V0.2.7 on Ubuntu 22.04.](https://youtu.be/DZcVzly-s0M)
- [How to Build CubicSDR 0.2.7 on Ubuntu 22.04.](https://youtu.be/lsxtULKj_U4)
- [How to Install the AirSpyHF+ Routines on Ubuntu 22.04.](https://youtu.be/EzILVYP_20U)
- [How to Install SdrGlut on Ubuntu 22.04.](https://youtu.be/7K2V2rC0PEA)
- [How to Install RtAudio on Ubuntu 22.04](https://youtu.be/uARzCMbZ1pA)
- [CubicSDR v0.2.7 vs. SdrGlut v1.29 on Ubuntu 22.04.](https://youtu.be/3vVvpbkqy3Y)
- [SdrGlut V1.27 demonstrates the zoom frequency option with a SDRplay RSPduo.](https://youtu.be/83Iac8pRmZQ)
- [Build the Hoverman Over The Air TV antenna - Its the Best](https://youtu.be/G0M0Q0uGP4o)
- [The BladeRF-xA5 - how to receive both channels simultaneously.](https://youtu.be/Cn9l2YkIV9U)
- [Using the BladeRF xA5 to Display Real Time TV](https://youtu.be/r1wbiAUUUDk)
- [SdrGlut v1.29 vs. Gqrx 2.15.8 on MacOS Monterey with The BladeRF xA5](https://youtu.be/IvgDLU85edQ)
- [CubicSDR v0.2.7 vs. SdrGlut v1.28 with A bladeRF 2.0 micro xa5.](https://youtu.be/aF8lEFgrQVY)
- [Icom IC-R8600 vs. HackRF One at FRS Frequencies with SdrGlut.](https://youtu.be/mknOltFDJDI)
- [Icom IC-R8600 vs. BladeRF xA5 at FRS Frequencies with SdrGlut.](https://youtu.be/X8vDfFUc77I)
- [Icom IC-R8600 vs. SDRplay RSP2 at CB Frequencies with SdrGlut.](https://youtu.be/YOl3T4rkO_g)
- [Icom IC-R8600 vs. SDRplay RSP2 at FRS Frequencies with SdrGlut.](https://youtu.be/q5jHIm74ztM)
- [BladeRF xA5 vs. RTL-SDR.com at FRS Frequencies with SdrGlut.](https://youtu.be/h60fr9P4-U0)
- [BladeRF xA5 vs. SDRplay RSP2 at FRS Frequencies with SdrGlut.](https://youtu.be/XxvjWbmtfPs)
- [BladeRF xA5 vs. HackRF One at FRS Frequencies with SdrGlut.](https://youtu.be/oFb1m-Fa2_g)
- [Airspy HF+ vs. SDRplay RSPduo at CB Frequencies.](https://youtu.be/U_EnK4oW5aA)
- [Airspy HF+ vs. SDRplay RSPduo at MURS Frequencies.](https://youtu.be/oZlzmqPGcwM)
- [SDRplay RSPduo Channel 0 vs. Channel 1 using SdrGlut.](https://youtu.be/_cbs6En1Tqc)
- [Capturing SDR I/Q streams with SdrGlut.](https://youtu.be/N_384vv7IWY)
- [The Icom IC-R8600 being controlled by SdrGlut.](https://youtu.be/PEzeFCsx7H8)
- [Using Gmsh with the Mystic Finite Element Code](https://youtu.be/4uChJbdchbg)
- [SdrGlut (a Sofware Defined Radio) - Testing the Voice Control Module](https://youtu.be/pf-idEg8Hd8)
- [SdrGlut (a Software Defined Radio) revisits CW on 40 meters](https://youtu.be/LFf3LfJNvAA)
- [Software Defined Radios are Flaky](https://youtu.be/KaE-tjLuX40)
- [SdrGlut shows real time TV using the SDRplay RSPduo](https://youtu.be/VrzVkWhv9kY)
- [Installing Voice Recognition on the Raspberry Pi](https://youtu.be/ZcBeYBxeINY)
- [OpenGL speed tests of the MacBook pro M1](https://youtu.be/BXh3y1-3pug)
- [Using the Raspberry Pi as a IR Transmitter](https://youtu.be/1f0Z4V7Ii7w)
- [SdrGlut uses DSD to decode Public Service Channels with the RSPduo](https://youtu.be/x0Bo27QmZjA)
- [Using The Raspberry Pi as a IR Receiver.](https://youtu.be/UsrgqJ1gPJw)
- [SdrTest being remotely controlled by the control utility.](https://youtu.be/Ux1QHtEMzTY)
- [How to Install SdrGlut on MacOS Catalina (a software defined radio)](https://youtu.be/cOPHQ9_nHEA)
- [How to Install SdrGlut on Ubuntu 18.04 (a software defined radio)](https://youtu.be/LVpo78ubRiI)
- [Using gnuradio and the Impulse Source to Show Filter Frequency Response](https://youtu.be/vsm18e7duPU)
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
- [Icom IC-R30 vs a HackRF One at FRS frequencies with SdrGlut.](https://youtu.be/c5-g440IR7M)
- [Icom IC-R30 vs a Cobra 38WXST at CB frequencies](https://youtu.be/FBa0GskAYCI)
- [Icom IC-R30 vs a Eton Elite Executive at CB frequencies](https://youtu.be/jJe3iPdgmLY)
- [Icom IC-R8600 vs a ALINCO DX-R8 at CB frequencies](https://youtu.be/H_mMCzuW_aI)
- [Icom IC-R8600 vs a AOR AR8200 MK3 at CB frequencies.](https://youtu.be/xeJGhNRZTTM)
- [Icom IC-R8600 vs a AOR AR8200 MK3 at FRS frequencies.](https://youtu.be/PPTMccFdxdk)
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
- [Ultimaker S3 Printing With Two Colors - The whole story.](https://youtu.be/J3oaRju67QY)



## Installation of RtAudio

The lastest version of SdrGlut needs to have RtAudio installed. Here are videos showing how to do the install on various systems. The Raspberry Pi can follows the instructions that were given for Ubuntu 18.04.

Video of RtAudio installation on:
- [MacOS](https://youtu.be/HlnC5K5qQ98)
- [Windows 10](https://youtu.be/DyYIKiu2zVI)
- [Ubuntu18.04](https://youtu.be/mOGWG1K52wA)
- [Raspberry pi](https://youtu.be/YKyYXtv4aGw)
- [Ubuntu22.04](https://youtu.be/uARzCMbZ1pA)

## Problems With libliquid

There are recent versions of libliquid that have the same version number and need different function calls. In MainSource/firstFile.h the defines -

#define LIQUID_VERSION_4 1

#define WINDOWS_LONG_NAMES 1

can to be turned on or off as needed. The LIQUID_VERSION_NUMBER logic works correctly about 80 percent of the time.
  
## Installation steps on Ubuntu 22.04


```
In the file "MainSource/firstFile.h" comment out the line -

//#define WINDOWS_LONG_NAMES 1

Then -

bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential
sudo apt-get install libsoapysdr-dev
sudo apt-get install libopenal-dev
sudo apt-get install libliquid-dev
sudo apt-get install freeglut3-dev
sudo apt-get install libalut-dev
sudo apt-get install librtaudio-dev
sudo apt-get install git
cd ~/Desktop
git clone --depth=1 https//github.com/righthalfplane/SdrGlut.git
cd SdrGlut
make -f makefileUbunta -j 4
./sdrglut.x
```

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

# Install on MacOS 12.4 with brew

This is a copy of some of the instruction from -

https://github.com/pothosware/homebrew-pothos/wiki


brew tap pothosware/homebrew-pothos

brew update

brew install soapyremote

brew install soapyrtlsdr

brew install soapyhackrf

brew install soapybladerf

brew install soapyairspy

brew install soapyairspyhf

brew install soapyredpitaya

brew install soapyiris

brew install limesuite

brew install liquid-dsp

brew install rtaudio

brew install libsndfile

cd ~/Desktop

git clone --depth 1 https://github.com/righthalfplane/SdrGlut.git

cd SdrGlut

make -f makefileMACINTOSH

./sdrglut.x

To compile with the GUI envoke - SdrGlut.xcodeproj

# Install on MacOS 12.4 (M1) with brew

This is a copy of some of the instruction from -

https://github.com/pothosware/homebrew-pothos/wiki

brew tap pothosware/homebrew-pothos

brew update

brew install libusb

cp /opt/homebrew/include/libusb-1.0/libusb.h /opt/homebrew/include/

brew install soapyremote

brew install soapyrtlsdr

brew install soapyhackrf

brew install soapybladerf

brew install soapyairspy

brew install soapyairspyhf

brew install soapyredpitaya

brew install soapyiris

brew install limesuite

brew install liquid-dsp

brew install rtaudio

brew install libsndfile

cd ~/Desktop

git clone --depth 1 https://github.com/righthalfplane/SdrGlut.git

cd SdrGlut

make -f makefileM1Brew

./sdrglut.x

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

https://www.sdrplay.com/downloads/

SDRplay_RSP_API-MacOSX-3.07.3.pkg


# Install SdrGlut on Windows 7 and Windows 10


## Install RtAudio
See - above


## Install OpenAL -

OpenAL must be loaded on to window. Get the two packages -

OpenAL1.1.CoreSDK

OpenAL 1.1 windows install

from "https://www.openal.org/downloads"

and install them.

## Install PothosSDR and SdrGlut -

The easiest way to install SdrGlut is to install the PothosSDR package first -

https://downloads.myriadrf.org/builds/PothosSDR/

Run PothosSDR-2021.07.25-vc16-x64.exe and select

add PothosSDR to system path for all users

full install

git clone https://github.com/righthalfplane/SdrGlut.git

Then use the GUI and go to the directory SdrGlut/Sdrglut-windows and click the project SdrGlut.sln.

Select "Release" and "X64"  -

Then select "Start without Debugging", SdrGlut should compile and start running.
