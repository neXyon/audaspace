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
#include <vector>
AUD_NAMESPACE_BEGIN

/**
* This class allows to easily convolve a sound using the Fourier transform method.
*/
class AUD_API FFTConvolver
{
private:
	int m_M;
	int m_L;
	int m_N;
	int m_realBufLen;

	void* m_inBuffer;
	float* m_tail;
	std::shared_ptr<std::vector<fftwf_complex>> m_irBuffer;

	fftwf_plan m_fftPlanR2C;
	fftwf_plan m_fftPlanC2R;

	int m_tailPos;

	// delete copy constructor and operator=
	FFTConvolver(const FFTConvolver&) = delete;
	FFTConvolver& operator=(const FFTConvolver&) = delete;

public:
	FFTConvolver(std::shared_ptr<std::vector<fftwf_complex>>, int M, int L, int N, bool measure = false);
	virtual ~FFTConvolver();

	void getNext(const sample_t* inBuffer, sample_t* outBuffer, int length);
	void getTail(int& length, bool& eos, sample_t* buffer);
	void clearTail();
};

AUD_NAMESPACE_END
