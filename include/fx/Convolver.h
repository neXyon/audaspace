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
#include <condition_variable>
#include <atomic>
#include <deque>

AUD_NAMESPACE_BEGIN
/**
* This class allows to convolve a sound with a very large impulse response.
*/
class AUD_API Convolver
{
private:
	/**
	* The length of the impulse response parts.
	*/
	int m_M;

	/**
	* The max length of the input slices.
	*/
	int m_L;

	/**
	* The FFT size, must be at least M+L-1.
	*/
	int m_N;

	/**
	* The impulse response divided in parts.
	*/
	std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> m_irBuffers;

	/**
	* Accumulation buffers for the threads.
	*/
	std::vector<fftwf_complex*> m_threadAccBuffers;

	/**
	* A vector of FFTConvolvers used to calculate the partial convolutions.
	*/
	std::vector<std::unique_ptr<FFTConvolver>> m_fftConvolvers;

	/**
	* The actual number of threads being used.
	*/
	int m_numThreads;

	/**
	* A vector of thread objects.
	*/
	std::vector<std::thread> m_threads;

	/**
	* A mutex for the sum of thread accumulators.
	*/
	std::mutex m_sumMutex;

	/**
	* A vector of mutexes, one per thread.
	*/
	std::vector<std::mutex> m_mutexes;

	/**
	* A vector condition variables, one per thread.
	*/
	std::vector<std::condition_variable> m_conditions;

	/**
	* A flag to control thread execution when a reset is scheduled.
	*/
	std::atomic_bool m_resetFlag;

	/**
	* A flag used to make the threads end as soon as they can.
	*/
	std::atomic_bool m_stopFlag;

	/**
	* Global accumulation buffer.
	*/
	fftwf_complex* m_accBuffer;

	/**
	* Delay line.
	*/
	std::deque<fftwf_complex*> m_delayLine;

	/**
	* The complete length of the impulse response.
	*/
	int m_irLength;

	/**
	* Counter for the tail;
	*/
	int m_tailCounter;

	// delete copy constructor and operator=
	Convolver(const Convolver&) = delete;
	Convolver& operator=(const Convolver&) = delete;

public:

	/**
	* Creates a new FFTConvolver. This constructor uses the default value of N
	* \param ir A shared pointer to a vector with the data of the various impulse response parts in the frequency domain (see ImpulseResponse class for an easy way to obtain it).
	* \param irLength The length of the full impulse response.
	* \param nThreads The number of threads per channel that can be used.
	* \param measure A flag that will change how the object will be instanced.
	*		-If true the object creation will take a long time, but convolution will be faster.
	*		-If false the object creation will be fast, but convolution will be a bit slower.
	*/
	Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int irLength, int nThreads = 1, bool measure = false);

	/**
	* Creates a new FFTConvolver.
	* \param ir A shared pointer to a vector with the data of the various impulse response parts in the frequency domain (see ImpulseResponse class for an easy way to obtain it).
	* \param M The number of samples of the impulse response parts.
	* \paran L The max number of samples that can be processed at a time.
	* \param N Must be at least M+L-1, but larger values are possible, the performance will be better if N is a power of 2
	* \param irLength The length of the full impulse response.
	* \param nThreads The number of threads per channel that can be used.
	* \param measure A flag that will change how the object will be instanced.
	*		-If true the object creation will take a long time, but convolution will be faster.
	*		-If false the object creation will be fast, but convolution will be a bit slower.
	*/
	Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int M, int L, int N, int irLength, int nThreads=1, bool measure = false);
	virtual ~Convolver();

	/**
	* Convolves the data that is provided with the inpulse response.
	* \param[in] inBuffer A buffer with the input data to be convolved, nullptr if the source sound has ended (the convolved sound is larger than the source sound).
	* \param[in] outBuffer A buffer in which the convolved data will be written.
	* \param[in,out] length The number of samples you wish to obtain. If an inBuffer is provided this argument must match its length.
	*						When this method returns, the value of length represent the number of samples written into the outBuffer.
	* \param[out] eos True if the end of the sound is reached, false otherwise.
	*/
	void getNext(sample_t* inBuffer, sample_t* outBuffer, int& length, bool& eos);

	/**
	* Resets all the internally stored data so the convolution of a new sound can be started. 
	*/
	void reset();

private:

	/**
	* The function that will be asigned to the different threads.
	* \param id The id of the thread, starting with 0.
	*/
	void threadFunction(int id);

	/**
	* This function will be called from threadFunction(), and will process the input signal with a subset of the impulse response parts.
	* \param id The id of the thread, that executes this method, starting with 0.
	* \param start The index of the first FFTConvolver object to be used.
	* \param end The index of the last FFTConvolver object to be used +1.
	*/
	void processSignalFragment(int id, int start, int end);
};

AUD_NAMESPACE_END