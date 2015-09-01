#pragma once

/**
* @file ConvolverReader.h
* @ingroup fx
* The ConvolverReader class.
*/

#include "IReader.h"
#include "ISound.h"
#include "Convolver.h"
#include "ImpulseResponse.h"
#include "util/Barrier.h"

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

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
	* The impulse response in the frequency domain.
	*/
	std::shared_ptr<ImpulseResponse> m_ir;
	
	/**
	* The length of the impulse response fragments, FIXED_N/2 will be used.
	*/
	int m_M;

	/**
	* The max length of the input slices, FIXED_N/2 will be used.
	*/
	int m_L;

	/**
	* The FFT size, FIXED_N value will be used.
	*/
	int m_N;

	/**
	* The array of convolvers that will be used, one per channel.
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
	* Current position in which the m_outBuffer is being read.
	*/
	int m_outBufferPos;
	
	/**
	* Effective length of rhe m_outBuffer.
	*/
	int m_eOutBufLen;

	/**
	* Real length of the m_outBuffer.
	*/
	int m_outBufLen;

	/**
	* Flag indicating whether the end of the sound has been reached or not.
	*/
	bool m_eosReader;

	/**
	* Flag indicating whether the end of the extra data generated in the convolution has been reached or not.
	*/
	bool m_eosTail;

	/**
	* The number of channels of the sound to be convolved.
	*/
	int m_inChannels;

	/**
	* The number of channels of the impulse response.
	*/
	int m_irChannels;

	/**
	* The number of threads per channel that will be used for convolution.
	*/
	int m_nConvolutionThreads;

	/**
	* The number of threads used for channels.
	*/
	int m_nChannelThreads;

	/**
	* A vector of thread objects.
	*/
	std::vector<std::thread> m_threads;

	/**
	* Flag used to end the channel threads.
	*/
	std::atomic_bool m_stopFlag;

	/**
	* Length of the input data to be used by the channel threads.
	*/
	int m_lastLengthIn;

	/**
	* Barrier used to sync the channel threads.
	*/
	Barrier m_barrier;

	/**
	* The last channel to be processed by the main thread.
	*/
	int m_end;

	// delete copy constructor and operator=
	ConvolverReader(const ConvolverReader&) = delete;
	ConvolverReader& operator=(const ConvolverReader&) = delete;

public:
	/**
	* Creates a new convolver reader.
	* \param reader A reader of the input sound to be assigned to this reader.
	* \param ir A shared pointer to an impulseResponse object that will be used to convolve the sound.
	* \param nConvolutionThreads The number of threads per channel that can be used for convolution.
	* \param nChannelThreads The number of threads that can be used for channels.
	*/
	ConvolverReader(std::shared_ptr<IReader> reader, std::shared_ptr<ImpulseResponse> ir, int nConvolutionThreads=1, int nChannelThreads = 2);
	virtual ~ConvolverReader();

	virtual bool isSeekable() const;
	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;
	virtual Specs getSpecs() const;
	virtual void read(int& length, bool& eos, sample_t* buffer);

private:
	/**
	* Divides a sound buffer in several buffers, one per channel.
	* \param buffer The buffer that will be divided.
	* \param len The length of the buffer.
	*/
	void divideByChannel(const sample_t* buffer, int len);

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
	* The tunction that the threads will run.
	*/
	void threadFunction(int id);
};

AUD_NAMESPACE_END