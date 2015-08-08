#pragma once

/**
* @file ConvolverReader.h
* @ingroup fx
* The ConvolverReader class.
*/

#include "IReader.h"
#include "ISound.h"
#include "FFTConvolver.h"

#include <memory>
#include <vector>
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
	
	int m_L;
	int m_M;

	std::unique_ptr<FFTConvolver> m_convolver;
	std::vector<std::unique_ptr<FFTConvolver>> m_convolvers;

	sample_t* m_inBuffer;
	sample_t* m_outBuffer;

	int m_outBufferPos;
	int m_eOutBufLen;

	int m_position;
	bool m_eosReader;
	bool m_eosTail;

	int m_inChannels;
	int m_irChannels;
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