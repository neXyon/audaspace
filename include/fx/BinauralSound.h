#pragma once

/**
* @file BinauralSound.h
* @ingroup fx
* The BinauralSound class.
*/

#include "ISound.h"
#include "HRTF.h"
#include "Source.h"
#include "util/ThreadPool.h"
#include "util/FFTPlan.h"

#include <memory>
#include <vector>

AUD_NAMESPACE_BEGIN

/**
* This class represents a sound that can sound different depending on its realtive position with the listener.
*/
class AUD_API BinauralSound : public ISound
{
private:
	/**
	* A pointer to the imput sound.
	*/
	std::shared_ptr<ISound> m_sound;

	/**
	* A pointer to an HRTF object with a collection of impulse responses.
	*/
	std::shared_ptr<HRTF> m_hrtfs;

	/**
	* A pointer to a Source object which represents the source of the sound.
	*/
	std::shared_ptr<Source> m_source;

	/**
	* A shared ptr to a thread pool.
	*/
	std::shared_ptr<ThreadPool> m_threadPool;

	/**
	* A shared ponter to an FFT plan.
	*/
	std::shared_ptr<FFTPlan> m_plan;

	// delete copy constructor and operator=
	BinauralSound(const BinauralSound&) = delete;
	BinauralSound& operator=(const BinauralSound&) = delete;

public:
	/**
	* Creates a new ConvolverSound.
	* \param sound The sound that will be convolved. It must have only one channel.
	* \param hrtfs The HRTF set that will be used.
	* \param threadPool A shared pointer to a ThreadPool object with 1 or more threads.
	* \param plan A shared pointer to a FFTPlan object that will be used for convolution.
	*/
	BinauralSound(std::shared_ptr<ISound> sound, std::shared_ptr<HRTF> hrtfs, std::shared_ptr<Source> source, std::shared_ptr<ThreadPool> threadPool, std::shared_ptr<FFTPlan> plan);

	/**
	* Creates a new BinauralSound. A default FFT plan will be created.
	* \param sound The sound that will be convolved. Must have only one channel.
	* \param hrtfs The HRTF set that will be used.
	* \param threadPool A shared pointer to a ThreadPool object with 1 or more threads.
	*/
	BinauralSound(std::shared_ptr<ISound> sound, std::shared_ptr<HRTF> hrtfs, std::shared_ptr<Source> source, std::shared_ptr<ThreadPool> threadPool);

	virtual std::shared_ptr<IReader> createReader();

	/**
	* Retrieves the HRTF set being used.
	* \return A shared pointer to the current HRTF object being used.
	*/
	std::shared_ptr<HRTF> getHRTFs();

	/**
	* Changes the set of HRTFs used for convolution, it'll only affect newly created readers.
	* \param hrtfs A shared pointer to the new HRTF object.
	*/
	void setHRTFs(std::shared_ptr<HRTF> hrtfs);

	/**
	* Retrieves the Source object being used.
	* \return A shared pointer to the current Source object being used.
	*/
	std::shared_ptr<Source> getSource();

	/**
	* Changes the Source object used to change the source position of the sound.
	* \param source A shared pointer to the new Source object.
	*/
	void setSource(std::shared_ptr<Source> source);
};
AUD_NAMESPACE_END