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

#define FIXED_N 8192

AUD_NAMESPACE_BEGIN
/**
* This class allows to easily convolve a sound using the Fourier transform and the overlap add method.
*/
class AUD_API FFTConvolver
{
private:
	/**
	* The length of the impulse response.
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
	* The real length of the internal buffer.
	*/
	int m_realBufLen;

	/**
	* The internal buffer.
	*/
	void* m_inBuffer;

	/**
	* A buffer to store the extra data obtained after each partial convolution.
	*/
	float* m_tail;

	/**
	* The provided impulse response.
	*/
	std::shared_ptr<std::vector<fftwf_complex>> m_irBuffer;

	/**
	* The plan to transform the input to the frequency domain (FFTW specific).
	*/
	fftwf_plan m_fftPlanR2C;

	/**
	* The plan to transform the input to the time domain again (FFTW specific).
	*/
	fftwf_plan m_fftPlanC2R;

	/**
	* If the tail is being read, this marks the current position.
	*/
	int m_tailPos;

	// delete copy constructor and operator=
	FFTConvolver(const FFTConvolver&) = delete;
	FFTConvolver& operator=(const FFTConvolver&) = delete;

public:
	/**
	* Creates a new FFTConvolver. This constructor uses the default value of N
	* \param ir A shared pointer to a vector with the impulse response data in the frequency domain (see ImpulseResponse class for an easy way to obtain it).
	* \param measure A flag that will change how the object will be instanced.
	*		-If true the object creation will take a long time, but convolution will be faster.
	*		-If false the object creation will be fast, but convolution will be a bit slower.
	*/
	FFTConvolver(std::shared_ptr<std::vector<fftwf_complex>> ir, bool measure = false);

	/**
	* Creates a new FFTConvolver.
	* \param ir A shared pointer to a vector with the impulse response datas in the frequency domain (see ImpulseResponse class for an easy way to obtain it).
	* \param M The number of samples of the impulse response.
	* \paran L The max number of samples that can be processed at a time.
	* \param N Must be at least M+L-1, but larger values are possible, the performance will be better if N is a power of 2
	* \param measure A flag that will change how the object will be instanced.
	*		-If true the object creation will take a long time, but convolution will be faster.
	*		-If false the object creation will be fast, but convolution will be a bit slower.
	*/
	FFTConvolver(std::shared_ptr<std::vector<fftwf_complex>> ir, int M, int L, int N, bool measure = false);
	virtual ~FFTConvolver();

	/**
	* Convolves the data that is provided with the inpulse response
	* \param inBuffer[in] A buffer with the input data to be convolved.
	* \param outBuffer[in] A pointer to the buffer in which the convolution result will be written.
	* \param length[in,out] The number of samples to be convolved (the length of both the inBuffer and the outBuffer). 
	*						The convolution output should be larger than the input, but since this class uses the overlap 
	*						add method, the extra length will be saved internally.
	*						It must be equal or lower than L or the call will fail, setting this variable to 0 since no data would be
	*						written in the outBuffer.
	*/
	void getNext(const sample_t* inBuffer, sample_t* outBuffer, int& length);
	
	/**
	* Gets the internally stored extra data which is result of the convolution.
	* \param[in,out] length The count of samples that should be read. Shall
	*                contain the real count of samples after reading, in case
	*                there were only fewer samples available.
	*                A smaller value also indicates the end of the data.
	* \param[out] eos End of stream, whether the end is reached or not.
	* \param[in] buffer The pointer to the buffer to read into.
	*/
	void getTail(int& length, bool& eos, sample_t* buffer);

	/**
	* Resets the internally stored data so a new convolution can be started.
	*/
	void clearTail();
};

AUD_NAMESPACE_END
