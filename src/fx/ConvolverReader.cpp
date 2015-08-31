#include "fx/ConvolverReader.h"
#include "Exception.h"

#include <cstring>
#include <math.h>
#include <algorithm>
#include <iostream>
AUD_NAMESPACE_BEGIN
ConvolverReader::ConvolverReader(std::shared_ptr<IReader> reader, std::shared_ptr<ImpulseResponse> ir, int nConvolutionThreads, int nChannelThreads) :
	m_reader(reader), m_ir(ir), m_eosReader(false), m_eosTail(false), m_nConvolutionThreads(nConvolutionThreads), m_inChannels(reader->getSpecs().channels), m_nChannelThreads(std::min(nChannelThreads, m_inChannels)), m_stopFlag(false), m_barrier(m_nChannelThreads+1)
{
	m_irChannels = m_ir->getNumberOfChannels();

	for (int i = 0; i < m_nChannelThreads; i++)
		m_threads.push_back(std::thread(&ConvolverReader::threadFunction, this, i));

	int irLength = m_ir->getLength();
	if (m_irChannels != 1 && m_irChannels != m_inChannels)
		AUD_THROW(StateException, "The impulse response and the sound must either have the same amount of channels or the impulse response must be mono");

	m_N = FIXED_N;
	m_M = m_L = m_N / 2;
	
	if (m_irChannels > 1)
		for (int i = 0; i < m_inChannels; i++)
			m_convolvers.push_back(std::unique_ptr<Convolver>(new Convolver(ir->getChannel(i), irLength, m_nConvolutionThreads, false)));
	else
		for (int i = 0; i < m_inChannels; i++)
			m_convolvers.push_back(std::unique_ptr<Convolver>(new Convolver(ir->getChannel(0), irLength, m_nConvolutionThreads, false)));

	for (int i = 0; i < m_inChannels; i++)
		m_vecInOut.push_back((sample_t*)std::malloc(m_L*sizeof(sample_t)));
	m_outBuffer = (sample_t*)std::malloc(m_L*m_inChannels*sizeof(sample_t));
	m_outBufLen = m_eOutBufLen = m_outBufferPos = m_L*m_inChannels;

	m_barrier.wait();
}

ConvolverReader::~ConvolverReader()
{
	m_stopFlag = true;
	std::free(m_outBuffer);
	for (int i = 0; i < m_inChannels; i++)
		std::free(m_vecInOut[i]);

	for (int i = 0; i < m_threads.size(); i++)
		if (m_threads[i].joinable())
			m_threads[i].join();
}

bool ConvolverReader::isSeekable() const
{
	return m_reader->isSeekable();
}

void ConvolverReader::seek(int position)
{
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
	return m_reader->getPosition();
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
}

void ConvolverReader::loadBuffer()
{
	m_lastLengthIn = m_L;
	m_reader->read(m_lastLengthIn, m_eosReader, m_outBuffer);
	if (!m_eosReader || m_lastLengthIn>0)
	{
		divideByChannel(m_outBuffer, m_lastLengthIn*m_inChannels);
		m_barrier.wait();
		m_barrier.wait();
		joinByChannel(0, m_lastLengthOut);
		m_eOutBufLen = m_lastLengthOut*m_inChannels;
	}
	else if(!m_eosTail)
	{
		m_lastLengthIn = m_L;

		m_barrier.wait();
		m_barrier.wait();

		joinByChannel(0, m_lastLengthOut);
		m_eOutBufLen = m_lastLengthOut*m_inChannels;
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

void ConvolverReader::threadFunction(int id)
{
	int total = m_inChannels;
	int share = std::ceil((float)total / (float)m_nChannelThreads);
	int start = id*share;
	int end = std::min(start + share, total);
	
	while (!m_stopFlag)
	{
		m_barrier.wait();
		m_barrier.wait();
		int l=m_lastLengthIn;
		for (int i = start; i < end; i++)
			if(!m_eosReader)
				m_convolvers[i]->getNext(m_vecInOut[i], m_vecInOut[i], l, m_eosTail);
			else
				m_convolvers[i]->getNext(nullptr, m_vecInOut[i], l, m_eosTail);
		if (id == 0)
			m_lastLengthOut = l;
	}
}

AUD_NAMESPACE_END
