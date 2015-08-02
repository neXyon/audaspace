#pragma once

/**
* @file PlaybackCategory.h
* @ingroup fx
* The PlaybackCategory class.
*/

#include "devices/IHandle.h"
#include "devices/IDevice.h"
#include "VolumeStorage.h"

#include <list>
#include <memory>

AUD_NAMESPACE_BEGIN

/**
* This class represents a category of related sounds which are currently playing and allows to control them easily.
*/
class AUD_API PlaybackCategory
{
private:
	/**
	* Vector of handles that belong to the category.
	*/
	std::list<std::shared_ptr<IHandle>> m_handles;

	/**
	* Device that will play the sounds.
	*/
	std::shared_ptr<IDevice> m_device;

	/**
	* Status of the category.
	*/
	Status m_status;

	/**
	* Volume of all the sounds of the category.
	*/
	std::shared_ptr<VolumeStorage> m_volumeStorage;

	// delete copy constructor and operator=
	PlaybackCategory(const PlaybackCategory&) = delete;
	PlaybackCategory& operator=(const PlaybackCategory&) = delete;

public:
	/**
	* Creates a new PlaybackCategory.
	* \param A shared pointer to the device which will be used for playback.
	*/
	PlaybackCategory(std::shared_ptr<IDevice> device);

	/**
	* Adds a new handle to the category.
	* \param handle The handle to be added.
	*/
	std::shared_ptr<IHandle> play(std::shared_ptr<ISound> sound);

	/**
	* Resumes all the paused sounds of the category.
	*/
	void resume();

	/**
	* Pauses all current played back sounds of the category.
	*/
	void pause();

	/**
	* Retrieves the volume of the category.
	* \return The volume.
	*/
	float getVolume();

	/**
	* Sets the volume for the category.
	* \param volume The volume.
	*/
	void setVolume(float volume);

	/**
	* Stops all the playing back or paused sounds.
	*/
	void stop();

	/**
	* Retrieves the shared volume of the category.
	* \return A shared pointer to the VolumeStorage object that represents the shared volume of the category.
	*/
	std::shared_ptr<VolumeStorage> getSharedVolume();

	/**
	* Cleans the category erasing all the invalid handles.
	*/
	void cleanHandles();
};

AUD_NAMESPACE_END
