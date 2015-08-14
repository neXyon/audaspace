#include "fx/Convolver.h"

#include <cstring>
#include <math.h>
#include <vector>

AUD_NAMESPACE_BEGIN
Convolver::Convolver(sample_t* ir, int N, bool measure) :
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