#include "fx/FFTConvolver.h"

#include <math.h>
#include <cstring>

AUD_NAMESPACE_BEGIN

FFTConvolver::FFTConvolver(std::shared_ptr<std::vector<fftwf_complex>> ir, bool measure) :
	FFTConvolver(ir, FIXED_N / 2, FIXED_N / 2, FIXED_N, measure)
{
}

FFTConvolver::FFTConvolver(std::shared_ptr<std::vector<fftwf_complex>> ir, int M, int L, int N, bool measure) :
	m_M(M), m_L(L), m_N(N), m_tailPos(0), m_irBuffer(ir)
{
	int flag;
	if (measure)
		flag = FFTW_MEASURE;
	else
		flag = FFTW_ESTIMATE;

	m_tail = (float*)calloc(m_M - 1, sizeof(float));
	m_realBufLen = ((m_N / 2) + 1) * 2;
	m_inBuffer = fftwf_malloc(m_realBufLen * sizeof(fftwf_complex));
	m_shiftBuffer = (sample_t*)std::calloc(m_N, sizeof(sample_t));

	m_fftPlanR2C = fftwf_plan_dft_r2c_1d(m_N, (float*)m_inBuffer, (fftwf_complex*)m_inBuffer, flag);
	m_fftPlanC2R = fftwf_plan_dft_c2r_1d(m_N, (fftwf_complex*)m_inBuffer, (float*)m_inBuffer, flag);
}

FFTConvolver::~FFTConvolver()
{
	std::free(m_tail);
	std::free(m_shiftBuffer);
	fftwf_free(m_inBuffer);
	fftwf_destroy_plan(m_fftPlanC2R);
	fftwf_destroy_plan(m_fftPlanR2C);
}

void FFTConvolver::getNext(const sample_t* inBuffer, sample_t* outBuffer, int& length)
{
	if (length > m_L || length <= 0)
	{
		length = 0;
		return;
	}

	std::memset(m_inBuffer, 0, m_realBufLen * sizeof(fftwf_complex));
	std::memcpy(m_inBuffer, inBuffer, length*sizeof(sample_t));

	fftwf_execute(m_fftPlanR2C);
	for (int i = 0; i < m_realBufLen / 2; i++)
	{
		((fftwf_complex*)m_inBuffer)[i][0] = ((((fftwf_complex*)m_inBuffer)[i][0] * (*m_irBuffer)[i][0]) - (((fftwf_complex*)m_inBuffer)[i][1] * (*m_irBuffer)[i][1])) / m_N;
		((fftwf_complex*)m_inBuffer)[i][1] = ((((fftwf_complex*)m_inBuffer)[i][0] * (*m_irBuffer)[i][1]) + (((fftwf_complex*)m_inBuffer)[i][1] * (*m_irBuffer)[i][0])) / m_N;
	}
	fftwf_execute(m_fftPlanC2R);

	for (int i = 0; i < m_M - 1; i++)
		((float*)m_inBuffer)[i] += m_tail[i];

	for (int i = 0; i < m_M - 1; i++)
		m_tail[i] = ((float*)m_inBuffer)[i + length];

	std::memcpy(outBuffer, m_inBuffer, length * sizeof(sample_t));
}

void FFTConvolver::getNext(const sample_t* inBuffer, sample_t* outBuffer, int& length, fftwf_complex* transformedData)
{
	if (length > m_L || length <= 0)
	{
		length = 0;
		return;
	}

	std::memset(m_inBuffer, 0, m_realBufLen * sizeof(fftwf_complex));
	std::memcpy(m_inBuffer, inBuffer, length*sizeof(sample_t));

	fftwf_execute(m_fftPlanR2C);
	std::memcpy(transformedData, m_inBuffer, (m_realBufLen / 2)*sizeof(fftwf_complex));
	for (int i = 0; i < m_realBufLen / 2; i++)
	{
		((fftwf_complex*)m_inBuffer)[i][0] = ((((fftwf_complex*)m_inBuffer)[i][0] * (*m_irBuffer)[i][0]) - (((fftwf_complex*)m_inBuffer)[i][1] * (*m_irBuffer)[i][1])) / m_N;
		((fftwf_complex*)m_inBuffer)[i][1] = ((((fftwf_complex*)m_inBuffer)[i][0] * (*m_irBuffer)[i][1]) + (((fftwf_complex*)m_inBuffer)[i][1] * (*m_irBuffer)[i][0])) / m_N;
	}
	fftwf_execute(m_fftPlanC2R);

	for (int i = 0; i < m_M - 1; i++)
		((float*)m_inBuffer)[i] += m_tail[i];

	for (int i = 0; i < m_M - 1; i++)
		m_tail[i] = ((float*)m_inBuffer)[i + length];

	std::memcpy(outBuffer, m_inBuffer, length * sizeof(sample_t));
}

