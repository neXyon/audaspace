#pragma once

/**
* @file FFTPlan.h
* @ingroup fx
* The FFTPlan class.
*/

#include "fftw3.h"
#include "Audaspace.h"

#include <memory>
#include <vector>

#define DEFAULT_N 2048

AUD_NAMESPACE_BEGIN

/**
*
*/
class AUD_API FFTPlan
{
private:
	/**
	* The size of the FFT plan.
	*/
	int m_N;

	/**
	* The plan to transform the input to the frequency domain.
	*/
	fftwf_plan m_fftPlanR2C;

	/**
	* The plan to transform the input to the time domain again.
	*/
	fftwf_plan m_fftPlanC2R;

	/**
	* The size of a buffer for its use with the FFT plan (in bytes).
	*/
	unsigned int m_bufferSize;

	// delete copy constructor and operator=
	FFTPlan(const FFTPlan&) = delete;
	FFTPlan& operator=(const FFTPlan&) = delete;

public:
	/**
	* Creates a new FFTPlan object with DEFAULT_N size.
	* \param measure A flag that will change how the plan will be created.
	*		-If true the plan creation will take longer, but the FFT and IFFT methods will be faster.
	*		-If false the plan creation will be faster, but the FFT and IFFT methods will be a bit slower.
	*/
	FFTPlan(bool measure = false);

	/**
	* Creates a new FFTPlan object with a custom size.
	* \param n The size of the FFT plan.
	* \param measure A flag that will change how the plan will be created.
	*		-If true the plan creation will take longer, but the FFT and IFFT methods will be faster.
	*		-If false the plan creation will be faster, but the FFT and IFFT methods will be a bit slower.
	*/
	FFTPlan(int n, bool measure = false);
	~FFTPlan();

	/**
	* Retrieves the size of the FFT plan.
	* \return The size of the plan.
	*/
	int getSize();

	/**
	* Calculates the FFT of an input buffer with the current plan.
	* \param[in] buffer A buffer with the input data an in which the output data will be written.
	*/
	void FFT(void* buffer);

	/**
	* Calculates the IFFT of an input buffer with the current plan.
	* \param[in] buffer A buffer with the input data an in which the output data will be written.
	*/
	void IFFT(void* buffer);

	/**
	* Reserves memory for a buffer that can be used for inplace transformations with this plan.
	* \return A pointer to a buffer of size ((N/2)+1)*2*sizeof(fftwf_complex).
	* \warning The returned buffer must be freed with the freeBuffer method of this class.
	*/
	void* getBuffer();

	/**
	* Frees one of the buffers reserved with the getRealOnlyBuffer(), getComplexOnlyBuffer() or getInplaceBuffer() method.
	* \param a pointer to the buufer taht must be freed.
	*/
	void freeBuffer(void* buffer);
};

AUD_NAMESPACE_END