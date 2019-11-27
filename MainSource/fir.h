//////////////////////////////////////////////////////////////////////
// fir.h: interface for the CFir class.
//
//  This class implements a FIR  filter using a dual flat coefficient
//array to eliminate testing for buffer wrap around.
//
//Also a decimate by 3 half band filter class CDecimateBy2 is implemented
//
// History:
//	2011-01-29  Initial creation MSW
//	2011-03-27  Initial release
//	2011-08-05  Added decimate by 2 class
//	2011-08-07  Modified FIR filter initialization
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
#ifndef FIR_H
#define FIR_H

#include "datatypes.h"

#define MAX_NUMCOEF 75
////////////
//class for FIR Filters
////////////
class CFir
{
public:
    CFir();

	void InitConstFir( int NumTaps, const TYPEREAL* pCoef, TYPEREAL Fsamprate);
	void InitConstFir( int NumTaps, const TYPEREAL* pICoef, const TYPEREAL* pQCoef, TYPEREAL Fsamprate);
	int InitLPFilter(int NumTaps, TYPEREAL Scale, TYPEREAL Astop, TYPEREAL Fpass, TYPEREAL Fstop, TYPEREAL Fsamprate);
	int InitHPFilter(int NumTaps, TYPEREAL Scale, TYPEREAL Astop, TYPEREAL Fpass, TYPEREAL Fstop, TYPEREAL Fsamprate);
	void GenerateHBFilter( TYPEREAL FreqOffset);
	void ProcessFilter(int InLength, TYPEREAL* InBuf, TYPEREAL* OutBuf);
	void ProcessFilter(int InLength, TYPEREAL* InBuf, TYPECPX* OutBuf);
	void ProcessFilter(int InLength, TYPECPX* InBuf, TYPECPX* OutBuf);

// private:
	TYPEREAL Izero(TYPEREAL x);
	TYPEREAL m_SampleRate;
	int m_NumTaps;
	int m_State;
	TYPEREAL m_Coef[MAX_NUMCOEF*2];
	TYPEREAL m_ICoef[MAX_NUMCOEF*2];
	TYPEREAL m_QCoef[MAX_NUMCOEF*2];
	TYPEREAL m_rZBuf[MAX_NUMCOEF];
	TYPECPX m_cZBuf[MAX_NUMCOEF];
};

////////////
//class for the Half Band decimate by 2 FIR filters
////////////
class CDecimateBy2
{
public:
	CDecimateBy2(int len, const TYPEREAL* pCoef);
	~CDecimateBy2(){if(m_pHBFirRBuf) delete m_pHBFirRBuf; if(m_pHBFirCBuf) delete m_pHBFirCBuf;}
	int DecBy2(int InLength, TYPEREAL* pInData, TYPEREAL* pOutData);
	int DecBy2(int InLength, TYPECPX* pInData, TYPECPX* pOutData);
	TYPEREAL* m_pHBFirRBuf;
	TYPECPX* m_pHBFirCBuf;
	int m_FirLength;
	const TYPEREAL* m_pCoef;
};

#endif // FIR_H
