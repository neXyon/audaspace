#pragma once

/**
* @file MutableSound.h
* @ingroup fx
* The MutableSound class.
*/

#include "ISound.h"

#include <memory>

AUD_NAMESPACE_BEGIN

/**
* Ths class allows to create MutableReaders for any sound.
*/
class AUD_API MutableSound : public ISound
{
private:
	/**
	* A pointer to a sound.
	*/
	std::shared_ptr<ISound> m_sound;

	// delete copy constructor and operator=
	MutableSound(const MutableSound&) = delete;
	MutableSound& operator=(const MutableSound&) = delete;

public:
	/**
	* Creates a new MutableSound.
	* \param The sound in which the MutabeReaders created with the createReader() method will be based.
	*		If shared pointer to a SoundList object is used in several mutable sounds the sequential
	*		playback will not work properly. A copy of the SoundList object must be made in this case.
	*/
	MutableSound(std::shared_ptr<ISound> sound);

	virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END