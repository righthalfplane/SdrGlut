
#include "sdrReceive.h"

#include "cStack.h"

#include <iostream>
#include <vector>

#include <string.h>


int setFilters(struct playData *rx,struct Filters *f);

int findRadio(struct playData *rx);

int Process(void *rxv);


double rtime(void);

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );



struct fmultStruct{
	double dt;
	double sino;
	double coso;
	double w;
	double sindt;
	double cosdt;
	double gain;
};

struct frequencyStruct{
    double frequency;
    int flag;
};

class cReceive;

class cDemod{
public:
    cDemod();
	~cDemod();
	int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );
         
	int rxBuffer(void *rxv);
	int process(struct playData *rxi);
	void processAll();
	struct playData *rx;
	struct Filters f;
	float *bAudio;
	float *bRf;
};

class cReceive{
public:
    cReceive(int argc, char * argv []);
	~cReceive();
	int initPlay(struct playData *rx);
	int playRadio(struct playData *rx);
	int stopPlay(struct playData *rx);
	int updateLine();
	int updateSweep1(double fmins,double fmaxs);
	int updateSweep2(double fmins,double fmaxs,int pass);
	int updateSweep3(double fmins,double fmaxs);
	int sweepRadio();
	int fftIndex(double frequency);
	int setFrequency3(double frequency);
	int processScan(struct playData *rx);
	void mprint(const char *fmt, ...);
	int printInfo(struct playData *rx);
	int printAudioInfo(struct playData *rx);
	int findRadio(struct playData *rx);
	struct playData rxs;
	struct playData *rx;
	class cDemod *d;
	std::vector<frequencyStruct> frequency;
	std::vector<fmultStruct> fmult;
	std::vector<int> kill;
	long FFTlength;

	double lineTime;
    double lineDumpInterval;
    double lineAlpha;
    
	double *range;
	double *range3;
	double *magnitude;
	double *magnitude2;
	double *magnitude3;
	double *frequencies;
	double *ampitude;
	
	int scanRun;
    int scanWait;
    
    std::vector<double> scanFrequencies;

    int scanFound[200];
    double pauseTime;
    double pauseTimeDelta;
    int pauseChannel;
    
    
    int iplay;
};
