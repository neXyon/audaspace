

/*******************************************************************************
 * Copyright 2009-2025 Jörg Müller
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

#include "fx/AnimateableTimeStretchPitchScaleReader.h"

#include "IReader.h"

AUD_NAMESPACE_BEGIN

#include <cstdio>

AnimateableTimeStretchPitchScaleReader::AnimateableTimeStretchPitchScaleReader(std::shared_ptr<IReader> reader, std::shared_ptr<AnimateableProperty> timeStretch,
                                                                               std::shared_ptr<AnimateableProperty> pitchScale, StretcherQualityOption quality,
                                                                               bool preserveFormant) :
    TimeStretchPitchScaleReader(reader, 1.0, 1.0, quality, preserveFormant), m_timeStretch(timeStretch), m_pitchScale(pitchScale)
{
}

void AnimateableTimeStretchPitchScaleReader::read(int& length, bool& eos, sample_t* buffer)
{
	float value;
	int position = getPosition();
	m_timeStretch->read(position, &value);
	setTimeRatio(value);
	m_pitchScale->read(position, &value);
	setPitchScale(value);
	TimeStretchPitchScaleReader::read(length, eos, buffer);
}

AUD_NAMESPACE_END
