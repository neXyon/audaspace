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

#include "fx/IIRFilterReader.h"

AUD_NAMESPACE_BEGIN

class IDynamicIIRFilterCalculator;

/**
 * This class is for dynamic infinite impulse response filters with simple
 * coefficients that change depending on the sample rate.
 */
class DynamicIIRFilterReader : public IIRFilterReader
{
private:
	/**
	 * The sound for dynamically recalculating filter coefficients.
	 */
	std::shared_ptr<IDynamicIIRFilterCalculator> m_calculator;

public:
	DynamicIIRFilterReader(std::shared_ptr<IReader> reader,
							   std::shared_ptr<IDynamicIIRFilterCalculator> calculator);

	virtual void sampleRateChanged(SampleRate rate);
};

AUD_NAMESPACE_END
