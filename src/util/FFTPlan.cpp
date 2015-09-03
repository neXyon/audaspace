#include "util/FFTPlan.h"

AUD_NAMESPACE_BEGIN
FFTPlan::FFTPlan(bool measure) : 
	FFTPlan(DEFAULT_N, measure)
{
}

FFTPlan::FFTPlan(int n, bool measure) :
	m_N(n), m_bufferSize(((n/2)+1)*2*sizeof(fftwf_complex))
{
	int flag;
	if (measure)
		flag = FFTW_MEASURE;
	else
		flag = FFTW_ESTIMATE;

	void* buf = fftwf_malloc(m_bufferSize);
	m_fftPlanR2C = fftwf_plan_dft_r2c_1d(m_N, (float*)buf, (fftwf_complex*)buf, flag);
	m_fftPlanC2R = fftwf_plan_dft_c2r_1d(m_N, (fftwf_complex*)buf, (float*)buf, flag);
	fftwf_free(buf);
}

FFTPlan::~FFTPlan()
{
	fftwf_destroy_plan(m_fftPlanC2R);
	fftwf_destroy_plan(m_fftPlanR2C);
}

int FFTPlan::getSize()
{
	return m_N;
}

void FFTPlan::FFT(void* buffer)
{
	fftwf_execute_dft_r2c(m_fftPlanR2C, (float*)buffer, (fftwf_complex*)buffer);
}

void FFTPlan::IFFT(void* buffer)
{
	fftwf_execute_dft_c2r(m_fftPlanC2R, (fftwf_complex*)buffer, (float*)buffer);
}

void* FFTPlan::getBuffer()
{
	return fftwf_malloc(m_bufferSize);
}

void FFTPlan::freeBuffer(void* buffer)
{
	fftwf_free(buffer);
}

AUD_NAMESPACE_END