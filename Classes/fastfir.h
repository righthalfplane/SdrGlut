//////////////////////////////////////////////////////////////////////
// FastFIR.h: interface for the CFastFIR class.
//
//  This class implements a FIR Bandpass filter using a FFT convolution algorithm
//The filter is complex and is specified with 3 parameters:
// sample frequency, Hicut and Lowcut frequency
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
#ifndef FASTFIR_H
#define FASTFIR_H

#include "datatypes.h"
#include "fft.h"

class CFastFIR  
{
public:
	CFastFIR();
    virtual ~CFastFIR();

	void SetupParameters( TYPEREAL FLoCut,TYPEREAL FHiCut,TYPEREAL Offset, TYPEREAL SampleRate);
	int ProcessData(int InLength, TYPECPX* InBuf, TYPECPX* OutBuf);

private:
	inline void CpxMpy(int N, TYPECPX* m, TYPECPX* src, TYPECPX* dest);
	void FreeMemory();

	TYPEREAL m_FLoCut;
	TYPEREAL m_FHiCut;
	TYPEREAL m_Offset;
	TYPEREAL m_SampleRate;

	int m_InBufInPos;
	TYPEREAL* m_pWindowTbl;
	TYPECPX* m_pFFTOverlapBuf;
	TYPECPX* m_pFilterCoef;
	TYPECPX* m_pFFTBuf;
//	QMutex m_Mutex;		//for keeping threads from stomping on each other
	CFft m_Fft;
};
#endif // FASTFIR_H
