#include "fx/ConvolverReader.h"
#include "Exception.h"

#include <cstring>
#include <math.h>
#include <algorithm>

#define N 16384
AUD_NAMESPACE_BEGIN

ConvolverReader::ConvolverReader(std::shared_ptr<IReader> reader, std::shared_ptr<IReader> irReader) :
	m_reader(reader), m_irReader(irReader), m_position(0), m_eosReader(false), m_eosTail(false)
{
	m_irChannels = irReader->getSpecs().channels;
	m_inChannels = reader->getSpecs().channels;
	int irLength = m_irReader->getLength();
	if (m_irChannels != 1 && m_irChannels != m_inChannels)
		AUD_THROW(StateException, "The impulse response and the sound must either have the same amount of channels or the impulse response must be mono");

	m_N = N;
	m_M = m_L = N / 2;
	
	auto irVector = processFilter();

	if (m_irChannels > 1)
		for (int i = 0; i < m_inChannels; i++)
			m_convolvers.push_back(std::make_unique<Convolver>(irVector[i], m_N, irLength, false));
	else
		for (int i = 0; i < m_inChannels; i++)
			m_convolvers.push_back(std::make_unique<Convolver>(irVector[0], m_N, irLength, false));

	for (int i = 0; i < m_inChannels; i++)
		m_vecInOut.push_back((sample_t*)std::malloc(m_L*sizeof(sample_t)));
	m_outBuffer = (sample_t*)std::malloc(m_L*m_inChannels * sizeof(sample_t));
	m_outBufLen = m_eOutBufLen = m_outBufferPos = m_L*m_inChannels;
}

ConvolverReader::~ConvolverReader()
{
	delete m_outBuffer;
	for (int i = 0; i < m_inChannels; i++)
		std::free(m_vecInOut[i]);
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
}

int ConvolverReader::getLength() const
{
	return m_reader->getLength();
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
		return;
	}
	
	int writePos = 0;
	do
	{
		int writeLength = std::min((length*m_inChannels) - writePos, m_outBufLen);
		int l = m_L;
		int bufRest = m_eOutBufLen - m_outBufferPos;
		if (bufRest < writeLength)
		{
			if (bufRest > 0)
				std::memcpy(buffer + writePos, m_outBuffer + m_outBufferPos, bufRest*sizeof(sample_t));
			loadBuffer(0);
			std::memcpy(buffer + writePos + bufRest, m_outBuffer, (writeLength - bufRest)*sizeof(sample_t));
			m_outBufferPos = writeLength - bufRest;
		}
		else
		{
			std::memcpy(buffer + writePos, m_outBuffer + m_outBufferPos, writeLength*sizeof(sample_t));
			m_outBufferPos += writeLength;
		}
		writePos += writeLength;
	} while (writePos < length*m_inChannels);
}

void ConvolverReader::loadBuffer(int ini)
{
	int l = m_L;
	m_reader->read(l, m_eosReader, m_outBuffer);
	divideByChannel(m_outBuffer, l*m_inChannels, m_inChannels);
	for (int i = 0; i < m_inChannels; i++)
		m_convolvers[i]->getNext(m_vecInOut[i], l);
	joinByChannel(l);
	return;
}

std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>>> ConvolverReader::processFilter()
{
	int channels = m_irReader->getSpecs().channels;
	//int channels = 2;
	bool eos = false;
	int length = m_irReader->getLength();
	//int length = 16;
	sample_t* buffer = (sample_t*)std::malloc(length * channels * sizeof(sample_t));
	//sample_t* buffer = new sample_t[32]{1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,16,16};
	std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>>> result;
	int numParts = ceil((float)length / (N / 2));
	for (int i = 0; i < channels; i++)
	{
		result.push_back(std::make_shared<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>>());
		for (int j = 0; j < numParts; j++)
			(*result[i]).push_back(std::make_shared<std::vector<fftwf_complex>>((N / 2) + 1));
	}
	int l = length;
	m_irReader->read(l, eos, buffer);
	/*Specs specs = m_irReader->getSpecs();
	if (!eos || l != length)
	{
		std::free(buffer);
		AUD_THROW(StateException, "The impulse response can not be read");
	}*/

	void* bufferFFT = fftwf_malloc(((N / 2) + 1) * 2 * sizeof(fftwf_complex));
	fftwf_plan p = fftwf_plan_dft_r2c_1d(N, (float*)bufferFFT, (fftwf_complex*)bufferFFT, FFTW_ESTIMATE);
	for (int i = 0; i < channels; i++)
	{
		int partStart = 0;
		for (int h = 0; h < numParts; h++) 
		{
			int k = 0;
			int len = std::min(partStart + ((N / 2)*channels), length*channels);
			std::memset(bufferFFT, 0, ((N / 2) + 1) * 2 * sizeof(fftwf_complex));
			for (int j = partStart; j < len; j += channels)
			{
				((float*)bufferFFT)[k] = buffer[j + i];
				k++;
			}
			fftwf_execute(p);
			for (int j = 0; j < (N / 2) + 1; j++)
			{
				(*(*result[i])[h])[j][0] = ((fftwf_complex*)bufferFFT)[j][0];
				(*(*result[i])[h])[j][1] = ((fftwf_complex*)bufferFFT)[j][1];
			}
			partStart += N/2*channels;
		}	
	}

	fftwf_free(bufferFFT);
	fftwf_destroy_plan(p);
	std::free(buffer);

	return result;
}

void ConvolverReader::divideByChannel(sample_t* buffer, int len, int channels)
{
	int k = 0;
	for (int i = 0; i < len; i += channels)
	{	
		for (int j = 0; j < channels; j++)
			m_vecInOut[j][k] = buffer[i + j];
		k++;
	}
}

void ConvolverReader::joinByChannel(int len)
{
	int k = 0;
	for (int i = 0; i < len*m_inChannels; i += m_inChannels)
	{
		for (int j = 0; j < m_vecInOut.size(); j++)
			m_outBuffer[i + j] = m_vecInOut[j][k];
		k++;
	}
}

AUD_NAMESPACE_END