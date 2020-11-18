//////////////////////////////////////////////////////////////////////
// agc.h: interface for the CAgc class.
//
//  This class implements an automatic gain function.
//
// History:
//	2010-09-15  Initial creation MSW
//	2011-03-27  Initial release
//////////////////////////////////////////////////////////////////////
//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
//Copyright 2010 Moe Wheatley. All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are
//permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice, this list of
//	  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice, this list
//	  of conditions and the following disclaimer in the documentation and/or other materials
//	  provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY Moe Wheatley ``AS IS'' AND ANY EXPRESS OR IMPLIED
//WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Moe Wheatley OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//The views and conclusions contained in the software and documentation are those of the
//authors and should not be interpreted as representing official policies, either expressed
//or implied, of Moe Wheatley.
//=============================================================================
#ifndef AGCX_H
#define AGCX_H

#include "datatypes.h"
//#include <QMutex>

#define MAX_DELAY_BUF 2048

class CAgc
{
public:
	CAgc();
	virtual ~CAgc();
	void SetParameters(bool AgcOn, bool UseHang, int Threshold, int ManualGain, int Slope, int Decay, TYPEREAL SampleRate);
	void ProcessData(int Length, TYPECPX* pInData, TYPECPX* pOutData);
	void ProcessData(int Length, TYPEREAL* pInData, TYPEREAL* pOutData);
    double agcGain;

private:
	bool m_AgcOn;				//internal copy of AGC settings parameters
	bool m_UseHang;
	int m_Threshold;
	int m_ManualGain;
	int m_Decay;
	TYPEREAL m_SampleRate;

	TYPEREAL m_SlopeFactor;
	TYPEREAL m_ManualAgcGain;

	TYPEREAL m_DecayAve;
	TYPEREAL m_AttackAve;

	TYPEREAL m_AttackRiseAlpha;
	TYPEREAL m_AttackFallAlpha;
	TYPEREAL m_DecayRiseAlpha;
	TYPEREAL m_DecayFallAlpha;

	TYPEREAL m_FixedGain;
	TYPEREAL m_Knee;
	TYPEREAL m_GainSlope;
	TYPEREAL m_Peak;

	int m_SigDelayPtr;
	int m_MagBufPos;
	int m_DelaySamples;
	int m_WindowSamples;
	int m_HangTime;
	int m_HangTimer;

	//QMutex m_Mutex;		//for keeping threads from stomping on each other
	TYPECPX m_SigDelayBuf[MAX_DELAY_BUF];
	TYPEREAL m_MagBuf[MAX_DELAY_BUF];
};
#endif //  AGCX_H
