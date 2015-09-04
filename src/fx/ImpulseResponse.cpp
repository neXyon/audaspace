#include "fx/ImpulseResponse.h"

#include <algorithm>
#include <cstring>

AUD_NAMESPACE_BEGIN
ImpulseResponse::ImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse) :
	ImpulseResponse(impulseResponse, std::make_shared<FFTPlan>(false))
{
}

ImpulseResponse::ImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse, std::shared_ptr<FFTPlan> plan)
{
	auto reader = impulseResponse->createReader();
	m_length = reader->getLength();
	processImpulseResponse(impulseResponse->createReader(), plan);
}

int ImpulseResponse::getNumberOfChannels()
{
	return m_channels;
}

int ImpulseResponse::getLength()
{
	return m_length;
}

std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ImpulseResponse::getChannel(int n)
{
	return m_processedIR[n];
}

void ImpulseResponse::processImpulseResponse(std::shared_ptr<IReader> reader, std::shared_ptr<FFTPlan> plan)
{
	m_channels = reader->getSpecs().channels;
	int N = plan->getSize();
	bool eos = false;
	int length = reader->getLength();
	sample_t* buffer = (sample_t*)std::malloc(length * m_channels * sizeof(sample_t));
	int numParts = ceil((float)length / (plan->getSize() / 2));

	for (int i = 0; i < m_channels; i++)
	{
		m_processedIR.push_back(std::make_shared<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>>());
		for (int j = 0; j < numParts; j++)
			(*m_processedIR[i]).push_back(std::make_shared<std::vector<fftwf_complex>>((N / 2) + 1));
	}
	length += reader->getSpecs().rate;
	reader->read(length, eos, buffer);


	void* bufferFFT = plan->getBuffer();
	for (int i = 0; i < m_channels; i++)
	{
		int partStart = 0;
		for (int h = 0; h < numParts; h++)
		{
			int k = 0;
			int len = std::min(partStart + ((N / 2)*m_channels), length*m_channels);
			std::memset(bufferFFT, 0, ((N / 2) + 1) * 2 * sizeof(fftwf_complex));
			for (int j = partStart; j < len; j += m_channels)
			{
				((float*)bufferFFT)[k] = buffer[j + i];
				k++;
			}
			plan->FFT(bufferFFT);
			for (int j = 0; j < (N / 2) + 1; j++)
			{
				(*(*m_processedIR[i])[h])[j][0] = ((fftwf_complex*)bufferFFT)[j][0];
				(*(*m_processedIR[i])[h])[j][1] = ((fftwf_complex*)bufferFFT)[j][1];
			}
			partStart += N / 2 * m_channels;
		}
	}
	plan->freeBuffer(bufferFFT);
	std::free(buffer);
}
AUD_NAMESPACE_END
