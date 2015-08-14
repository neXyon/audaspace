#include "fx/Convolver.h"

#include <math.h>

AUD_NAMESPACE_BEGIN
Convolver::Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int N, bool measure) :
	m_M(N/2), m_L(N/2), m_N(N), m_tailPos(0)
{

}

Convolver::~Convolver()
{
}

void Convolver::getNext(sample_t* buffer, int length)
{
}

void Convolver::reset()
{
}
AUD_NAMESPACE_END