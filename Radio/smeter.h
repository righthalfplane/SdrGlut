//////////////////////////////////////////////////////////////////////
// smeter.h: interface for the CSMeter class.
//
// History:
//	2010-09-22  Initial creation MSW
//	2011-03-27  Initial release
/////////////////////////////////////////////////////////////////////
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
#ifndef SMETER_H
#define SMETER_H
#include "datatypes.h"

class CSMeter
{
public:
    CSMeter();
    void Reset();
    void ProcessData(int length, TYPECPX* pInData, TYPEREAL SampleRate);
	TYPEREAL GetPeak();
	TYPEREAL GetAve();
	void SetSMeterCalibration(TYPEREAL offset) { m_CalibrationOffset=offset;}

private:
	TYPEREAL m_AverageMag;
	TYPEREAL m_PeakMag;
	TYPEREAL m_SampleRate;
	TYPEREAL m_AttackAve;
	TYPEREAL m_DecayAve;
	TYPEREAL m_AttackAlpha;
	TYPEREAL m_DecayAlpha;
	TYPEREAL m_CalibrationOffset;
};

#endif // SMETER_H