void FFTConvolver::getNext(const fftwf_complex* inBuffer, sample_t* outBuffer, int& length)
{
	if (length > m_L || length <= 0)
	{
		length = 0;
		return;
	}

	std::memset(m_inBuffer, 0, m_realBufLen * sizeof(fftwf_complex));
	for (int i = 0; i < m_realBufLen / 2; i++)
	{
		((fftwf_complex*)m_inBuffer)[i][0] = ((inBuffer[i][0] * (*m_irBuffer)[i][0]) - (inBuffer[i][1] * (*m_irBuffer)[i][1])) / m_N;
		((fftwf_complex*)m_inBuffer)[i][1] = ((inBuffer[i][0] * (*m_irBuffer)[i][1]) + (inBuffer[i][1] * (*m_irBuffer)[i][0])) / m_N;
	}
	fftwf_execute(m_fftPlanC2R);

	for (int i = 0; i < m_M - 1; i++)
		((float*)m_inBuffer)[i] += m_tail[i];

	for (int i = 0; i < m_M - 1; i++)
		m_tail[i] = ((float*)m_inBuffer)[i + length];

	std::memcpy(outBuffer, m_inBuffer, length * sizeof(sample_t));
}

void FFTConvolver::getTail(int& length, bool& eos, sample_t* buffer)
{
	if (length <= 0)
	{
		length = 0;
		eos = m_tailPos >= m_M - 1;
		return;
	}

	eos = false;
	if (m_tailPos + length > m_M - 1)
	{
		length = m_M - 1 - m_tailPos;
		if (length < 0)
			length = 0;
		eos = true;
		m_tailPos = m_M - 1;
	}
	else
		m_tailPos += length;
	std::memcpy(buffer, m_tail, length*sizeof(sample_t));
}

void FFTConvolver::clear()
{
	std::memset(m_shiftBuffer, 0, m_N * sizeof(sample_t));
	std::memset(m_tail, 0, m_M - 1);
}

void FFTConvolver::IFFT_FDL(const fftwf_complex* inBuffer, sample_t* outBuffer, int& length)
{
	if (length > m_L || length <= 0)
	{
		length = 0;
		return;
	}
	std::memset(m_inBuffer, 0, m_realBufLen * sizeof(fftwf_complex));
	std::memcpy(m_inBuffer, inBuffer, (m_realBufLen / 2)*sizeof(fftwf_complex));
	fftwf_execute(m_fftPlanC2R);
	std::memcpy(outBuffer, ((sample_t*)m_inBuffer)+m_L, length*sizeof(sample_t));
}

void FFTConvolver::getNextFDL(const fftwf_complex* inBuffer, fftwf_complex* accBuffer)
{
	for (int i = 0; i < m_realBufLen / 2; i++)
	{
		accBuffer[i][0] += ((inBuffer[i][0] * (*m_irBuffer)[i][0]) - (inBuffer[i][1] * (*m_irBuffer)[i][1])) / m_N;
		accBuffer[i][1] += ((inBuffer[i][0] * (*m_irBuffer)[i][1]) + (inBuffer[i][1] * (*m_irBuffer)[i][0])) / m_N;
	}
}

void FFTConvolver::getNextFDL(const sample_t* inBuffer, fftwf_complex* accBuffer, int& length, fftwf_complex* transformedData)
{
	if (length > m_L || length <= 0)
	{
		length = 0;
		return;
	}

	std::memcpy(m_shiftBuffer, m_shiftBuffer + m_L, m_L*sizeof(sample_t));
	std::memcpy(m_shiftBuffer + m_L, inBuffer, length*sizeof(sample_t));

	std::memset(m_inBuffer, 0, m_realBufLen * sizeof(fftwf_complex));
	std::memcpy(m_inBuffer, m_shiftBuffer, (m_L+length)*sizeof(sample_t));

	fftwf_execute(m_fftPlanR2C);
	std::memcpy(transformedData, m_inBuffer, (m_realBufLen / 2)*sizeof(fftwf_complex));
	for (int i = 0; i < m_realBufLen / 2; i++)
	{
		accBuffer[i][0] += ((((fftwf_complex*)m_inBuffer)[i][0] * (*m_irBuffer)[i][0]) - (((fftwf_complex*)m_inBuffer)[i][1] * (*m_irBuffer)[i][1])) / m_N;
		accBuffer[i][1] += ((((fftwf_complex*)m_inBuffer)[i][0] * (*m_irBuffer)[i][1]) + (((fftwf_complex*)m_inBuffer)[i][1] * (*m_irBuffer)[i][0])) / m_N;
	}
}
AUD_NAMESPACE_END
