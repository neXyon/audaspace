#pragma once

/**
* @file FFTConvolver.h
* @ingroup fx
* The FFTConvolver class.
*/

#include "IReader.h"
#include "ISound.h"
#include "fftw3.h"

#include <memory>
AUD_NAMESPACE_BEGIN

/**
* This class represents a reader for a sound that has its own shared volume
*/
class AUD_API FFTConvolver
{
private:
	int m_M;
	int m_L;
	int m_N;
	int m_Npow2;
	int m_realBufLen;

	void* m_inBuffer;
	void* m_irBuffer;
	float* m_tail;

	fftwf_plan m_fftPlanR2C;
	fftwf_plan m_fftPlanC2R;

	int m_tailPos;

	// delete copy constructor and operator=
	FFTConvolver(const FFTConvolver&) = delete;
	FFTConvolver& operator=(const FFTConvolver&) = delete;
	FFTConvolver() = delete;

public:
	FFTConvolver(sample_t* irBuffer, int M, int L, bool measure=false);
	virtual ~FFTConvolver();

	void getNext(const sample_t* inBuffer, sample_t* outBuffer, int length);
	void getTail(int& length, bool& eos, sample_t* buffer);
};

AUD_NAMESPACE_END
