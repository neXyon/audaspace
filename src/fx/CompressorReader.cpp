/*******************************************************************************
 * Copyright 2026 Mayank
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

#include "fx/CompressorReader.h"

#include <cmath>

namespace
{
	inline float linearToDb(float linear)
	{
		if(linear <= 1e-6f)
		{
			return -120.0f;
		}
		return 20.0f * std::log10(linear);
	}

	inline float dbToLinear(float db)
	{
		if(db <= -120)
		{
			return 0.0f;
		}
		return std::pow(10.0f, db / 20.0f);
	}
} // namespace

AUD_NAMESPACE_BEGIN

CompressorReader::CompressorReader(std::shared_ptr<IReader> reader, float threshold, float ratio, float attack, float release, float makeupGain) :
    EffectReader(reader), m_threshold(threshold), m_ratio(ratio), m_attack(attack), m_release(release), m_makeupGain(makeupGain)
{
	// Precalculate compression parameters and smoothing factors
	m_thresholdDb = linearToDb(m_threshold);
	m_compressionSlope = (m_ratio > 1.0f) ? (1.0f - 1.0f / m_ratio) : 0.0f;

	SampleRate sampleRate = m_reader->getSpecs().rate;
	m_alphaAttack = (sampleRate > 0 && m_attack > 0) ? std::exp(-1.0f / (sampleRate * m_attack)) : 0.0f;
	m_alphaRelease = (sampleRate > 0 && m_release > 0) ? std::exp(-1.0f / (sampleRate * m_release)) : 0.0f;
}

void CompressorReader::read(int& length, bool& eos, sample_t* buffer)
{
	m_reader->read(length, eos, buffer);

	if(length == 0)
	{
		return;
	}

	Specs specs = m_reader->getSpecs();
	const Channels channels = specs.channels;
	const int totalSamples = length * channels;

	for(int i = 0; i < totalSamples; i += channels)
	{
		// Peak detection across all channels
		float peak = 0.0f;
		for(int ch = 0; ch < channels; ch++)
		{
			peak = std::max(peak, std::abs(buffer[i + ch]));
		}

		const float peakDb = linearToDb(peak);

		// Calculate target gain reduction in decibels
		const float overshootDb = peakDb - m_thresholdDb;
		const float targetReductionDb = (overshootDb > 0.0f) ? (overshootDb * m_compressionSlope) : 0.0f;

		// Apply attack or release smoothing
		if(targetReductionDb >= m_gainReductionDb)
		{
			m_gainReductionDb = m_alphaAttack * m_gainReductionDb + (1.0f - m_alphaAttack) * targetReductionDb;
		}
		else
		{
			m_gainReductionDb = m_alphaRelease * m_gainReductionDb + (1.0f - m_alphaRelease) * targetReductionDb;
		}

		// Calculate final gain and apply to all channels.
		const float gain = dbToLinear(-m_gainReductionDb) * m_makeupGain;
		for(int ch = 0; ch < channels; ch++)
		{
			buffer[i + ch] *= gain;
		}
	}
}

void CompressorReader::seek(int position)
{
	m_reader->seek(position);

	m_gainReductionDb = 0.0f;
}

AUD_NAMESPACE_END
