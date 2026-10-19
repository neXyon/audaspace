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
#include <cstring>

#include "Exception.h"

AUD_NAMESPACE_BEGIN

inline float CompressorReader::linearToDb(float linear)
{
	if(linear <= MINIMUM_AMPLITUDE)
	{
		return 20.0f * std::log10(MINIMUM_AMPLITUDE);
	}
	return 20.0f * std::log10(linear);
}

inline float CompressorReader::dbToLinear(float db)
{
	if(db <= 20.0f * std::log10(MINIMUM_AMPLITUDE))
	{
		return 0.0f;
	}
	return std::pow(10.0f, db / 20.0f);
}

CompressorReader::CompressorReader(std::shared_ptr<IReader> reader, std::shared_ptr<IReader> sidechain_reader, float threshold, float ratio, float attack, float release,
                                   float makeupGain) :
    EffectReader(reader), m_sidechain_reader(sidechain_reader), m_makeupGain(makeupGain)
{
	// Precalculate compression parameters and smoothing factors
	m_thresholdDb = linearToDb(threshold);
	m_compressionSlope = (ratio > 1.0f) ? (1.0f - 1.0f / ratio) : 0.0f;

	SampleRate sampleRate = m_reader->getSpecs().rate;
	m_alphaAttack = (sampleRate > 0 && attack > 0) ? std::exp(-1.0f / (sampleRate * attack)) : 0.0f;
	m_alphaRelease = (sampleRate > 0 && release > 0) ? std::exp(-1.0f / (sampleRate * release)) : 0.0f;
}

void CompressorReader::read(int& length, bool& eos, sample_t* buffer)
{
	Specs specs = m_reader->getSpecs();
	Specs specs2 = m_sidechain_reader->getSpecs();
	if(!AUD_COMPARE_SPECS(specs, specs2))
		AUD_THROW(StateException, "CompressorReader: Both readers must have the same sample rate and channel count.");

	m_reader->read(length, eos, buffer);

	if(length == 0)
	{
		return;
	}

	int samplesize = AUD_SAMPLE_SIZE(specs);
	m_buffer.assureSize(length * samplesize);
	sample_t* buf = m_buffer.getBuffer();

	int len2 = length;
	bool eos2 = false;
	m_sidechain_reader->read(len2, eos2, buf);

	if(len2 < length)
	{
		std::memset(buf + len2 * specs.channels, 0, (length - len2) * samplesize);
	}

	const Channels channels = specs.channels;
	const int totalSamples = length * channels;

	for(int i = 0; i < totalSamples; i += channels)
	{
		const float levelDb = linearToDb(buf[i]);

		// Calculate target gain reduction in decibels
		const float overshootDb = levelDb - m_thresholdDb;
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
	m_sidechain_reader->seek(position);

	m_gainReductionDb = 0.0f;
}

bool CompressorReader::isSeekable() const
{
	return m_reader->isSeekable() && m_sidechain_reader->isSeekable();
}

AUD_NAMESPACE_END
