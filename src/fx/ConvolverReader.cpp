#include "fx/ConvolverReader.h"
#include "Exception.h"

#include <cstring>
#include <math.h>

AUD_NAMESPACE_BEGIN

ConvolverReader::ConvolverReader(std::shared_ptr<IReader> reader, std::shared_ptr<IReader> irReader) :
	m_reader(reader), m_irReader(irReader), m_position(0), m_eosReader(false), m_eosTail(false)
{
	m_irChannels = irReader->getSpecs().channels;
	m_inChannels = reader->getSpecs().channels;
	if (m_irChannels != 1 && m_irChannels != m_inChannels)
		AUD_THROW(StateException, "The impulse response and the sound must either have the same amount of channels or the impulse response must be mono");

	m_M = m_irReader->getLength();
	m_N = pow(2, ceil(log2(m_M + AUD_DEFAULT_BUFFER_SIZE - 1)));
	m_L = AUD_DEFAULT_BUFFER_SIZE;
	
	auto irVector = processImpulseResponse();

	if (m_irChannels > 1)
		for (int i = 0; i < m_inChannels; i++)
			m_convolvers.push_back(std::make_unique<FFTConvolver>(irVector[i], m_M, m_L, m_N));
	else
		for (int i = 0; i < m_inChannels; i++)
			m_convolvers.push_back(std::make_unique<FFTConvolver>(irVector[0], m_M, m_L, m_N));

	for (int i = 0; i < m_inChannels; i++)
		m_vecInOut.push_back((sample_t*)std::malloc(m_L*sizeof(sample_t)));
	m_outBuffer = (sample_t*)std::malloc(m_L*m_inChannels * sizeof(sample_t));
	m_outBufferPos = m_L*m_inChannels;
	m_eOutBufLen = m_L*m_inChannels;
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
		m_convolvers[i]->clearTail();
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

	int l = m_L;

	int bufRest = m_eOutBufLen - m_outBufferPos;
	if (bufRest < length*m_inChannels)
	{
		if (bufRest > 0)
			std::memcpy(buffer, m_outBuffer + m_outBufferPos, bufRest*sizeof(sample_t));
		loadBuffer(0);
		std::memcpy(buffer + bufRest, m_outBuffer, ((length*m_inChannels) - bufRest)*sizeof(sample_t));
		m_outBufferPos = (length*m_inChannels) - bufRest;
	}
	else
	{
		std::memcpy(buffer, m_outBuffer + m_outBufferPos, length*m_inChannels*sizeof(sample_t));
		m_outBufferPos += length*m_inChannels;
	}




	/*for (int i = 0; i < m_M*m_irChannels; i += m_irChannels)
	{
		int k = 0;
		for (int j = 0; j < m_irChannels; j++)
			irBperChannel[j][k] = irBuffer[i + j];
		k++;
	}

	int l = m_L;
	int l2 = m_L;
	int bufRest = m_eOutBufLen - m_outBufferPos;
	if (bufRest < length)
	{
		if(bufRest>0)
			std::memcpy(buffer, m_outBuffer + m_outBufferPos, bufRest);
		if (!m_eosReader)
		{
			m_reader->read(l, m_eosReader, m_inBuffer);
			m_convolver->getNext(m_inBuffer, l);
			if (m_eosReader)
			{
				l2 = m_L - l;
				m_convolver->getTail(l2, m_eosTail, m_outBuffer + l);
				if (m_eosTail)
					m_eOutBufLen = l + l2;
			}
		}
		else
		{
			if (!m_eosTail)
			{
				m_convolver->getTail(l2, m_eosTail, m_outBuffer);
				if (m_eosTail)
					m_eOutBufLen = l2;
			}
		}
		if (length - bufRest <= m_eOutBufLen) 
		{
			std::memcpy(buffer + bufRest, m_outBuffer, length - bufRest);
			m_outBufferPos = length - bufRest;
		}
		else
		{
			std::memcpy(buffer + bufRest, m_outBuffer, m_eOutBufLen);
			m_outBufferPos = m_eOutBufLen;
			length = bufRest;
		}
	}
	else
	{
		std::memcpy(buffer, m_outBuffer + m_outBufferPos, length);
		m_outBufferPos += length;
	}
	if(m_eosTail && m_outBufferPos > m_eOutBufLen)
	{
		eos = true;
	}*/
}

void ConvolverReader::loadBuffer(int ini)
{
	int l = m_L;
	m_reader->read(l, m_eosReader, m_outBuffer + ini);
	divideByChannel(m_outBuffer, l*m_inChannels, m_inChannels);
	for (int i = 0; i < m_inChannels; i++)
		m_convolvers[i]->getNext(m_vecInOut[i], l);
	joinByChannel(l);
	return;
}

std::vector<std::shared_ptr<std::vector<fftwf_complex>>> ConvolverReader::processImpulseResponse()
{
	int channels = m_irReader->getSpecs().channels;
	bool eos = false;
	int length = m_irReader->getLength();
	sample_t* buffer = (sample_t*)std::malloc(length * channels * sizeof(sample_t));
	std::vector<std::shared_ptr<std::vector<fftwf_complex>>> result;
	for (int i = 0; i < channels; i++)
		result.push_back(std::make_shared<std::vector<fftwf_complex>>((m_N / 2) + 1));

	int l = length;
	m_irReader->read(l, eos, buffer);
	//if (!eos || l != length)
	//{
	//	std::free(buffer);
	//	AUD_THROW(StateException, "The impulse response can not be read");
	//}

	void* bufferFFT = fftwf_malloc(((m_N / 2) + 1) * 2 * sizeof(fftwf_complex));
	fftwf_plan p = fftwf_plan_dft_r2c_1d(length, (float*)bufferFFT, (fftwf_complex*)bufferFFT, FFTW_ESTIMATE);
	for (int i = 0; i < channels; i++)
	{
		int k = 0;
		std::memset(bufferFFT, 0, ((m_N / 2) + 1) * 2 * sizeof(fftwf_complex));
		for (int j = i; j < length*channels; j += channels)
		{
			((float*)bufferFFT)[k] = buffer[j];
			k++;
		}
		fftwf_execute(p);
		for (int j = 0; j < (m_N / 2) + 1; j++)
		{
			(*result[i])[j][0] = ((fftwf_complex*)bufferFFT)[j][0];
			(*result[i])[j][1] = ((fftwf_complex*)bufferFFT)[j][1];
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
		for (int j = 0; j < m_irChannels; j++)
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