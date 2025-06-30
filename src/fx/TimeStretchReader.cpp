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

AUD_NAMESPACE_BEGIN

TimeStretchReader::TimeStretchReader(std::shared_ptr<IReader> reader, double timeRatio, double pitchScale, TimeStretchQualityOptions quality) :
    EffectReader(reader),
    m_timeRatio(timeRatio),
    m_pitchScale(pitchScale),
    m_position(0),
    m_length(0),
    m_quality(-1),
    m_finishedReader(false),
    m_input(reader->getSpecs().channels),
    m_processData(reader->getSpecs().channels),
    m_output(reader->getSpecs().channels),
    m_retrieveData(reader->getSpecs().channels)
{
	configure(quality);
}

void TimeStretchReader::read(int& length, bool& eos, sample_t* buffer)
{
	if(length == 0)
		return;

	int samplesize = AUD_SAMPLE_SIZE(m_reader->getSpecs());

	int channels = m_reader->getSpecs().channels;
	int len;
	sample_t* buf;

	int available = m_stretcher->available();
	while(available < length + m_dropAmount && !m_finishedReader)
	{
		// size_t need = m_stretcher->getSamplesRequired();
		// Note for the V3 engine, the needed samples can be 0 sometimes...
		// For now, choose the block size process size to be 1024. It's actually
		len = 1024;

		m_buffer.assureSize(std::max(m_padAmount, len) * samplesize);
		buf = m_buffer.getBuffer();

		if(m_padAmount > 0)
		{
			std::memset(buf, 0, m_padAmount * samplesize);
			m_padAmount = 0;
		}
		else
		{
			m_reader->read(len, m_finishedReader, buf);
		}

		for(int channel = 0; channel < channels; channel++)
		{
			m_input[channel].assureSize(len * sizeof(sample_t));
			sample_t* channelBuf = m_input[channel].getBuffer();
			for(int i = 0; i < len; i++)
			{
				channelBuf[i] = buf[i * channels + channel];
			}
		}

		for(int channel = 0; channel < channels; channel++)
		{
			m_processData[channel] = m_input[channel].getBuffer();
		}

		m_stretcher->process(m_processData.data(), len, m_finishedReader);

		available = m_stretcher->available();
	}

	if(available <= 0)
	{
		length = 0;
		return;
	}

	int readAmt = std::min(length, available);
	length = readAmt;

	for(int channel = 0; channel < channels; channel++)
	{
		m_output[channel].assureSize(std::max(readAmt, m_dropAmount) * sizeof(sample_t));
		m_retrieveData[channel] = m_output[channel].getBuffer();
	}

	if(m_dropAmount > 0)
	{
		m_stretcher->retrieve(m_retrieveData.data(), m_dropAmount);
		m_dropAmount = 0;
	}

	readAmt = std::min(m_stretcher->available(), length);
	size_t frameRetrieved = m_stretcher->retrieve(m_retrieveData.data(), readAmt);

	// Interleave the retrieved data into buffer
	for(int channel = 0; channel < channels; channel++)
	{
		sample_t* outputBuf = m_output[channel].getBuffer();
		for(int i = 0; i < frameRetrieved; i++)
		{
			buffer[i * channels + channel] = outputBuf[i];
		}
	}

	m_length += frameRetrieved;
	m_position += frameRetrieved;

	eos = m_stretcher->available() == -1;
}

double TimeStretchReader::getTimeRatio() const
{
	return m_timeRatio;
}

void TimeStretchReader::setTimeRatio(double timeRatio)
{
	if(timeRatio >= 1.0 / 256.0 && timeRatio <= 256.0 && timeRatio != m_stretcher->getTimeRatio())
	{
		m_timeRatio = timeRatio;
		m_stretcher->setTimeRatio(timeRatio);
		m_padAmount = m_stretcher->getPreferredStartPad();
		m_dropAmount = m_stretcher->getStartDelay();
		m_stretcher->reset();
	}
}

double TimeStretchReader::getPitchScale() const
{
	return m_pitchScale;
}

