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

#include "fx/AnimateableTimeStretchPitchScale.h"

#include "fx/AnimateableTimeStretchPitchScaleReader.h"

AUD_NAMESPACE_BEGIN

AnimateableTimeStretchPitchScale::AnimateableTimeStretchPitchScale(std::shared_ptr<ISound> sound, float timeStretch, float pitchScale, StretcherQuality quality,
                                                                   bool preserveFormant) :
    Effect(sound),
    m_quality(quality),
    m_preserveFormant(preserveFormant),
    m_timeStretch(std::make_shared<AnimateableProperty>(1, timeStretch)),
    m_pitchScale(std::make_shared<AnimateableProperty>(1, pitchScale))
{
}

std::shared_ptr<IReader> AnimateableTimeStretchPitchScale::createReader()
{
	return std::make_shared<AnimateableTimeStretchPitchScaleReader>(getReader(), m_timeStretch, m_pitchScale, m_quality, m_preserveFormant);
}

double AnimateableTimeStretchPitchScale::getTimeRatio() const
{
	float timeStretch;
	m_timeStretch->read(0, &timeStretch);
	return timeStretch;
}

double AnimateableTimeStretchPitchScale::getPitchScale() const
{
	float pitchScale;
	m_pitchScale->read(0, &pitchScale);
	return pitchScale;
}

bool AnimateableTimeStretchPitchScale::getPreserveFormant() const
{
	return m_preserveFormant;
}

AnimateableProperty* AnimateableTimeStretchPitchScale::getAnimProperty(AnimateablePropertyType type)
{
	switch(type)
	{
	case AP_TIME_STRETCH:
		return m_timeStretch.get();
	case AP_PITCH_SCALE:
		return m_pitchScale.get();
	default:
		return nullptr;
	}
}

AUD_NAMESPACE_END