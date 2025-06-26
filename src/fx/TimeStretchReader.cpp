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

#include "fx/TimeStretchReader.h"

#include <chrono>
#include <iostream>

#include "IReader.h"

#include "util/Buffer.h"

using namespace RubberBand;

static int read_call_counter = 0;

AUD_NAMESPACE_BEGIN

TimeStretchReader::TimeStretchReader(std::shared_ptr<IReader> reader, double timeRatio, double pitchScale, TimeStretchQualityOptions quality) :
    EffectReader(reader),
    m_timeRatio(timeRatio),
    m_pitchScale(pitchScale),
    m_position(0),
    m_length(0),
    m_options(quality),
    m_stretcher(reader->getSpecs().rate, reader->getSpecs().channels, RubberBandStretcher::OptionProcessRealTime, timeRatio, pitchScale)
{
}

void TimeStretchReader::read(int& length, bool& eos, sample_t* buffer)
{
	read_call_counter++;
	if(length == 0)
		return;

	int samplesize = AUD_SAMPLE_SIZE(m_reader->getSpecs());

	int channels = m_reader->getSpecs().channels;
	int len;
	sample_t* buf;

	int available = m_stretcher.available();
	bool reader_eos = false;
	while(available < length + m_dropAmount && !reader_eos)
	{
		size_t need = m_stretcher.getSamplesRequired();
		if(need == 0)
			break;

		len = need;

		m_buffer.assureSize(std::max(m_padAmount, len) * samplesize);
		buf = m_buffer.getBuffer();

		if(m_padAmount > 0)
		{
			std::memset(buf, 0, m_padAmount * samplesize);
			m_padAmount = 0;
		}
		else
		{
			m_reader->read(len, reader_eos, buf);
		}

		std::vector<std::vector<sample_t>> deInterLeaved(channels, std::vector<sample_t>(len));
		for(int i = 0; i < len; i++)
		{
			for(int channel = 0; channel < channels; channel++)
			{
				deInterLeaved[channel][i] = buf[i * channels + channel];
			}
		}

		std::vector<sample_t*> processData(channels);
		for(int channel = 0; channel < channels; channel++)
		{
			processData[channel] = deInterLeaved[channel].data();
		}

		m_stretcher.process(processData.data(), len, reader_eos);

		available = m_stretcher.available();
	}

	if(available <= 0)
	{
		length = 0;
		return;
	}

	int readAmt = std::min(length, available);
	length = readAmt;

	std::vector<std::vector<sample_t>> output(channels, std::vector<sample_t>(std::max(readAmt, m_dropAmount)));
	std::vector<sample_t*> outputData(channels);

	for(int channel = 0; channel < channels; channel++)
	{
		outputData[channel] = output[channel].data();
	}

	if(available <= m_dropAmount)
	{
		size_t discard = std::min(available, m_dropAmount);
		m_stretcher.retrieve(outputData.data(), discard);
		m_dropAmount -= discard;
		length = 0;
		printf("DROPPED");
		return;
	}

	if(m_dropAmount > 0)
	{
		m_stretcher.retrieve(outputData.data(), m_dropAmount);
		m_dropAmount = 0;
	}

	readAmt = std::min(m_stretcher.available(), length);
	size_t frameRetrieved = m_stretcher.retrieve(outputData.data(), readAmt);

	for(int i = 0; i < frameRetrieved; i++)
	{
		for(int channel = 0; channel < channels; channel++)
		{
			buffer[i * channels + channel] = output[channel][i];
		}
	}

	m_length += frameRetrieved;
	m_position += frameRetrieved;

	eos = m_stretcher.available() == -1;
}

double TimeStretchReader::getTimeRatio() const
{
	return m_timeRatio;
}

void TimeStretchReader::setTimeRatio(double timeRatio)
{
	if(timeRatio != m_stretcher.getTimeRatio())
	{
		m_stretcher.setTimeRatio(timeRatio);
		m_padAmount = m_stretcher.getPreferredStartPad();
		m_dropAmount = m_stretcher.getStartDelay();
		// m_stretcher.reset();
	}
}

double TimeStretchReader::getPitchScale() const
{
	return m_pitchScale;
}

void TimeStretchReader::setPitchScale(double pitchScale)
{
	if(pitchScale != m_stretcher.getPitchScale())
	{
		m_stretcher.setPitchScale(pitchScale);
		m_padAmount = m_stretcher.getPreferredStartPad();
		m_dropAmount = m_stretcher.getStartDelay();
		// m_stretcher.reset();
	}
}

void TimeStretchReader::seek(int position)
{
	m_stretcher.reset();
	m_length = 0;
	m_reader->seek(position);
}

int TimeStretchReader::getLength() const
{
	return m_length;
}

int TimeStretchReader::getPosition() const
{
	return m_position;
}

AUD_NAMESPACE_END