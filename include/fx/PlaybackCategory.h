#pragma once

/**
* @file PlaybackCategory.h
* @ingroup fx
* The PlaybackCategory class.
*/

#include "devices/IHandle.h"

#include <vector>
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
	std::vector<std::shared_ptr<IHandle>> m_handles;

	// delete copy constructor and operator=
	PlaybackCategory(const PlaybackCategory&) = delete;
	PlaybackCategory& operator=(const PlaybackCategory&) = delete;

public:
	PlaybackCategory();

	/**
	* Adds a new handle to the category.
	* \param handle The handle to be added.
	*/
	void addHandle(std::shared_ptr<IHandle> handle);

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
};

AUD_NAMESPACE_END