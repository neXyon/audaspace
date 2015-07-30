#pragma once

/**
* @file PlaybackManager.h
* @ingroup fx
* The PlaybackManager class.
*/

#include "PlaybackCategory.h"
#include "devices\IDevice.h"
#include "ISound.h"

#include <unordered_map>
#include <memory>

AUD_NAMESPACE_BEGIN

/**
* This class allows to control groups of playing sounds easily.
* The sounds are part of categories.
*/
class AUD_API PlaybackManager
{
private:
	/**
	* Unordered map of categories, each category has different name.
	*/
	std::unordered_map<std::string, std::shared_ptr<PlaybackCategory>> m_categories;

	/**
	* Device used for playback.
	*/
	std::shared_ptr<IDevice> m_device;

	// delete copy constructor and operator=
	PlaybackManager(const PlaybackManager&) = delete;
	PlaybackManager& operator=(const PlaybackManager&) = delete;

public:
	PlaybackManager(std::shared_ptr<IDevice> device);

	/**
	* Adds a new handle to an existent category or creates a new one.
	* \param handle The handle to be added.
	* \param catName Name of the category of the sound.
	*/
	void addHandle(std::shared_ptr<IHandle> handle, std::string catName);

	/**
	* Adds an existent category to the manager.
	* \param category The category to be added.
	* \param catName Name of the category.
	*/
	void addCategory(std::shared_ptr<PlaybackCategory> category, std::string catName);

	/**
	* Plays a sound and adds it to a new or existent category.
	* \param sound The sound to be played and added to a category.
	* \param catName Name of the category.
	* \return The handle of the playback.
	*/
	std::shared_ptr<IHandle> play(std::shared_ptr<ISound> sound, std::string catName);

	/**
	* Resumes all the paused sounds of a category.
	* \param catName Name of the category.
	*/
	void resume(std::string catName);

	/**
	* Pauses all current playing sounds of a category.
	* \param catName Name of the category.
	*/
	void pause(std::string catName);

	/**
	* Retrieves the volume of a category.
	* \param catName Name of the category.
	* \return The volume.
	*/
	float getVolume(std::string catName);

	/**
	* Sets the volume for a category.
	* \param volume The volume.
	* \param catName Name of the category.
	*/
	void setVolume(float volume, std::string catName);

	/**
	* Stops and erases a category of sounds.
	* \param catName Name of the category.
	*/
	void stop(std::string catName);
};

AUD_NAMESPACE_END
