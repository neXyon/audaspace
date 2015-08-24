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
	* Output buffers into which output of the partial convolutions will be written.
	*/
	std::vector<sample_t*> m_fftOutBuffers;

	/**
	* A vector of FFTConvolvers used to calculate the partial convolutions.
	*/
	std::vector<std::unique_ptr<FFTConvolver>> m_fftConvolvers;

	/**
	* The max number of threads that can be used.
	*/
	int m_maxThreads;

	/**
	* The actual number of threads being used.
	*/
	int m_numThreads;

	/**
	* A vector of thread objects.
	*/
	std::vector<std::thread> m_threads;

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
	* An input buffer for the various FFTConvolvers
	*/
	sample_t* m_inBuffer;

	/**
	* The output buffer in which all the partial results are written and summed.
	*/
	sample_t* m_outBuffer;

	/**
	* The length of the m_outBuffer.
	*/
	int m_bufLength;

	/**
	* The complete length of the impulse response.
	*/
	int m_irLength;
	
	/**
	* The length of the last input slice.
	*/
	int m_inLength;

	/**
	* The position from which the m_outBuffer is being read.
	*/
	int m_readPosition;

	/**
	* The position from which the m_outBuffer is being written.
	*/
	int m_writePosition;

	/**
	* A flag that determines if the object must be reset before convolving more data.
	*/
	bool m_soundEnded;

	/**
	* The buffer position that contains the end of the valid data.
	*/
	int m_endPosition;

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
	Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int irLength, int nThreads = 4, bool measure = false);

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
	Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int M, int L, int N, int irLength, int nThreads=4, bool measure = false);
	virtual ~Convolver();

	/**
	* Convolves the data that is provided with the inpulse response.
	* \param buffer[in] A buffer with the input data to be convolved and into which the convolved data will be written.
	* \param length[in,out] The number of samples to be convolved (the length of both the buffer).
	*						The convolution output should be larger than the input, but since the 
	*						overlap	add method is used, the extra length will be saved internally.
	*						It must be equal or lower than L or the call will fail, setting this 
	*						variable to 0 since no data would be written in the buffer.
	*/
	void getNext(sample_t* buffer, int& length);

	/**
	* Gets the extra data which is generated as result of the convolution. This method calls endSound() when it is used for the 
	* first time after a reset, so calling reset will be needed to be able to convolve more data.
	* \param[in,out] length The count of samples that should be read. Shall
	*                contain the real count of samples after reading, in case
	*                there were only fewer samples available.
	*                A smaller value also indicates the end of the data.
	* \param[out] eos End of stream, whether the end is reached or not.
	* \param[in] buffer The pointer to the buffer to read into.
	*/
	void getRest(int& length, bool& eos, sample_t* buffer);

	/**
	* Resets all the internally stored data so the convolution of a new sound can be started. 
	*/
	void reset();

private:
	/**
	* This method should only be called when all the sound data has already been convolved and getting the extra data
	* generated in the convolution is needed. To convolve more data after using this method, the reset() method must be called.
	* This method is called by getRest().
	*/
	void endSound();

	/**
	* The function that will be asigned to the different threads.
	* \param id The id of the thread, starting with 0.
	*/
	void threadFunction(int id);

	/**
	* This funtion will be called from threadFunction(), and will process the input signal with a subset of the impulse 
	* response parts, depending on the id of the thread and the total number of threads.
	* \param id The id of the thread, that executes this method, starting with 0.
	*/
	void processSignalFragment(int id);
};

AUD_NAMESPACE_END