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

AnimateableTimeStretchPitchScaleReader::AnimateableTimeStretchPitchScaleReader(std::shared_ptr<IReader> reader, std::shared_ptr<AnimateableProperty> timeStretch,
                                                                               std::shared_ptr<AnimateableProperty> pitchScale, StretcherQuality quality, bool preserveFormant,
                                                                               float fps) :
    TimeStretchPitchScaleReader(reader, timeStretch->readSingle(0), pitchScale->readSingle(0), quality, preserveFormant),
    m_timeStretch(timeStretch),
    m_pitchScale(pitchScale),
    m_fps(fps)
{
}

void AnimateableTimeStretchPitchScaleReader::read(int& length, bool& eos, sample_t* buffer)
{
	int position = getPosition();

	double time = double(position) / double(m_reader->getSpecs().rate);
	float frame = time * m_fps;

	float timeRatio = m_timeStretch->readSingle(frame);
	setTimeRatio(timeRatio);
	float pitchScale = m_pitchScale->readSingle(frame);
	setPitchScale(pitchScale);
	TimeStretchPitchScaleReader::read(length, eos, buffer);
}

void AnimateableTimeStretchPitchScaleReader::seek(int position)
{
	double sampleRate = double(m_reader->getSpecs().rate);

	double time = double(position) / sampleRate;
	float frame = time * m_fps;

	float timeRatio = m_timeStretch->readSingle(frame);
	setTimeRatio(timeRatio);
	float pitchScale = m_pitchScale->readSingle(frame);
	setPitchScale(pitchScale);

	int inputSampleIndex = 0;
	double outputSamplePos = 0.0;

	float ratio = 1.0f;
	float lastRatio = 1.0f;

	// TODO: Change block size based the read length? Or set it to just one
	int blockSize = 1024;

	while(outputSamplePos < position)
	{
		double outputTime = outputSamplePos / sampleRate;
		float frame = outputTime * m_fps;

		ratio = m_timeStretch->readSingle(frame);
		if(ratio <= 0.0f)
			ratio = lastRatio;
		else
			lastRatio = ratio;

		outputSamplePos += ratio * blockSize;
		inputSampleIndex += blockSize;
	}

	m_reader->seek(inputSampleIndex);
	m_finishedReader = false;
	m_stretcher->reset();
	reset();
	m_position = position;
}

AUD_NAMESPACE_END
