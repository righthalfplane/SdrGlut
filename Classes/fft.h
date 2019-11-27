// fft.h: interface for the CFft class.
//
//  This is a somewhat modified version of Takuya OOURA's
//     original radix 4 FFT package.
// A C++ wrapper around his code plus some specialized methods
// for displaying power vs frequency
//
//Copyright(C) 1996-1998 Takuya OOURA
//    (email: ooura@mmm.t.u-tokyo.ac.jp).
//
// History:
//	2010-09-15  Initial creation MSW
//	2011-03-27  Initial release
//////////////////////////////////////////////////////////////////////
#ifndef FFT_H
#define FFT_H

#include <stdio.h>
#include "datatypes.h"

#define MAX_FFT_SIZE 65536
#define MIN_FFT_SIZE 512

class CFft
{
public:
	CFft();
	virtual ~CFft();
	void SetFFTParams( qint32 size,
						bool invert,
						TYPEREAL dBCompensation,
						TYPEREAL SampleFreq);
	//Methods to obtain spectrum formated power vs frequency
	void SetFFTAve( qint32 ave);
	void ResetFFT();
	bool GetScreenIntegerFFTData(qint32 MaxHeight, qint32 MaxWidth,
									TYPEREAL MaxdB, TYPEREAL MindB,
									qint32 StartFreq, qint32 StopFreq,
									qint32* OutBuf );
	qint32 PutInDisplayFFT(qint32 n, TYPECPX* InBuf);

	//Methods for doing Fast convolutions using forward and reverse FFT
	void FwdFFT( TYPECPX* pInOutBuf);
	void RevFFT( TYPECPX* pInOutBuf);

private:
	void FreeMemory();
	void makewt(qint32 nw, qint32 *ip, TYPEREAL *w);
	void makect(qint32 nc, qint32 *ip, TYPEREAL *c);
	void bitrv2(qint32 n, qint32 *ip, TYPEREAL *a);
	void cftfsub(qint32 n, TYPEREAL *a, TYPEREAL *w);
	void rftfsub(qint32 n, TYPEREAL *a, qint32 nc, TYPEREAL *c);
	void CpxFFT(qint32 n, TYPEREAL *a, TYPEREAL *w);
	void cft1st(qint32 n, TYPEREAL *a, TYPEREAL *w);
	void cftmdl(qint32 n, qint32 l, TYPEREAL *a, TYPEREAL *w);
	void bitrv2conj(int n, int *ip, TYPEREAL *a);
	void cftbsub(int n, TYPEREAL *a, TYPEREAL *w);

	bool m_Overload;
	bool m_Invert;
	qint32 m_AveCount;
	qint32 m_TotalCount;
	qint32 m_FFTSize;
	qint32 m_LastFFTSize;
	qint32 m_AveSize;
	qint32 m_StartFreq;
	qint32 m_StopFreq;
	qint32 m_BinMin;
	qint32 m_BinMax;
	qint32 m_PlotWidth;

	TYPEREAL m_K_C;
	TYPEREAL m_K_B;
	TYPEREAL m_dBCompensation;
	TYPEREAL m_SampleFreq;
	qint32* m_pWorkArea;
	qint32* m_pTranslateTbl;
	TYPEREAL* m_pSinCosTbl;
	TYPEREAL* m_pWindowTbl;
	TYPEREAL* m_pFFTPwrAveBuf;
	TYPEREAL* m_pFFTAveBuf;
	TYPEREAL* m_pFFTSumBuf;
	TYPEREAL* m_pFFTInBuf;
};

#endif // FFT_H
