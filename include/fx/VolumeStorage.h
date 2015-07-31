#pragma once

/**
* @file PlaybackManager.h
* @ingroup fx
* The PlaybackManager class.
*/

#include "Audaspace.h"

#include <atomic>

AUD_NAMESPACE_BEGIN

/**
* This class stores a volume value and allows to change if for a number of sounds in one go.
*/
class AUD_API VolumeStorage
{
private:
	/**
	* Volume value.
	*/
	std::atomic<float> m_volume;

	// delete copy constructor and operator=
	VolumeStorage(const VolumeStorage&) = delete;
	VolumeStorage& operator=(const VolumeStorage&) = delete;

public:
	/**
	* Creates a new VolumeStorage instance with volume 1
	*/
	VolumeStorage();

	/**
	* Creates a VolumeStorage instance with an initial value.
	* \param volume The value of the volume.
	*/
	VolumeStorage(float volume);

	/**
	* Retrieves the current volume value.
	* \return the current volume.
	*/
	float getVolume();

	/**
	* Changes the volume value.
	* \param volume The new value for the volume.
	*/
	void setVolume(float volume);
};

AUD_NAMESPACE_END