void TimeStretchReader::setPitchScale(double pitchScale)
{
	if(pitchScale >= 1.0 / 256.0 && pitchScale <= 256.0 && pitchScale != m_stretcher->getPitchScale())
	{
		m_timeRatio = pitchScale;
		m_stretcher->setPitchScale(pitchScale);
		m_padAmount = m_stretcher->getPreferredStartPad();
		m_dropAmount = m_stretcher->getStartDelay();
		m_stretcher->reset();
	}
}

void TimeStretchReader::seek(int position)
{
	m_stretcher->reset();
	m_length = 0;
	m_reader->seek(position);
	m_finishedReader = false;
	m_padAmount = m_stretcher->getPreferredStartPad();
	m_dropAmount = m_stretcher->getStartDelay();
	m_position = position;
}

int TimeStretchReader::getLength() const
{
	return m_length;
}

int TimeStretchReader::getPosition() const
{
	return m_position;
}

TimeStretchReader::~TimeStretchReader()
{
	delete m_stretcher;
}

void TimeStretchReader::configure(TimeStretchQualityOptions quality)
{
	RubberBandStretcher::Options options = RubberBandStretcher::OptionProcessRealTime;

	if(quality == m_quality)
		return;

	if(!m_stretcher)
	{
		delete m_stretcher;
	}

	if(quality & TimeStretchQualityOption::HIGH)
	{
		options |= RubberBandStretcher::OptionEngineFiner;
	}
	else
	{
		options |= RubberBandStretcher::OptionEngineFaster;
	}

	RubberBandStretcher::Option windowOption = RubberBandStretcher::OptionWindowStandard;

	if(quality & TimeStretchQualityOption::CRISP_0)
	{
		options |= RubberBandStretcher::OptionTransientsSmooth;
		options |= RubberBandStretcher::OptionPhaseIndependent;
		options |= RubberBandStretcher::OptionDetectorCompound;
		windowOption = RubberBandStretcher::OptionWindowLong;
	}
	else if(quality & TimeStretchQualityOption::CRISP_1)
	{
		options |= RubberBandStretcher::OptionTransientsCrisp;
		options |= RubberBandStretcher::OptionPhaseIndependent;
		options |= RubberBandStretcher::OptionDetectorSoft;
		windowOption = RubberBandStretcher::OptionWindowLong;
	}
	else if(quality & TimeStretchQualityOption::CRISP_2)
	{
		options |= RubberBandStretcher::OptionTransientsSmooth;
		options |= RubberBandStretcher::OptionPhaseIndependent;
		options |= RubberBandStretcher::OptionDetectorCompound;
	}
	else if(quality & TimeStretchQualityOption::CRISP_3)
	{
		options |= RubberBandStretcher::OptionTransientsSmooth;
		options |= RubberBandStretcher::OptionPhaseLaminar;
		options |= RubberBandStretcher::OptionDetectorCompound;
	}
	else if(quality & TimeStretchQualityOption::CRISP_4)
	{
		options |= RubberBandStretcher::OptionTransientsMixed;
		options |= RubberBandStretcher::OptionPhaseLaminar;
		options |= RubberBandStretcher::OptionDetectorCompound;
	}
	else if(quality & TimeStretchQualityOption::CRISP_5)
	{
		options |= RubberBandStretcher::OptionTransientsCrisp;
		options |= RubberBandStretcher::OptionPhaseLaminar;
		options |= RubberBandStretcher::OptionDetectorCompound;
	}
	else if(quality & TimeStretchQualityOption::CRISP_6)
	{
		options |= RubberBandStretcher::OptionTransientsCrisp;
		options |= RubberBandStretcher::OptionPhaseIndependent;
		options |= RubberBandStretcher::OptionDetectorCompound;
		windowOption = RubberBandStretcher::OptionWindowShort;
	}

	options |= windowOption;

	m_stretcher = new RubberBandStretcher(m_reader->getSpecs().rate, m_reader->getSpecs().channels, options, m_timeRatio, m_pitchScale);
	m_quality = quality;
	m_padAmount = m_stretcher->getPreferredStartPad();
	m_dropAmount = m_stretcher->getStartDelay();
}

AUD_NAMESPACE_END