#include "fx/FFTConvolver.h"

#include <cstring>
#include <math.h>
#include <vector>

AUD_NAMESPACE_BEGIN
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

	m_fftPlanR2C = fftwf_plan_dft_r2c_1d(m_N, (float*)m_inBuffer, (fftwf_complex*)m_inBuffer, flag);
	m_fftPlanC2R = fftwf_plan_dft_c2r_1d(m_N, (fftwf_complex*)m_inBuffer, (float*)m_inBuffer, flag);
}

FFTConvolver::~FFTConvolver()
{
	fftwf_free(m_inBuffer);
	fftwf_destroy_plan(m_fftPlanC2R);
	fftwf_destroy_plan(m_fftPlanR2C);
}

void FFTConvolver::getNext(sample_t* buffer, int length)
{
	std::memset(m_inBuffer, 0, m_realBufLen * sizeof(fftwf_complex));
	std::memcpy(m_inBuffer, buffer, length*sizeof(sample_t));

	fftwf_execute(m_fftPlanR2C);
	for (int i = 0; i < m_realBufLen / 2; i++)
	{
		fftwf_complex a, b;
		a[0] = ((fftwf_complex*)m_inBuffer)[i][0];
		a[1] = ((fftwf_complex*)m_inBuffer)[i][1];
		b[0] = (*m_irBuffer)[i][0];
		b[1] = (*m_irBuffer)[i][1];

		((fftwf_complex*)m_inBuffer)[i][0] = ((a[0] * b[0]) - (a[1] * b[1]));
		((fftwf_complex*)m_inBuffer)[i][1] = ((a[0] * b[1]) + (a[1] * b[0]));
	}
	fftwf_execute(m_fftPlanC2R);

	for (int i = 0; i < m_N; i++)
		((float*)m_inBuffer)[i] /= (m_N);

	for (int i = 0; i < m_M - 1; i++)
		((float*)m_inBuffer)[i] += m_tail[i];

	for (int i = 0; i < m_M - 1; i++)
		m_tail[i] = ((float*)m_inBuffer)[i + length];

	std::memcpy(buffer, m_inBuffer, length * sizeof(sample_t));
}

void FFTConvolver::getTail(int& length, bool& eos, sample_t* buffer)
{
	if (length <= 0)
	{
		length = 0;
		return;
	}
	if (m_tailPos + length > m_M - 1)
	{
		length = m_M - 1 - m_tailPos;
		eos = true;
		m_tailPos = m_M - 1;
	}
	else
		m_tailPos += length;
	std::memcpy(buffer, m_tail, length);
}

void FFTConvolver::clearTail()
{
	std::memset(m_tail, 0, m_M - 1);
}
AUD_NAMESPACE_END