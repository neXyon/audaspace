#pragma once

/**
* @file ConvolverSound.h
* @ingroup fx
* The ConvolverSound class.
*/

#include "ISound.h"
#include "ImpulseResponse.h"

#include <memory>
#include <vector>

AUD_NAMESPACE_BEGIN

/**
* This class represents a sound that can be modified depending on a given impulse response
*/
class AUD_API ConvolverSound : public ISound
{
private:
	/**
	* A pointer to the imput sound.
	*/
	std::shared_ptr<ISound> m_sound;

	/**
	* A pointer to the impulse response.
	*/
	std::shared_ptr<ImpulseResponse> m_impulseResponse;

	/**
	*The max number of threads this sound will use when playing(per channel)
	*/
	int m_nThreads;

	// delete copy constructor and operator=
	ConvolverSound(const ConvolverSound&) = delete;
	ConvolverSound& operator=(const ConvolverSound&) = delete;

public:
	/**
	* Creates a new ConvolverSound.
	* \param sound The sound that will be convolved.
	* \param impulseResponse The impulse response sound.
	* \param nThreads The max number of threads per channel that this sound can use when playing (default 4) .
	*/
	ConvolverSound(std::shared_ptr<ISound> sound, std::shared_ptr<ImpulseResponse> impulseResponse, int nThreads=4);

	virtual std::shared_ptr<IReader> createReader();

	/**
	* Retrieves the impulse response sound being used.
	* \return A shared pointer to the current impulse response being used.
	*/
	std::shared_ptr<ImpulseResponse> getImpulseResponse();

	/**
	* Changes the inpulse response used for convolution, it'll only affect newly created readers.
	* \param impulseResponse A shared pointer to the new impulse response sound.
	*/
	void setImpulseResponse(std::shared_ptr<ImpulseResponse> impulseResponse);
};

AUD_NAMESPACE_END