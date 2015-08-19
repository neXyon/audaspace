#pragma once

/**
* @file ImpulseResponse.h
* @ingroup fx
* The ImpulseResponse class.
*/

#include "util/StreamBuffer.h"
#include "FFTConvolver.h"

#include <memory>
#include <vector>

AUD_NAMESPACE_BEGIN

/**
* This class represents an impulse response that can be used in convolution.
* The impulse response will be processed and transformed upon creation.
* The main objetive of this class is to allow the reutilization of an impulse response in various sounds without having to process it more than one time.
*/
class AUD_API ImpulseResponse
{
private:
	/**
	* A tri-dimensional array (channels, parts, values) The impulse response is divided in channels and those channels are divided
	* in parts according to the value of the FIXED_N constant. Those parts are transformed to the frequency domain transform which
	* generates uni-dimensional arrays of fftwtf_complex data (complex numbers).
	*/
	std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>>> m_processedIR;

	/**
	* The number of channels of the impulse response.
	*/
	int m_channels;

	/**
	* The length of the impulse response.
	*/
	int m_length;

	// delete copy constructor and operator=
	ImpulseResponse(const ImpulseResponse&) = delete;
	ImpulseResponse& operator=(const ImpulseResponse&) = delete;

public:
	/**
	* Creates a new ImpulseResponse objects.
	* The impulse response will be split and transformed to the frequency domain.
	* \param impulseResponse The impulse response sound.
	*/
	ImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse);

	/**
	* Retrieves the number of channels of the impulse response.
	* \return The number of channels of the impulse response.
	*/
	int getNumberOfChannels();

	/**
	* Retrieves the length of the impulse response.
	* \return The length of the impulse response.
	*/
	int getLength();

	/**
	* Retrieves one channel of the impulse response.
	* \param n The desired channel number (from 0 to channels-1).
	* \return The desired channel of the impulse response.
	*/
	std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> getChannel(int n);

private:
	/**
	* Processes the impulse response sound for its use in the FFTConvolver class.
	* \param A shared pointer to a reader of the desired sound.
	*/
	void processImpulseResponse(std::shared_ptr<IReader> reader);
};

AUD_NAMESPACE_END
