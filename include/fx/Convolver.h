#pragma once

/**
* @file Convolver.h
* @ingroup fx
* The Convolver class.
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
class AUD_API Convolver
{
private:
	int m_M;
	int m_L;
	int m_N;

	std::shared_ptr<IReader> m_irReader;
	int m_realBufLen;

	void* m_inBuffer;
	float* m_tail;
	std::shared_ptr<std::vector<std::vector<fftwf_complex>>> m_irBuffer;

	fftwf_plan m_fftPlanR2C;
	fftwf_plan m_fftPlanC2R;

	int m_tailPos;

	// delete copy constructor and operator=
	Convolver(const Convolver&) = delete;
	Convolver& operator=(const Convolver&) = delete;

public:
	Convolver(std::shared_ptr<IReader> ir, int N, bool measure = false);
	virtual ~Convolver();

	void getNext(sample_t* buffer, int length);
	void reset();
};

AUD_NAMESPACE_END