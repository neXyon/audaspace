/*******************************************************************************
 * Copyright 2009-2013 Jörg Müller
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#pragma once

/**
 * @file LowpassCalculator.h
 * @ingroup fx
 * The LowpassCalculator class.
 */

#include "fx/IDynamicIIRFilterCalculator.h"

AUD_NAMESPACE_BEGIN

class LowpassCalculator : public IDynamicIIRFilterCalculator
{
private:
	/**
	 * The cutoff frequency.
	 */
	const float m_frequency;

	/**
	 * The Q factor.
	 */
	const float m_Q;

	// delete copy constructor and operator=
	LowpassCalculator(const LowpassCalculator&) = delete;
	LowpassCalculator& operator=(const LowpassCalculator&) = delete;

public:
	LowpassCalculator(float frequency, float Q);

	virtual void recalculateCoefficients(SampleRate rate, std::vector<float> &b, std::vector<float> &a);
};

AUD_NAMESPACE_END
