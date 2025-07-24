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

AnimateableTimeStretchPitchScale::AnimateableTimeStretchPitchScale(std::shared_ptr<ISound> sound, double timeRatio, double pitchScale, StretcherQualityOption quality,
                                                                   bool preserveFormant) :
    Effect(sound),
    m_timeRatio(timeRatio),
    m_pitchScale(pitchScale),
    m_quality(quality),
    m_preserveFormant(preserveFormant),
    m_pitch_scale(1, pitchScale),
    m_time_stretch(1, timeRatio)
{
}

std::shared_ptr<IReader> AnimateableTimeStretchPitchScale::createReader()
{
	return std::shared_ptr<IReader>(new AnimateableTimeStretchPitchScaleReader(getReader(), this, m_timeRatio, m_pitchScale, m_quality, m_preserveFormant));
}

double AnimateableTimeStretchPitchScale::getTimeRatio() const
{
	return m_timeRatio;
}

double AnimateableTimeStretchPitchScale::getPitchScale() const
{
	return m_pitchScale;
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
		return &m_time_stretch;
	case AP_PITCH_SCALE:
		return &m_pitch_scale;
	default:
		return nullptr;
	}
}

AUD_NAMESPACE_END