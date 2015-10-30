#pragma once

/**
* @file ConvolverSound.h
* @ingroup fx
* The ConvolverSound class.
*/

#include "ISound.h"
#include "ImpulseResponse.h"
#include "util/ThreadPool.h"
#include "util/FFTPlan.h"

#include <memory>
#include <vector>

AUD_NAMESPACE_BEGIN

/**
* This class represents a sound that can be modified depending on a given impulse response.
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
	* A shared ptr to a thread pool.
	*/
	std::shared_ptr<ThreadPool> m_threadPool;

	/**
	* A shared ponter to an FFT plan.
	*/
	std::shared_ptr<FFTPlan> m_plan;

	// delete copy constructor and operator=
	ConvolverSound(const ConvolverSound&) = delete;
	ConvolverSound& operator=(const ConvolverSound&) = delete;

public:
	/**
	* Creates a new ConvolverSound.
	* \param sound The sound that will be convolved.
	* \param impulseResponse The impulse response sound.
	* \param threadPool A shared pointer to a ThreadPool object with 1 or more threads.
	* \param plan A shared pointer to a FFTPlan object that will be used for convolution.
	*/
	ConvolverSound(std::shared_ptr<ISound> sound, std::shared_ptr<ImpulseResponse> impulseResponse, std::shared_ptr<ThreadPool> threadPool, std::shared_ptr<FFTPlan> plan);

	/**
	* Creates a new ConvolverSound. A default FFT plan will be created.
	* \param sound The sound that will be convolved.
	* \param impulseResponse The impulse response sound.
	* \param threadPool A shared pointer to a ThreadPool object with 1 or more threads.
	*/
	ConvolverSound(std::shared_ptr<ISound> sound, std::shared_ptr<ImpulseResponse> impulseResponse, std::shared_ptr<ThreadPool> threadPool);

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