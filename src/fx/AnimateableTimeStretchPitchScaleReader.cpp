

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

AnimateableTimeStretchPitchScaleReader::AnimateableTimeStretchPitchScaleReader(std::shared_ptr<IReader> reader, float timeRatio, float pitchScale, StretcherQualityOptions quality,
                                                                               bool preserveFormant) :
    EffectReader(reader), m_position(0), m_pitch_scale(1, pitchScale), m_time_stretch(1, timeRatio)
{
	m_reader = std::make_shared<TimeStretchPitchScaleReader>(reader, timeRatio, pitchScale, quality, preserveFormant);
}

void AnimateableTimeStretchPitchScaleReader::read(int& length, bool& eos, sample_t* buffer)
{
	const int step = 1;
	int processed = 0;

	int channels = m_reader->getSpecs().channels;

	while(processed < length)
	{
		int chunk = std::min(step, length - processed);
		float value;

		m_time_stretch.read(m_position + processed, &value);

		m_reader->setTimeRatio(value);
		m_pitch_scale.read(m_position + processed, &value);

		m_reader->setPitchScale(value);

		int samplesRead = chunk;
		m_reader->read(samplesRead, eos, &buffer[processed * channels]);

		if(samplesRead <= 0 || eos)
		{
			break;
		}

		processed += samplesRead;
	}
	m_position += processed;
	length = processed;
}

void AnimateableTimeStretchPitchScaleReader::seek(int position)
{
	m_position = position;
	m_reader->seek(position);
}

int AnimateableTimeStretchPitchScaleReader::getLength() const
{
	return m_reader->getLength();
}

int AnimateableTimeStretchPitchScaleReader::getPosition() const
{
	return m_position;
}

AnimateableProperty* AnimateableTimeStretchPitchScaleReader::getAnimProperty(AnimateablePropertyType type)
{
	switch(type)
	{
	case AP_PITCH_SCALE:
		return &m_pitch_scale;
	case AP_TIME_STRETCH:
		return &m_time_stretch;
	default:
		return nullptr;
	}
}

AUD_NAMESPACE_END
