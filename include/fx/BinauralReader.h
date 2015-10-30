#pragma once

/**
* @file BinauralReader.h
* @ingroup fx
* The BinauralReader class.
*/

#include "IReader.h"
#include "ISound.h"
#include "Convolver.h"
#include "HRTF.h"
#include "Source.h"
#include "util/FFTPlan.h"
#include "util/ThreadPool.h"

#include <memory>
#include <vector>
#include <future>

AUD_NAMESPACE_BEGIN

/**
* This class represents a reader for a sound that can sound different depending on its realtive position with the listener.
*/
class AUD_API BinauralReader : public IReader
{
private:
	/**
	* The reader of the input sound.
	*/
	std::shared_ptr<IReader> m_reader;

	/**
	* The HRTF set.
	*/
	std::shared_ptr<HRTF> m_hrtfs;

	/**
	* A Source object that will be used to change the source position of the sound.
	*/
	std::shared_ptr<Source> m_source;

	/**
	* The FFT size, given by the FFTPlan.
	*/
	int m_N;

	/**
	* The length of the impulse response fragments, m_N/2 will be used.
	*/
	int m_M;

	/**
	* The max length of the input slices, m_N/2 will be used.
	*/
	int m_L;

	/**
	* The array of convolvers that will be used.
	*/
	std::vector<std::unique_ptr<Convolver>> m_convolvers;

	/**
	* The output buffer in which the convolved data will be written and from which the reader will read.
	*/
	sample_t* m_outBuffer;

	/**
	* A vector of buffers (one per channel) on which the audio signal will be separated per channel so it can be convolved.
	*/
	std::vector<sample_t*> m_vecInOut;

	/**
	* A shared ptr to a thread pool.
	*/
	std::shared_ptr<ThreadPool> m_threadPool;

	/**
	* A vector of futures to sync tasks.
	*/
	std::vector<std::future<int>> m_futures;

	// delete copy constructor and operator=
	BinauralReader(const BinauralReader&) = delete;
	BinauralReader& operator=(const BinauralReader&) = delete;

public:
	/**
	* Creates a new convolver reader.
	* \param reader A reader of the input sound to be assigned to this reader. It must have one channel.
	* \param hrtfs A shared pointer to an HRTF object that will be used to get a particular impulse response depending on the source.
	* \param source A shared pointer to a Source object that will be used to change the source position of the sound.
	* \param threadPool A shared pointer to a ThreadPool object with 1 or more threads.
	* \param plan A shared pointer to and FFT plan that will be used for convolution.
	*/
	BinauralReader(std::shared_ptr<IReader> reader, std::shared_ptr<HRTF> hrtfs, std::shared_ptr<Source> source, std::shared_ptr<ThreadPool> threadPool, std::shared_ptr<FFTPlan> plan);
	virtual ~BinauralReader();

	virtual bool isSeekable() const;
	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;
	virtual Specs getSpecs() const;
	virtual void read(int& length, bool& eos, sample_t* buffer);

private:
	/**
	* Joins several buffers (one per channel) into the m_outBuffer.
	* \param start The starting position from which the m_outBuffer will be written.
	* \param len The amout of samples that will be joined.
	*/
	void joinByChannel(int start, int len);

	/**
	* Loads the m_outBuffer with data.
	*/
	void loadBuffer();

	/**
	* The function that the threads will run. It will process a subset of channels.
	* \param id An id number that will determine which subset of channels will be processed.
	* \param input A flag that will indicate if thare is input data.
	*		-If true there is new input data.
	*		-If false there isn't new input data.
	* \return The number of samples obtained.
	*/
	int threadFunction(int id, bool input);
};

AUD_NAMESPACE_END