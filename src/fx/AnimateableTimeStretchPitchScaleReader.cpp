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

#include <chrono>
#include "fx/AnimateableTimeStretchPitchScaleReader.h"

#include "IReader.h"

AUD_NAMESPACE_BEGIN

AnimateableTimeStretchPitchScaleReader::AnimateableTimeStretchPitchScaleReader(std::shared_ptr<IReader> reader, float fps, std::shared_ptr<AnimateableProperty> timeStretch,
                                                                               std::shared_ptr<AnimateableProperty> pitchScale, StretcherQuality quality, bool preserveFormant) :
    TimeStretchPitchScaleReader(reader, timeStretch->readSingle(0), pitchScale->readSingle(0), quality, preserveFormant),
    m_fps(fps),
    m_timeStretch(timeStretch),
    m_pitchScale(pitchScale)
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

    int inputSamplePos = 0;
    double outputSamplePos = 0.0;

    float ratio = 1.0f;
    float lastRatio = 1.0f;

    const double frameDuration = 1.0 / m_fps; 
    const double samplesPerFrame = frameDuration * sampleRate;
    int frameIndex = 0;
		

		// TODO: update the block size or make it adaptive?
		const int blockSize = 4096;



		auto t2_beg = std::chrono::high_resolution_clock::now();
		while(outputSamplePos < position)
		{
			double outputTime = outputSamplePos / sampleRate;
			float frame = outputTime * m_fps;

			ratio = m_timeStretch->readSingle(frame);
			if(ratio <= 0.0f)
				ratio = lastRatio;
			else
				lastRatio = ratio;

			outputSamplePos += blockSize;
			inputSamplePos += static_cast<int>(blockSize / ratio);
		}
		
		auto t2_end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> t2_elapsed = t2_end - t2_beg;

		if (position != 0)
			printf("Sample-based seek (Approach #2): %d (%.3f ms)\n", inputSamplePos, t2_elapsed.count());


		outputSamplePos = 0;
		ratio = 1.0;
		lastRatio = 1.0;
		inputSamplePos = 0;

		auto t1_beg = std::chrono::high_resolution_clock::now();
    while (outputSamplePos < position)
    {
			ratio = m_timeStretch->readFrame(frameIndex);

			if (ratio <= 0.0f)
					ratio = lastRatio;
			else
					lastRatio = ratio;

			outputSamplePos += samplesPerFrame;
			inputSamplePos += static_cast<int>(samplesPerFrame / ratio);

			frameIndex++;
    }
		auto t1_end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> t1_elapsed = t1_end - t1_beg;

		if (position != 0)
			printf("Frame-based seek (Approach #1): %d (%.3f ms)\n", inputSamplePos, t1_elapsed.count());

		


 

    m_reader->seek(inputSamplePos);
    m_finishedReader = false;
    m_stretcher->reset();
    reset();
    m_position = position;
}
AUD_NAMESPACE_END

