#include "sound.h"
#include "WarningBatch.h"


static int sound2( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );

SNDFILE *sfopen(char *filename,int faudio);

soundClass::soundClass()
{
	
	audiodevice=0;
	faudio=48000;
	ncut=20;
	bS=NULL;
	soundRun=0;
	audioSync=0;
	inputID=-1;
	outputID=-1;
	gain=1.0;
	outfile=NULL;
}
int soundClass::startRecord(char *fname)
{

	if(fname == NULL){
	//	winout("stop Record %s\n",fname);
		if(outfile)sf_close(outfile);
		outfile=NULL;
	}else{
	//	winout("startRecord %s\n",fname);
		outfile=sfopen(fname,faudio);
		if(!outfile)winout("Error Opening File %s for WAV data\n",fname);
	}
	return 0;
}
SNDFILE *sfopen(char *filename,int faudio)
{
    char strbuffer[256];
    static int pversion=0;
    
    sf_command (NULL, SFC_GET_LIB_VERSION, strbuffer, sizeof (strbuffer)) ;
    
    if(pversion == 1)winout("sndfile version %s\n",strbuffer);
    pversion=0;
    
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof (sfinfo));
    sfinfo.samplerate = faudio;
    sfinfo.channels   = 1; // mono
    sfinfo.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    
    if(!filename)return NULL;
    
    SNDFILE * fid_wav = sf_open(filename, SFM_WRITE, &sfinfo);
    
    return fid_wav;
}

soundClass::~soundClass()
{
	winout("exit soundClass %p\n",this);
}
int sound2( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  soundClass *rx=(soundClass *)userData;
  
  rx->sound(outputBuffer,inputBuffer,nBufferFrames,
            streamTime, status, userData );

	return 0;
}
int soundClass::sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
	short int *buffer = (short int *) outputBuffer;

	//sdrClass *rx=(sdrClass *)userData;
	static long long int record=0;

	if ( status )winout("Stream underflow detected!\n");

	int ibuff = -1;
	
	if(bS)ibuff=bS->popBuffa();
	
	//winout("ibuff %d bS %p\n",ibuff,bS);

	if (ibuff >= 0){
		short int *buff= bS->buffa[ibuff % NUM_ABUFF2];

		double amin=1e33;
		double amax=-133;
		for (unsigned int n=0; n<nBufferFrames; n++ ) {
			short int v=(short int)(gain*buff[n]);
			if(v < amin)amin=v;
			if(v > amax)amax=v;
			buffer[2*n] = v;
			buffer[2*n+1]=v;
		}
		
		if(outfile){
			size_t ret=sf_write_short(outfile,buff,nBufferFrames);
			if(ret == 0){
				winout("Error sf_write_short \n");
			}
		}
		//winout("ibuff %u amax %g amin %g nBufferFrames %u gain %g\n",ibuff,amax,amin,nBufferFrames,gain);
	}else{
		for (unsigned int n=0; n<nBufferFrames*2; n++ ) {
			  *buffer++ = 0;
		}
		
		if(bS)winout("Audio Record %lld No Data\n",record);
	}
	
	audioSync=0;
	
	++record;
    
  return 0;
}

int soundClass::startSound()
{
	
	//winout("startSound Sleep faudio %g\n",faudio);
	
	if(soundRun)return 0;
	
	printAudio();
	
	soundRun=1;
	
	RtAudio dac;

	RtAudio::StreamParameters parameters;
	parameters.deviceId = dac.getDefaultOutputDevice();
	if(outputID >= 0)parameters.deviceId = outputID;
	parameters.nChannels = 2;
	parameters.firstChannel = 0;
	unsigned int bufferFrames = (unsigned int)(faudio/ncut);

	try {
#ifndef RTAUDIO_OLD
		RtAudio::StreamOptions options;
		dac.openStream( &parameters, NULL, RTAUDIO_SINT16,
						(unsigned int)faudio, &bufferFrames, &sound2, (void *)this, &options);
#else
		dac.openStream( &parameters, NULL, RTAUDIO_SINT16,
						(unsigned int)faudio, &bufferFrames, &sound2, (void *)this );
#endif
		dac.startStream();
	}
	catch (...) {
		winout("openStream error\n");
		//e.printMessage();
		exit( 0 );
	}
	
	while(soundRun >= 0){
	    //winout("startSound Sleeping %d\n",soundRun);
		Sleep2(20);
	}
	
	if (dac.isStreamOpen() ){
		try {
		   //winout("call stopStream2\n");
			// Stop the stream
			dac.stopStream();
		  // winout("return  stopStream2\n");
		}
		catch (...) {
			winout("stopStream error\n");
			//e.printMessage();
			exit( 0 );
		}
		 dac.closeStream();
	}
	
	soundRun=0;

	//winout("startSound soundRun %d  end\n",soundRun);

	return 0;
}

int soundClass::printAudio()
{	
	RtAudio dac;

	unsigned int deviceCount=dac.getDeviceCount();

	if (deviceCount < 1 ) {
		winout("\nNo audio devices found!\n");
		exit( 0 );
	}

	inputNames.clear();

	outputNames.clear();

	RtAudio::DeviceInfo info;

	
#ifndef RTAUDIO_OLD
	std::vector<unsigned int> id=dac.getDeviceIds();
#else
	std::vector<unsigned int> id;
	for(unsigned int n=0;n<deviceCount;++n){
		id.push_back(n);
	}
#endif
	
	int flag=0;

	if(flag == 0)winout("Number of Devices : %ld\n",(long)id.size());

	struct names name;
	
	for (unsigned int i=0; i<deviceCount; i++) {

		try {
			info=dac.getDeviceInfo(id[i]);
			if(info.outputChannels > 0){
			// Print, for example, the maximum number of output channels for each device
				if(flag == 0)winout("audio device = %d : output  channels = %d Device Name = %s",id[i],info.outputChannels,info.name.c_str());
				name.name=info.name;
				name.deviceID=id[i];
				name.sampleRate.clear();
				if(info.sampleRates.size()){
					if(flag == 0)winout(" sampleRates = ");
					for (int ii = 0; ii < (int)info.sampleRates.size(); ++ii){
						if(flag == 0)winout(" %d ",info.sampleRates[ii]);
						name.sampleRate.push_back(info.sampleRates[ii]);
				   }
				}
				if(flag == 0)winout("\n");
				outputNames.push_back(name);
			 }
 
			if(info.inputChannels > 0){
			// Print, for example, the maximum number of output channels for each device
				if(flag == 0)winout("audio device = %d : input   channels = %d Device Name = %s",id[i],info.inputChannels,info.name.c_str());
				name.name=info.name;
				name.deviceID=id[i];
				name.sampleRate.clear();
				if(info.sampleRates.size()){
					if(flag == 0)winout(" sampleRates = ");
					for (int ii = 0; ii < (int)info.sampleRates.size(); ++ii){
						if(flag == 0)winout(" %d ",info.sampleRates[ii]);
						name.sampleRate.push_back(info.sampleRates[ii]);
			        }
				}
				if(flag == 0)winout("\n");
				inputNames.push_back(name);
		   }

		}
		catch (...) {
			winout("Error Doing Audio\n");
			break;
		}

	}
		
	return 1;
}
