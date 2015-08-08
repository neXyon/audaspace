#include "fx/FFTConvolver.h"

#include <cstring>
#include <math.h>

AUD_NAMESPACE_BEGIN
FFTConvolver::FFTConvolver(sample_t* ir, int M, int L, bool measure) :
	m_M(M), m_L(L), m_tailPos(0)
{
	int flag;
	if (measure)
		flag = FFTW_MEASURE;
	else
		flag = FFTW_ESTIMATE;

	m_tail = (float*)calloc(m_M - 1, sizeof(float*));

	m_N = m_M + m_L - 1;
	m_Npow2 = pow(2, ceil(log2(m_N)));
	m_realBufLen = ((m_Npow2 / 2) + 1) * 2;

	m_inBuffer = fftwf_malloc(m_realBufLen * sizeof(fftwf_complex));
	m_irBuffer = fftwf_malloc(m_realBufLen * sizeof(fftwf_complex));

	fftwf_plan p = fftwf_plan_dft_r2c_1d(m_Npow2, (float*)m_irBuffer, (fftwf_complex*)m_irBuffer, FFTW_ESTIMATE);
	m_fftPlanR2C = fftwf_plan_dft_r2c_1d(m_Npow2, (float*)m_inBuffer, (fftwf_complex*)m_inBuffer, flag);
	m_fftPlanC2R = fftwf_plan_dft_c2r_1d(m_Npow2, (fftwf_complex*)m_inBuffer, (float*)m_inBuffer, flag);

	std::memset(m_irBuffer, 0, m_realBufLen * sizeof(fftwf_complex));
	std::memcpy(m_irBuffer, ir, m_M * sizeof(sample_t));

	fftwf_execute(p);
	fftwf_destroy_plan(p);
}

FFTConvolver::~FFTConvolver()
{
	fftwf_free(m_inBuffer);
	fftwf_free(m_irBuffer);
	fftwf_destroy_plan(m_fftPlanC2R);
	fftwf_destroy_plan(m_fftPlanR2C);
}

void FFTConvolver::getNext(const sample_t* inBuffer, sample_t* outBuffer, int length)
{
	std::memset(m_inBuffer, 0, m_realBufLen * sizeof(fftwf_complex));
	std::memcpy(m_inBuffer, inBuffer, length*sizeof(sample_t));

	fftwf_execute(m_fftPlanR2C);
	for (int i = 0; i < m_realBufLen; i++)
	{
		fftwf_complex a, b;
		a[0] = ((fftwf_complex*)m_inBuffer)[i][0];
		a[1] = ((fftwf_complex*)m_inBuffer)[i][1];
		b[0] = ((fftwf_complex*)m_irBuffer)[i][0];
		b[1] = ((fftwf_complex*)m_irBuffer)[i][1];

		((fftwf_complex*)m_inBuffer)[i][0] = ((a[0] * b[0]) - (a[1] * b[1]));
		((fftwf_complex*)m_inBuffer)[i][1] = (a[0] * b[1]) + (a[1] * b[0]);
	}
	fftwf_execute(m_fftPlanC2R);

	for (int i = 0; i < m_N; i++)
		((float*)m_inBuffer)[i] /= m_N * 8;

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

AUD_NAMESPACE_END