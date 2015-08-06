#include "fx/ConvolverReader.h"
#include "Exception.h"

#include <cstring>
#include <math.h>

AUD_NAMESPACE_BEGIN

ConvolverReader::ConvolverReader(std::shared_ptr<IReader> reader, std::shared_ptr<IReader> irReader) :
	m_reader(reader), m_irReader(irReader), m_lastLength(0), m_buffer(nullptr), m_irBuffer(nullptr)
{
	m_irLength = m_irReader->getLength()*m_irReader->getSpecs().channels;
	m_tail = (sample_t*)calloc(m_irLength - 1, sizeof(sample_t));
	m_finalBuffer = new sample_t[m_reader->getLength()*m_reader->getSpecs().channels];
	m_position = 0;
	convolveAll();
	/*int len = irReader->getLength()*AUD_SAMPLE_SIZE(irReader->getSpecs);
	sample_t* buf = (float*) fftwf_malloc(len);
	m_irBuffer = (fftwf_complex*)fftw_malloc(((len / sizeof(sample_t) / 2) + 1) * sizeof(fftwf_complex));
	fftwf_plan p = fftwf_plan_dft_r2c_1d(pow(2, ceil(log2(len))), buf, m_irBuffer, FFTW_ESTIMATE);

	irReader->seek(0);
	int length = irReader->getLength();
	bool eos = false;
	irReader->read(length, eos, buf);

	if (!eos)
	{
		fftwf_destroy_plan(p);
		fftwf_free(buf);
		fftwf_free(m_irBuffer);
		AUD_THROW(StateException, "The impulse response cannot be read");
	}

	fftwf_execute(p);
	fftwf_destroy_plan(p);
	fftwf_free(buf);*/
}

ConvolverReader::~ConvolverReader()
{
	if (m_irBuffer != nullptr)
		fftwf_free(m_irBuffer);
	if (m_buffer)
		fftwf_free(m_buffer);
}

bool ConvolverReader::isSeekable() const
{
	return m_reader->isSeekable();
}

