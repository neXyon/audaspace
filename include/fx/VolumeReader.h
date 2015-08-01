#pragma once

/**
* @file VolumeReader.h
* @ingroup fx
* The VolumeReader class.
*/

#include "IReader.h"
#include "ISound.h"
#include "VolumeStorage.h"

#include <memory>

AUD_NAMESPACE_BEGIN

/**
* This class represents a reader for a sound that has its own shared volume
*/
class AUD_API VolumeReader : public IReader
{
private:
	/**
	* The current reader.
	*/
	std::shared_ptr<IReader> m_reader;

	/**
	* A sound from which to get the reader.
	*/
	std::shared_ptr<VolumeStorage> m_volumeStorage;


	// delete copy constructor and operator=
	VolumeReader(const VolumeReader&) = delete;
	VolumeReader& operator=(const VolumeReader&) = delete;

public:
	/**
	* Creates a new volume reader.
	* \param reader A of the sound to be assigned to this reader.
	* \param volumeStorage A shared pointer to a VolumeStorage object.
	*/
	VolumeReader(std::shared_ptr<IReader> reader, std::shared_ptr<VolumeStorage> volumeStorage);

	virtual bool isSeekable() const;
	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;
	virtual Specs getSpecs() const;
	virtual void read(int& length, bool& eos, sample_t* buffer);
};

AUD_NAMESPACE_END