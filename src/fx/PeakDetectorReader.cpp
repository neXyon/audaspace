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

#include "fx/PeakDetectorReader.h"

#include <cmath>

AUD_NAMESPACE_BEGIN

PeakDetectorReader::PeakDetectorReader(std::shared_ptr<IReader> reader) : EffectReader(reader)
{
}

void PeakDetectorReader::read(int& length, bool& eos, sample_t* buffer)
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
		// Find peak amplitude across all channels for the current frame.
		float peak = 0.0f;
		for(int ch = 0; ch < channels; ch++)
		{
			peak = std::max(peak, std::abs(buffer[i + ch]));
		}

		// Replace each channel's value with the peak value.
		for(int ch = 0; ch < channels; ch++)
		{
			buffer[i + ch] = peak;
		}
	}
}

AUD_NAMESPACE_END
