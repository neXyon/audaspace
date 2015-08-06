#include "fx/ConvolverReader.h"
#include "Exception.h"

#include <cstring>
#include <math.h>

AUD_NAMESPACE_BEGIN

ConvolverReader::ConvolverReader(std::shared_ptr<IReader> reader, std::shared_ptr<IReader> irReader) :
	m_reader(reader), m_irReader(irReader)
{
	int len = irReader->getLength()*AUD_SAMPLE_SIZE(irReader->getSpecs);
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
		AUD_THROW(StateException, "The impulse response cannot be read");
	}

	fftwf_execute(p);
	fftwf_destroy_plan(p);
	fftwf_free(buf);
}

ConvolverReader::~ConvolverReader()
{
	fftw_free(m_irBuffer);
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

void ConvolverReader::read(int& length, bool& eos, sample_t* buffer)
{
	m_reader->read(length, eos, buffer);
	for (int i = 0; i < length * m_reader->getSpecs().channels; i++)
		buffer[i] = buffer[i];
}

AUD_NAMESPACE_END