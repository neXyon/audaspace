#include "fx/ImpulseResponse.h"

#include <algorithm>
#include <cstring>

AUD_NAMESPACE_BEGIN
ImpulseResponse::ImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse)
{
	auto reader = impulseResponse->createReader();
	m_length = reader->getLength();
	processImpulseResponse(impulseResponse->createReader());
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

void ImpulseResponse::processImpulseResponse(std::shared_ptr<IReader> reader)
{
	m_channels = reader->getSpecs().channels;
	bool eos = false;
	int length = reader->getLength();
	sample_t* buffer = (sample_t*)std::malloc(length * m_channels * sizeof(sample_t));
	int numParts = ceil((float)length / (FIXED_N / 2));

	for (int i = 0; i < m_channels; i++)
	{
		m_processedIR.push_back(std::make_shared<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>>());
		for (int j = 0; j < numParts; j++)
			(*m_processedIR[i]).push_back(std::make_shared<std::vector<fftwf_complex>>((FIXED_N / 2) + 1));
	}
	length += reader->getSpecs().rate;
	reader->read(length, eos, buffer);


	void* bufferFFT = fftwf_malloc(((FIXED_N / 2) + 1) * 2 * sizeof(fftwf_complex));
	fftwf_plan p = fftwf_plan_dft_r2c_1d(FIXED_N, (float*)bufferFFT, (fftwf_complex*)bufferFFT, FFTW_ESTIMATE);
	for (int i = 0; i < m_channels; i++)
	{
		int partStart = 0;
		for (int h = 0; h < numParts; h++)
		{
			int k = 0;
			int len = std::min(partStart + ((FIXED_N / 2)*m_channels), length*m_channels);
			std::memset(bufferFFT, 0, ((FIXED_N / 2) + 1) * 2 * sizeof(fftwf_complex));
			for (int j = partStart; j < len; j += m_channels)
			{
				((float*)bufferFFT)[k] = buffer[j + i];
				k++;
			}
			fftwf_execute(p);
			for (int j = 0; j < (FIXED_N / 2) + 1; j++)
			{
				(*(*m_processedIR[i])[h])[j][0] = ((fftwf_complex*)bufferFFT)[j][0];
				(*(*m_processedIR[i])[h])[j][1] = ((fftwf_complex*)bufferFFT)[j][1];
			}
			partStart += FIXED_N / 2 * m_channels;
		}
	}

	fftwf_free(bufferFFT);
	fftwf_destroy_plan(p);
	std::free(buffer);
}
AUD_NAMESPACE_END