void ConvolverReader::seek(int position)
{
	m_reader->seek(position);
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

void ConvolverReader::convolveAll()
{
	int length = 2 * m_irLength;
	if (length <= 0)
	{
		length = 0;
		return;
	}
	int pos = 0;
	while (pos+length<m_reader->getLength()*m_reader->getSpecs().channels)
	{
		if (m_lastLength < length)
		{
			m_lastLength = length;
			int len = (length * m_reader->getSpecs().channels) + (m_irReader->getLength() * m_irReader->getSpecs().channels);
			m_n = pow(2, ceil(log2(len)));
			m_bufLen = ((m_n / 2) + 1) * 2;

			m_buffer = (fftwf_complex*)fftwf_malloc(m_bufLen * sizeof(fftwf_complex));
			m_irBuffer = (fftwf_complex*)fftwf_malloc(m_bufLen * sizeof(fftwf_complex));
			fftwf_plan p = fftwf_plan_dft_r2c_1d(m_n, (float*)m_irBuffer, (fftwf_complex*)m_irBuffer, FFTW_ESTIMATE);
			m_fftPlanR2C = fftwf_plan_dft_r2c_1d(m_n, (float*)m_buffer, (fftwf_complex*)m_buffer, FFTW_ESTIMATE);
			m_fftPlanC2R = fftwf_plan_dft_c2r_1d(m_n, (fftwf_complex*)m_buffer, (float*)m_buffer, FFTW_ESTIMATE);

			memset(m_irBuffer, 0, m_bufLen * sizeof(fftwf_complex));
			m_irReader->seek(0);
			int irLength = m_irReader->getLength() + 1;
			bool end = false;
			m_irReader->read(irLength, end, (float*)m_irBuffer);
			m_irReader->seek(0);
			if (!end)
			{
				fftwf_destroy_plan(p);
				AUD_THROW(StateException, "The impulse response cannot be read");
			}
			fftwf_execute(p);
			fftwf_destroy_plan(p);
		}

		bool eos = false;
		memset(m_buffer, 0, m_bufLen * sizeof(fftwf_complex));
		m_reader->read(length, eos, (sample_t*)m_buffer);

		fftwf_execute(m_fftPlanR2C);
		for (int i = 0; i < m_bufLen; i++)
		{
			fftwf_complex a, b;
			a[0] = ((fftwf_complex*)m_buffer)[i][0];
			a[1] = ((fftwf_complex*)m_buffer)[i][1];
			b[0] = ((fftwf_complex*)m_irBuffer)[i][0];
			b[1] = ((fftwf_complex*)m_irBuffer)[i][1];

			((fftwf_complex*)m_buffer)[i][0] = (a[0] * b[0]) - (a[1] * b[1]);
			((fftwf_complex*)m_buffer)[i][1] = (a[0] * b[1]) + (a[1] * b[0]);
		}
		fftwf_execute(m_fftPlanC2R);

		for (int i = 0; i < m_n; i++)
			((float*)m_buffer)[i] = ((float*)m_buffer)[i] / (m_n*20);

		for (int i = 0; i < m_irLength - 1; i++)
		{
			((sample_t*)m_buffer)[i] += m_tail[i];
		}
		for (int i = 0; i < m_irLength - 1; i++)
		{
			if (eos == false)
				m_tail[i] = ((sample_t*)m_buffer)[i + (length * m_reader->getSpecs().channels)];
			else
				m_tail[i] = 0;
		}
		if (eos == true)
			length += (m_irLength - 1);
		std::memcpy(m_finalBuffer+(pos*m_reader->getSpecs().channels), m_buffer, length * AUD_SAMPLE_SIZE(m_reader->getSpecs()));
		pos = pos + length - 1;
	}
}

void ConvolverReader::read(int& length, bool& eos, sample_t* buffer)
{
	eos = false;
	std::memcpy(buffer, m_finalBuffer+(m_position*m_reader->getSpecs().channels), length * AUD_SAMPLE_SIZE(m_reader->getSpecs()));
	m_position += length;

	/*if (length <= 0)
	{
		length = 0;
		return;
	}

	if (m_lastLength < length)
	{
		m_lastLength = length;
		int len = (length * m_reader->getSpecs().channels) + (m_irReader->getLength() * m_irReader->getSpecs().channels) - 1;
		m_n = pow(2, ceil(log2(len)));
		m_bufLen = ((m_n / 2) + 1) * 2;

		m_buffer = (fftwf_complex*)fftwf_malloc(m_bufLen * sizeof(fftwf_complex));
		m_irBuffer = (fftwf_complex*)fftwf_malloc(m_bufLen * sizeof(fftwf_complex));
		fftwf_plan p = fftwf_plan_dft_r2c_1d(m_n, (float*)m_irBuffer, (fftwf_complex*)m_irBuffer, FFTW_ESTIMATE);
		m_fftPlanR2C = fftwf_plan_dft_r2c_1d(m_n, (float*)m_buffer, (fftwf_complex*)m_buffer, FFTW_ESTIMATE);
		m_fftPlanC2R = fftwf_plan_dft_c2r_1d(m_n, (fftwf_complex*)m_buffer, (float*)m_buffer, FFTW_ESTIMATE);

		memset(m_irBuffer, 0, m_bufLen * sizeof(fftwf_complex));
		m_irReader->seek(0);
		int irLength = m_irReader->getLength() + 1;
		bool end = false;
		m_irReader->read(irLength, end, (float*)m_irBuffer);
		m_irReader->seek(0);
		if (!end)
		{
			fftwf_destroy_plan(p);
			AUD_THROW(StateException, "The impulse response cannot be read");
		}
		fftwf_execute(p);
		fftwf_destroy_plan(p);
	}

	memset(m_buffer, 0, m_bufLen * sizeof(fftwf_complex));
	m_reader->read(length, eos, (sample_t*)m_buffer);

	fftwf_execute(m_fftPlanR2C);
	for (int i = 0; i < m_bufLen; i++)
	{
		fftwf_complex a, b;
		a[0] = ((fftwf_complex*)m_buffer)[i][0];
		a[1] = ((fftwf_complex*)m_buffer)[i][1];
		b[0] = ((fftwf_complex*)m_irBuffer)[i][0];
		b[1] = ((fftwf_complex*)m_irBuffer)[i][1];

		((fftwf_complex*)m_buffer)[i][0] = (a[0] * b[0]) - (a[1] * b[1]);
		((fftwf_complex*)m_buffer)[i][1] = (a[0] * b[1]) + (a[1] * b[0]);
	}
	fftwf_execute(m_fftPlanC2R);

	for (int i = 0; i < m_n; i++)
		((float*)m_buffer)[i] = ((float*)m_buffer)[i] / (m_n);

	for (int i = 0; i < m_irLength - 1; i++)
	{
		((sample_t*)m_buffer)[i] += m_tail[i];
	}
	for (int i = 0; i < m_irLength - 1; i++)
	{
		if (eos == false)
			m_tail[i] = ((sample_t*)m_buffer)[i + (length * m_reader->getSpecs().channels)];
		else
			m_tail[i] = 0;
	}
	if (eos == true)
		length += (m_irLength - 1);
	std::memcpy(buffer, m_buffer, length * AUD_SAMPLE_SIZE(m_reader->getSpecs()));*/
}

AUD_NAMESPACE_END