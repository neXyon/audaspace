#pragma once

/**
* @file ConvolverReader.h
* @ingroup fx
* The ConvolverReader class.
*/

#include "IReader.h"
#include "ISound.h"
#include "fftw3.h"

#include <memory>
AUD_NAMESPACE_BEGIN

/**
* This class represents a reader for a sound that has its own shared volume
*/
class AUD_API ConvolverReader : public IReader
{
private:
	/**
	* The reader of the input sound.
	*/
	std::shared_ptr<IReader> m_reader;

	/**
	* The reader of the impulse response sound.
	*/
	std::shared_ptr<IReader> m_irReader;

	/**
	* The reader of the impulse response sound.
	*/
	void* m_irBuffer;

	void* m_buffer;

	int m_bufLen;

	int m_n;

	sample_t* m_tail;
	
	int m_irLength;

	int m_lastLength;

	fftwf_plan m_fftPlanR2C;

	fftwf_plan m_fftPlanC2R;

	sample_t* m_finalBuffer;

	int m_position;
	// delete copy constructor and operator=
	ConvolverReader(const ConvolverReader&) = delete;
	ConvolverReader& operator=(const ConvolverReader&) = delete;

public:
	/**
	* Creates a new convolver reader.
	* \param reader A reader of the input sound to be assigned to this reader.
	* \param imputResponseReader A reader of the impulse response sound.
	*/
	ConvolverReader(std::shared_ptr<IReader> reader, std::shared_ptr<IReader> irReader);
	virtual ~ConvolverReader();

	virtual bool isSeekable() const;
	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;
	virtual Specs getSpecs() const;
	virtual void read(int& length, bool& eos, sample_t* buffer);

private:
	void convolveAll();
};

AUD_NAMESPACE_END