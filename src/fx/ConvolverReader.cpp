#include "fx/ConvolverReader.h"
#include "Exception.h"

#include <cstring>
#include <algorithm>

AUD_NAMESPACE_BEGIN
ConvolverReader::ConvolverReader(std::shared_ptr<IReader> reader, std::shared_ptr<ImpulseResponse> ir, std::shared_ptr<ThreadPool> threadPool, std::shared_ptr<FFTPlan> plan) :
	m_reader(reader), m_ir(ir), m_N(plan->getSize()), m_eosReader(false), m_eosTail(false), m_inChannels(reader->getSpecs().channels), m_irChannels(ir->getNumberOfChannels()), m_threadPool(threadPool), m_position(0)
{
	m_nChannelThreads = std::min((int)threadPool->getNumOfThreads(), m_inChannels);
	m_futures.resize(m_nChannelThreads);

	int irLength = m_ir->getLength();
	if (m_irChannels != 1 && m_irChannels != m_inChannels)
		AUD_THROW(StateException, "The impulse response and the sound must either have the same amount of channels or the impulse response must be mono");

	m_M = m_L = m_N / 2;
	
	if (m_irChannels > 1)
		for (int i = 0; i < m_inChannels; i++)
			m_convolvers.push_back(std::unique_ptr<Convolver>(new Convolver(ir->getChannel(i), irLength, m_threadPool, plan)));
	else
		for (int i = 0; i < m_inChannels; i++)
			m_convolvers.push_back(std::unique_ptr<Convolver>(new Convolver(ir->getChannel(0), irLength, m_threadPool, plan)));

	for (int i = 0; i < m_inChannels; i++)
		m_vecInOut.push_back((sample_t*)std::malloc(m_L*sizeof(sample_t)));
	m_outBuffer = (sample_t*)std::malloc(m_L*m_inChannels*sizeof(sample_t));
	m_outBufLen = m_eOutBufLen = m_outBufferPos = m_L*m_inChannels;
}

ConvolverReader::~ConvolverReader()
{
	std::free(m_outBuffer);
	for (int i = 0; i < m_inChannels; i++)
		std::free(m_vecInOut[i]);
}

bool ConvolverReader::isSeekable() const
{
	return m_reader->isSeekable();
}

void ConvolverReader::seek(int position)
{
	m_position = position;
	m_reader->seek(position);
	for (int i = 0; i < m_inChannels; i++)
		m_convolvers[i]->reset();
	m_eosTail = false;
	m_eosReader = false;
	m_outBufferPos = m_eOutBufLen = m_outBufLen;
}

int ConvolverReader::getLength() const
{
	return m_reader->getLength() + m_ir->getLength() - 1;
}

int ConvolverReader::getPosition() const
{
	return m_position;
}

Specs ConvolverReader::getSpecs() const
{
	return m_reader->getSpecs();
}

void ConvolverReader::read(int& length, bool& eos, sample_t* buffer)
{
	if (length <= 0)
	{
		length = 0;
		eos = (m_eosTail && m_outBufferPos >= m_eOutBufLen);
		return;
	}
	eos = false;
	int writePos = 0;
	do
	{
		int writeLength = std::min((length*m_inChannels) - writePos, m_eOutBufLen);
		int l = m_L;
		int bufRest = m_eOutBufLen - m_outBufferPos;
		if (bufRest < writeLength || (m_eOutBufLen == 0 && m_eosTail))
		{
			if (bufRest > 0)
				std::memcpy(buffer + writePos, m_outBuffer + m_outBufferPos, bufRest*sizeof(sample_t));
			if (!m_eosTail)
			{
				loadBuffer();
				writeLength = std::min(writeLength, m_eOutBufLen);
				int len = std::min(writeLength, std::abs(writeLength - bufRest));
				std::memcpy(buffer + writePos + bufRest, m_outBuffer, len*sizeof(sample_t));
				m_outBufferPos = len;
			}
			else
			{
				m_outBufferPos += bufRest;
				length = (writePos + bufRest) / m_inChannels;
				eos = true;
				return;
			}
		}
		else
		{
			std::memcpy(buffer + writePos, m_outBuffer + m_outBufferPos, writeLength*sizeof(sample_t));
			m_outBufferPos += writeLength;
		}
		writePos += writeLength;
	} while (writePos < length*m_inChannels);
	m_position += length;
}

void ConvolverReader::loadBuffer()
{
	m_lastLengthIn = m_L;
	m_reader->read(m_lastLengthIn, m_eosReader, m_outBuffer);
	if (!m_eosReader || m_lastLengthIn>0)
	{
		divideByChannel(m_outBuffer, m_lastLengthIn*m_inChannels);
		int len = m_lastLengthIn;

		for (int i = 0; i < m_futures.size(); i++)
			m_futures[i] = m_threadPool->enqueue(&ConvolverReader::threadFunction, this, i, true);
		for (auto &fut : m_futures)
			len = fut.get();

		joinByChannel(0, len);
		m_eOutBufLen = len*m_inChannels;
	}
	else if(!m_eosTail)
	{
		int len = m_lastLengthIn = m_L;
		for (int i = 0; i < m_futures.size(); i++)
			m_futures[i] = m_threadPool->enqueue(&ConvolverReader::threadFunction, this, i, false);
		for (auto &fut : m_futures)
			len = fut.get();

		joinByChannel(0, len);
		m_eOutBufLen = len*m_inChannels;
	}
}

void ConvolverReader::divideByChannel(const sample_t* buffer, int len)
{
	int k = 0;
	for (int i = 0; i < len; i += m_inChannels)
	{	
		for (int j = 0; j < m_inChannels; j++)
			m_vecInOut[j][k] = buffer[i + j];
		k++;
	}
}

void ConvolverReader::joinByChannel(int start, int len)
{
	int k = 0;
	for (int i = 0; i < len*m_inChannels; i += m_inChannels)
	{
		for (int j = 0; j < m_vecInOut.size(); j++)
			m_outBuffer[i + j + start] = m_vecInOut[j][k];
		k++;
	}
}

int ConvolverReader::threadFunction(int id, bool input)
{
	int share = std::ceil((float)m_inChannels / (float)m_nChannelThreads);
	int start = id*share;
	int end = std::min(start + share, m_inChannels);
	
	int l=m_lastLengthIn;
	for (int i = start; i < end; i++)
		if(input)
			m_convolvers[i]->getNext(m_vecInOut[i], m_vecInOut[i], l, m_eosTail);
		else
			m_convolvers[i]->getNext(nullptr, m_vecInOut[i], l, m_eosTail);
	
	return l;
}

AUD_NAMESPACE_END
