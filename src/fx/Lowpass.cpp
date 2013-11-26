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

#include "Lowpass.h"
#include "IIRFilterReader.h"
#include "LowpassCalculator.h"

AUD_NAMESPACE_BEGIN

Lowpass::Lowpass(std::shared_ptr<ISound> factory, float frequency,
									   float Q) :
		DynamicIIRFilter(factory, std::shared_ptr<IDynamicIIRFilterCalculator>(new LowpassCalculator(frequency, Q)))
{
}

AUD_NAMESPACE_END
