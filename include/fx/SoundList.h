#pragma once

/**
* @file SoundList.h
* @ingroup fx
* The SoundList class.
*/

#include "ISound.h"

#include <vector>
#include <memory>

AUD_NAMESPACE_BEGIN

/**
* 
*/
class AUD_API SoundList : public ISound
{
private:
	/**
	* The list of sounds that will play
	*/
	std::vector<std::shared_ptr<ISound>> m_list;

	/**
	* Flag for random playback
	*/
	bool m_random = false;

	/**
	* Current sound index. -1 if no reader has been created;
	*/
	int m_index = -1;

	// delete copy constructor and operator=
	SoundList(const SoundList&) = delete;
	SoundList& operator=(const SoundList&) = delete;

public:
	/**
	* Creates a new, empty sound list.
	* Sounds must be added to the list using the addSound() method. 
	*/
	SoundList();

	virtual std::shared_ptr<IReader> createReader();

	/**
	* Adds a sound to the list.
	* The added sounds can be played sequentially or randomly dependig 
	* on the m_random flag
	* \param sound A shared_ptr to the sound.
	* \return The index of the added sound.
	*/
	int addSound(std::shared_ptr<ISound> sound);

	/**
	* Removes a sound from the list.
	* \param index The index of the sound.
	*/
	void removeSound(int index);

	/**
	* Sets the playback mode of the sound list.
	* The two posible modes are random and sequential.
	* \param random True to activate the random mode, false to activate sequential mode.
	*/
	void setRandomMode(bool random);

	/**
	* Returns the playback mode of the sound list.
	* The two posible modes are random and sequential.
	* \return True if the random mode is activated, false otherwise.
	*/
	bool getRandomMode();

};

AUD_NAMESPACE_END
