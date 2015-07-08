#pragma once

/**
* @file MutableReader.h
* @ingroup fx
* The MutableReader class.
*/

#include "IReader.h"
#include "ISound.h"

#include <memory>
#include <vector>

AUD_NAMESPACE_BEGIN

/**
* This class represents a reader for a sound that changes with each playback.
*/
class AUD_API MutableReader : public IReader
{
private:
	/**
	* The current reader.
	*/
	std::shared_ptr<IReader> m_reader;

	/**
	* A list of sounds from which to get the reader.
	*/
	std::vector<std::shared_ptr<ISound>> m_sounds;

	/**
	* Flag for random playback
	*/
	bool m_random;

	/**
	* Flag for random playback
	*/
	int m_index;


	// delete copy constructor and operator=
	MutableReader(const MutableReader&) = delete;
	MutableReader& operator=(const MutableReader&) = delete;

public:
	/**
	* Creates a new mutable reader.
	* \param sounds A list of sounds from which to get readers.
	* \param random A flag specifying if the playback y random or sequential. True: random mode, false: sequential mode.
	* \param index The index of the sound that must play first.
	*/
	MutableReader(std::vector<std::shared_ptr<ISound>>& sounds, bool random, int index);

	virtual bool isSeekable() const;
	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;
	virtual Specs getSpecs() const;
	virtual void read(int& length, bool& eos, sample_t* buffer);
};

AUD_NAMESPACE_END
