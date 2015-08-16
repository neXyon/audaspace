#pragma once

/**
* @file Convolver.h
* @ingroup fx
* The Convolver class.
*/

#include "FFTConvolver.h"
#include "fftw3.h"

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
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

	std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> m_irBuffers;
	std::vector<sample_t*> m_fftOutBuffers;
	std::vector<std::unique_ptr<FFTConvolver>> m_fftConvolvers;

	int m_numThreads;
	std::vector<std::thread> m_threads;
	std::vector<std::mutex> m_mutexes;
	std::vector<std::condition_variable> m_conditions;
	std::atomic_bool m_resetFlag;
	std::atomic_bool m_stopFlag;

	sample_t* m_inBuffer;
	sample_t* m_outBuffer;
	int m_bufLength;
	int m_irLength;
	int m_inLength;
	int m_readPosition;
	int m_writePosition;

	// delete copy constructor and operator=
	Convolver(const Convolver&) = delete;
	Convolver& operator=(const Convolver&) = delete;

public:
	Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int N, int irLength, bool measure = false);
	virtual ~Convolver();

	void getNext(sample_t* buffer, int& length);
	void getRest(sample_t* buffer, int& length);
	void reset();

private:
	void threadFunction(int id);
	void processSignalFragment(int id);
};

AUD_NAMESPACE_END