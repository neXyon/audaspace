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

#include "fx/TimeStretchPitchScaleReader.h"

#include <iostream>

#include "IReader.h"

#include "util/Buffer.h"

using namespace RubberBand;

AUD_NAMESPACE_BEGIN

TimeStretchPitchScaleReader::TimeStretchPitchScaleReader(std::shared_ptr<IReader> reader, double timeRatio, double pitchScale, StretcherQualityOptions quality,
                                                         bool preserveFormant) :
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
	configure(quality, preserveFormant);
}

void TimeStretchPitchScaleReader::read(int& length, bool& eos, sample_t* buffer)
{
	if(length == 0)
		return;

	int samplesize = AUD_SAMPLE_SIZE(m_reader->getSpecs());
	int channels = m_reader->getSpecs().channels;

	sample_t* buf;

	int buf_position = 0;
	int left = length;

	while(buf_position < length)
	{
		int len = m_stretcher->getSamplesRequired();
		if(!m_finishedReader && len != 0)
		{
			m_buffer.assureSize(len * samplesize);
			sample_t* buf = m_buffer.getBuffer();

			m_reader->read(len, m_finishedReader, buf);
			m_position += len;

			// Deinterleave the input reader buffer for processing
			for(int channel = 0; channel < channels; channel++)
			{
				m_input[channel].assureSize(len * sizeof(sample_t));
				sample_t* channelBuf = m_input[channel].getBuffer();

				for(int i = 0; i < len; i++)
				{
					channelBuf[i] = buf[i * channels + channel];
				}

				m_processData[channel] = channelBuf;
			}

			m_stretcher->process(m_processData.data(), len, m_finishedReader);
		}

		int available = m_stretcher->available();
		if(available == -1)
		{
			eos = true;
			break;
		}

		int readAmt = std::min(left, available);
		if(readAmt == 0)
			break;

		left -= readAmt;

		for(int channel = 0; channel < channels; channel++)
		{
			m_output[channel].assureSize(readAmt * sizeof(sample_t));
			m_retrieveData[channel] = m_output[channel].getBuffer();
		}

		m_stretcher->retrieve(m_retrieveData.data(), readAmt);

		// Interleave the retrieved data into the buffer
		for(int channel = 0; channel < channels; channel++)
		{
			sample_t* outputBuf = m_output[channel].getBuffer();
			for(int i = 0; i < readAmt; i++)
			{
				buffer[(buf_position + i) * channels + channel] = outputBuf[i];
			}
		}

		buf_position += readAmt;
		m_length += readAmt;
	}

	length = buf_position;
}

double TimeStretchPitchScaleReader::getTimeRatio() const
{
	return m_timeRatio;
}

void TimeStretchPitchScaleReader::setTimeRatio(double timeRatio)
{
	if(timeRatio >= 1.0 / 256.0 && timeRatio <= 256.0 && timeRatio != m_stretcher->getTimeRatio())
	{
		m_timeRatio = timeRatio;
		m_stretcher->setTimeRatio(timeRatio);
	}
}

double TimeStretchPitchScaleReader::getPitchScale() const
{
	return m_pitchScale;
}

void TimeStretchPitchScaleReader::setPitchScale(double pitchScale)
{
	if(pitchScale >= 1.0 / 256.0 && pitchScale <= 256.0 && pitchScale != m_stretcher->getPitchScale())
	{
		m_pitchScale = pitchScale;
		m_stretcher->setPitchScale(pitchScale);
	}
}

void TimeStretchPitchScaleReader::seek(int position)
{
	m_length = 0;
	m_reader->seek(position);
	m_finishedReader = false;
	m_stretcher->reset();
	m_position = position;
}

int TimeStretchPitchScaleReader::getLength() const
{
	return m_length;
}

int TimeStretchPitchScaleReader::getPosition() const
{
	return m_position;
}

TimeStretchPitchScaleReader::~TimeStretchPitchScaleReader()
{
	delete m_stretcher;
}

void TimeStretchPitchScaleReader::configure(StretcherQualityOptions quality, bool preserveFormant)
{
	if(quality == m_quality && preserveFormant == m_preserveFormant)
		return;

	if(m_stretcher)
	{
		delete m_stretcher;
	}

	RubberBandStretcher::Options options = RubberBandStretcher::OptionProcessRealTime;

	options |= (quality & StretcherQualityOption::HIGH) ? RubberBandStretcher::OptionEngineFiner : RubberBandStretcher::OptionEngineFaster;

	options |= preserveFormant ? RubberBandStretcher::OptionFormantPreserved : RubberBandStretcher::OptionFormantShifted;

	RubberBandStretcher::Option windowOption = RubberBandStretcher::OptionWindowStandard;

	if(quality & StretcherQualityOption::CRISP_0)
	{
		options |= RubberBandStretcher::OptionTransientsSmooth;
		options |= RubberBandStretcher::OptionPhaseIndependent;
		options |= RubberBandStretcher::OptionDetectorCompound;
		windowOption = RubberBandStretcher::OptionWindowLong;
	}
	else if(quality & StretcherQualityOption::CRISP_1)
	{
		options |= RubberBandStretcher::OptionTransientsCrisp;
		options |= RubberBandStretcher::OptionPhaseIndependent;
		options |= RubberBandStretcher::OptionDetectorSoft;
		windowOption = RubberBandStretcher::OptionWindowLong;
	}
	else if(quality & StretcherQualityOption::CRISP_2)
	{
		options |= RubberBandStretcher::OptionTransientsSmooth;
		options |= RubberBandStretcher::OptionPhaseIndependent;
		options |= RubberBandStretcher::OptionDetectorCompound;
	}
	else if(quality & StretcherQualityOption::CRISP_3)
	{
		options |= RubberBandStretcher::OptionTransientsSmooth;
		options |= RubberBandStretcher::OptionPhaseLaminar;
		options |= RubberBandStretcher::OptionDetectorCompound;
	}
	else if(quality & StretcherQualityOption::CRISP_4)
	{
		options |= RubberBandStretcher::OptionTransientsMixed;
		options |= RubberBandStretcher::OptionPhaseLaminar;
		options |= RubberBandStretcher::OptionDetectorCompound;
	}
	else if(quality & StretcherQualityOption::CRISP_5)
	{
		options |= RubberBandStretcher::OptionTransientsCrisp;
		options |= RubberBandStretcher::OptionPhaseLaminar;
		options |= RubberBandStretcher::OptionDetectorCompound;
	}
	else if(quality & StretcherQualityOption::CRISP_6)
	{
		options |= RubberBandStretcher::OptionTransientsCrisp;
		options |= RubberBandStretcher::OptionPhaseIndependent;
		options |= RubberBandStretcher::OptionDetectorCompound;
		windowOption = RubberBandStretcher::OptionWindowShort;
	}

	options |= windowOption;

	m_stretcher = new RubberBandStretcher(m_reader->getSpecs().rate, m_reader->getSpecs().channels, options, m_timeRatio, m_pitchScale);
	m_quality = quality;
	m_preserveFormant = preserveFormant;
}

AUD_NAMESPACE_END