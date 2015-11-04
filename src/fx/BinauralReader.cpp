#include "fx/BinauralReader.h"
#include "Exception.h"

#include <cstring>
#include <algorithm>
#include <iostream>

#define NUM_OUTCHANNELS 2
#define NUM_CONVOLVERS 4

AUD_NAMESPACE_BEGIN
BinauralReader::BinauralReader(std::shared_ptr<IReader> reader, std::shared_ptr<HRTF> hrtfs, std::shared_ptr<Source> source, std::shared_ptr<ThreadPool> threadPool, std::shared_ptr<FFTPlan> plan) :
	m_reader(reader), m_hrtfs(hrtfs), m_source(source), m_N(plan->getSize()), m_threadPool(threadPool), m_position(0), m_eosReader(false), m_eosTail(false), m_transition(false)
{
	if(m_reader->getSpecs().channels!=1)
		AUD_THROW(StateException, "The sound must have only one channel");
	m_M = m_L = m_N / 2;

	m_RealAzimuth = m_Azimuth = m_source->getAzimuth();
	m_RealElevation = m_Elevation = m_source->getElevation();
	auto ir = m_hrtfs->getImpulseResponse(m_RealAzimuth, m_RealElevation);
	int irLength = ir->getLength();
	for (int i = 0; i < NUM_CONVOLVERS; i++)
		m_convolvers.push_back(std::unique_ptr<Convolver>(new Convolver(ir->getChannel(i % 2), irLength, m_threadPool, plan)));
	m_futures.resize(NUM_CONVOLVERS);

	m_outBuffer = (sample_t*)std::malloc(m_L*NUM_OUTCHANNELS*sizeof(sample_t));
	m_transBuffer = (sample_t*)std::malloc(m_L*NUM_OUTCHANNELS*sizeof(sample_t));
	m_eOutBufLen = m_outBufLen = m_outBufferPos = m_L * NUM_OUTCHANNELS;
	m_inBuffer = (sample_t*)std::malloc(m_L * sizeof(sample_t));
	for (int i = 0; i < NUM_CONVOLVERS; i++)
		m_vecOut.push_back((sample_t*)std::malloc(m_L * sizeof(sample_t)));
}

BinauralReader::~BinauralReader()
{
	std::free(m_outBuffer);
	std::free(m_transBuffer);
	std::free(m_inBuffer);
	for (int i = 0; i < m_vecOut.size(); i++)
		std::free(m_vecOut[i]);
}

bool BinauralReader::isSeekable() const
{
	return m_reader->isSeekable();
}

void BinauralReader::seek(int position)
{
	m_position = position;
	m_reader->seek(position);
	for (int i = 0; i < NUM_CONVOLVERS; i++)
		m_convolvers[i]->reset();
	m_eosTail = false;
	m_eosReader = false;
	m_outBufferPos = m_eOutBufLen = m_outBufLen;
}

int BinauralReader::getLength() const
{
	return m_reader->getLength();
}

int BinauralReader::getPosition() const
{
	return m_position;
}

Specs BinauralReader::getSpecs() const
{
	Specs spcs = m_reader->getSpecs();
	spcs.channels = CHANNELS_STEREO;
	return spcs;
}

void BinauralReader::read(int& length, bool& eos, sample_t* buffer)
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
		int writeLength = std::min((length*NUM_OUTCHANNELS) - writePos, m_eOutBufLen);
		int l = m_L;
		int bufRest = m_eOutBufLen - m_outBufferPos;
		if (bufRest < writeLength || (m_eOutBufLen == 0 && m_eosTail))
		{
			if (bufRest > 0)
				writeOutput(buffer + writePos, m_outBufferPos, bufRest);
			if (!m_eosTail)
			{
				int n = NUM_OUTCHANNELS;
				if (m_transition)
					n = NUM_CONVOLVERS;
				else
					if (m_transition = checkSource())
						n = NUM_CONVOLVERS;
				loadBuffer(n);
				writeLength = std::min(writeLength, m_eOutBufLen);
				int len = std::min(writeLength, std::abs(writeLength - bufRest));
				writeOutput(buffer + writePos + bufRest, 0, len);
				m_outBufferPos = len;
			}
			else
			{
				m_outBufferPos += bufRest;
				length = (writePos + bufRest) / NUM_OUTCHANNELS;
				eos = true;
				return;
			}
		}
		else
		{
			writeOutput(buffer + writePos, m_outBufferPos, writeLength);
			m_outBufferPos += writeLength;
		}
		writePos += writeLength;
	} while (writePos < length*NUM_OUTCHANNELS);
	m_position += length;
}

