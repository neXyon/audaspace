#pragma once

/**
* @file VolumeSound.h
* @ingroup fx
* The VolumeSound class.
*/

#include "ISound.h"
#include "VolumeStorage.h"
#include <memory>

AUD_NAMESPACE_BEGIN

/**
* Ths class allows to create a sound with its own volume.
*/
class AUD_API VolumeSound : public ISound
{
private:
	/**
	* A pointer to a sound.
	*/
	std::shared_ptr<ISound> m_sound;

	std::shared_ptr<VolumeStorage> m_volumeStorage;

	// delete copy constructor and operator=
	VolumeSound(const VolumeSound&) = delete;
	VolumeSound& operator=(const VolumeSound&) = delete;

public:
	/**
	* Creates a new VolumeSound.
	* \param The sound in which shall have its own volume.
	* \param volumeStorage A shared pointer to a VolumeStorage object. It allows to change the volume of various sound in one go.
	*/
	VolumeSound(std::shared_ptr<ISound> sound, std::shared_ptr<VolumeStorage> volumeStorage);

	virtual std::shared_ptr<IReader> createReader();

	/**
	* Retrieves the shared volume of this sound.
	* \return A shared pointer to the VolumeStorage object that this sound is using.
	*/
	std::shared_ptr<VolumeStorage> getSharedVolume();

	/**
	* Changes the shared volume of this sound, it'll only affect newly created readers.
	* \param volumeStorage A shared pointer to the new VolumeStorage object.
	*/
	void setSharedVolume(std::shared_ptr<VolumeStorage> volumeStorage);
};

AUD_NAMESPACE_END