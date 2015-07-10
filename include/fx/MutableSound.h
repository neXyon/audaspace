#pragma once

/**
* @file MutableSound.h
* @ingroup fx
* The MutableSound class.
*/

#include "SoundList.h"

#include <memory>

AUD_NAMESPACE_BEGIN

/**
* This sound allows to have a list of sounds that will play randomly or sequentially with each playback.
* Furthermore, the reader returned with the createReader() method permits to play the list in a loop.
*/
class AUD_API MutableSound : public ISound
{
private:
	/**
	* A pointer to the sound list
	*/
	std::shared_ptr<SoundList> m_sound;

	// delete copy constructor and operator=
	MutableSound(const MutableSound&) = delete;
	MutableSound& operator=(const MutableSound&) = delete;

public:
	/**
	* Creates the sound with an empty list.
	* Sounds must be added to the list using the addSound() method.
	*/
	MutableSound();

	/**
	* Creates a new mutable sound and initializes it.
	* \param list A vector with sounds to initialize the list.
	*/
	MutableSound(std::vector<std::shared_ptr<ISound>>& list);

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
	* There are two posible modes, random and sequential.
	* \param random True to activate the random mode, false to activate sequential mode.
	*/
	void setRandomMode(bool random);

	/**
	* Returns the playback mode of the sound list.
	* The two posible modes are random and sequential.
	* \return True if the random mode is activated, false otherwise.
	*/
	bool getRandomMode();

	/**
	* Returns the amount of sounds in the list.
	* \return The amount of sounds in the list.
	*/
	int getSize();
};

AUD_NAMESPACE_END