bool BinauralReader::checkSource()
{
	std::shared_ptr<ImpulseResponse> ir;
	if (m_Azimuth != m_source->getAzimuth() || m_Elevation != m_source->getElevation())
	{
		float az = m_Azimuth = m_source->getAzimuth();
		float el = m_Elevation = m_source->getElevation();
		ir = m_hrtfs->getImpulseResponse(az, el);
		if (az != m_RealAzimuth || el != m_RealElevation)
		{	
			m_RealAzimuth = az;
			m_RealElevation = el;
			for (int i = 0; i < NUM_OUTCHANNELS; i++)
			{
				auto temp = std::move(m_convolvers[i]);
				m_convolvers[i] = std::move(m_convolvers[i + NUM_OUTCHANNELS]);
				m_convolvers[i + NUM_OUTCHANNELS] = std::move(temp);
			}
			for (int i = 0; i < NUM_OUTCHANNELS; i++)
				m_convolvers[i]->setImpulseResponse(ir->getChannel(i));

			/*int pos = m_reader->getPosition() - ((m_eOutBufLen - m_outBufferPos) / NUM_OUTCHANNELS);
			if (pos>=0)
				m_reader->seek(m_reader->getPosition() - ((m_eOutBufLen - m_outBufferPos) / NUM_OUTCHANNELS));
			m_outBufferPos = m_eOutBufLen = m_outBufLen;*/
			return true;
		}
	}
	return false;
}

void BinauralReader::writeOutput(sample_t* target, int pos, int len)
{
	const int l = 512;
	float volume = 1.0f;
	SampleRate rate = m_reader->getSpecs().rate;
	static int position = 0;

	if (m_transition)
	{
		for (int i = 0; i < len; i++)
		{
			if (i % NUM_OUTCHANNELS == 0)
			{
				volume = (position + i) / (float)l;
				if (volume > 1.0f)
					volume = 1.0f;
				else if (volume < 0.0f)
					volume = 0.0f;
			}

			//std::memcpy(target, m_outBuffer + pos, len*sizeof(sample_t));
			target[i] = (m_outBuffer[i + pos] * (volume)) + (m_transBuffer[i + pos] * (1.0f - volume));
			float a = target[i];
		}
		position += len;
		if (position >= l)
		{
			m_transition = false;
			position = 0;
		}
	}
	else
		std::memcpy(target, m_outBuffer + pos, len*sizeof(sample_t));
}

void BinauralReader::loadBuffer(int nConvolvers)
{
	m_lastLengthIn = m_L;
	m_reader->read(m_lastLengthIn, m_eosReader, m_inBuffer);
	if (!m_eosReader || m_lastLengthIn > 0)
	{
		int len = m_lastLengthIn;
		for (int i = 0; i < nConvolvers; i++)
			m_futures[i] = m_threadPool->enqueue(&BinauralReader::threadFunction, this, i, true);
		for (int i = 0; i < nConvolvers; i++)
			len = m_futures[i].get();

		joinByChannel(0, len, nConvolvers);
		m_eOutBufLen = len*NUM_OUTCHANNELS;
	}
	else if (!m_eosTail)
	{
		int len = m_lastLengthIn = m_L;
		for (int i = 0; i < nConvolvers; i++)
			m_futures[i] = m_threadPool->enqueue(&BinauralReader::threadFunction, this, i, false);
		for (int i = 0; i < nConvolvers; i++)
			len = m_futures[i].get();

		joinByChannel(0, len, nConvolvers);
		m_eOutBufLen = len*NUM_OUTCHANNELS;
	}
}

void BinauralReader::joinByChannel(int start, int len, int nConvolvers)
{
	int k = 0;
	for (int i = 0; i < len*NUM_OUTCHANNELS; i += NUM_OUTCHANNELS)
	{
		for (int j = 0; j < NUM_OUTCHANNELS; j++)
		{
			m_outBuffer[i + j + start] = m_vecOut[j][k];
			if (nConvolvers == NUM_CONVOLVERS)
				m_transBuffer[i + j + start] = m_vecOut[j+NUM_OUTCHANNELS][k];
		}
		k++;
	}

	/*int k = 0;
	if (nConvolvers == 4)
	{
		float step = 1.0f / len;
		for (int i = 0; i < len*NUM_OUTCHANNELS; i += NUM_OUTCHANNELS)
		{
			for (int j = 0; j < NUM_OUTCHANNELS; j++)
				m_outBuffer[i + j + start] = (m_vecOut[j][k] * (step*k)) + (m_vecOut[j + NUM_OUTCHANNELS][k] * (1 - (step*k)));
			if (step*k>=1)
			std::cout << step*k;
			k++;
		}
	}
	else
	{
		int k = 0;
		for (int i = 0; i < len*NUM_OUTCHANNELS; i += NUM_OUTCHANNELS)
		{
			for (int j = 0; j < NUM_OUTCHANNELS; j++)
				m_outBuffer[i + j + start] = m_vecOut[j][k];
			k++;
		}
	}*/
}

int BinauralReader::threadFunction(int id, bool input)
{
	int l = m_lastLengthIn;
	if (input)
		m_convolvers[id]->getNext(m_inBuffer, m_vecOut[id], l, m_eosTail);
	else
		m_convolvers[id]->getNext(nullptr, m_vecOut[id], l, m_eosTail);
	return l;
}

AUD_NAMESPACE_END