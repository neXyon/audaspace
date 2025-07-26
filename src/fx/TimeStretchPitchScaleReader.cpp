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

#include "IReader.h"
#include "Exception.h"

#include "util/Buffer.h"

using namespace RubberBand;

AUD_NAMESPACE_BEGIN

TimeStretchPitchScaleReader::TimeStretchPitchScaleReader(std::shared_ptr<IReader> reader, double timeRatio, double pitchScale, StretcherQualityOption quality,
                                                         bool preserveFormant) :
    EffectReader(reader), m_position(0), m_finishedReader(false), m_channelData(reader->getSpecs().channels), m_deinterleaved(reader->getSpecs().channels)
{
	if (pitchScale < 1.0 / 256.0 || pitchScale > 256.0)
		AUD_THROW(StateException, "The pitch scale must be between 1/256 and 256");

	if (timeRatio < 1.0 / 256.0 || timeRatio > 256.0)
		AUD_THROW(StateException, "The time-stretch ratio must be between 1/256 and 256");

	RubberBandStretcher::Options options = RubberBandStretcher::OptionProcessRealTime | RubberBandStretcher::OptionEngineFiner | RubberBandStretcher::OptionChannelsTogether;

	switch(quality)
	{
	case StretcherQualityOption::HIGH:
		options |= RubberBandStretcher::OptionPitchHighQuality;
		break;
	case StretcherQualityOption::FAST:
		options |= RubberBandStretcher::OptionPitchHighSpeed;
		options |= RubberBandStretcher::OptionWindowShort;
		break;
	case StretcherQualityOption::CONSISTENT:
		options |= RubberBandStretcher::OptionPitchHighConsistency;
		break;
	default:
		break;
	}

	options |= preserveFormant ? RubberBandStretcher::OptionFormantPreserved : RubberBandStretcher::OptionFormantShifted;
	m_stretcher = std::make_unique<RubberBandStretcher>(m_reader->getSpecs().rate, m_reader->getSpecs().channels, options, timeRatio, pitchScale);
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

	eos = false;
	while(buf_position < length)
	{
		int len = m_stretcher->getSamplesRequired();
		if(!m_finishedReader && len != 0)
		{
			m_buffer.assureSize(len * samplesize);
			sample_t* buf = m_buffer.getBuffer();

			m_reader->read(len, m_finishedReader, buf);

			// Deinterleave the input reader buffer for processing
			for(int channel = 0; channel < channels; channel++)
			{
				m_deinterleaved[channel].assureSize(len * sizeof(sample_t));
				sample_t* channelBuf = m_deinterleaved[channel].getBuffer();

				for(int i = 0; i < len; i++)
				{
					channelBuf[i] = buf[i * channels + channel];
				}

				m_channelData[channel] = channelBuf;
			}

			m_stretcher->process(m_channelData.data(), len, m_finishedReader);
		}

		int available = m_stretcher->available();
		if(available == -1)
		{
			eos = true;
			break;
		}

		int readAmt = std::min(left, available);
		if(readAmt == 0)
			continue;

		left -= readAmt;

		for(int channel = 0; channel < channels; channel++)
		{
			m_deinterleaved[channel].assureSize(readAmt * sizeof(sample_t));
			m_channelData[channel] = m_deinterleaved[channel].getBuffer();
		}

		m_stretcher->retrieve(m_channelData.data(), readAmt);

		// Interleave the retrieved data into the buffer
		for(int channel = 0; channel < channels; channel++)
		{
			sample_t* outputBuf = m_deinterleaved[channel].getBuffer();
			for(int i = 0; i < readAmt; i++)
			{
				buffer[(buf_position + i) * channels + channel] = outputBuf[i];
			}
		}

		buf_position += readAmt;
	}

	length = buf_position;
	m_position += length;
	eos = m_stretcher->available() == -1;
}

double TimeStretchPitchScaleReader::getTimeRatio() const
{
	return m_stretcher->getTimeRatio();
}

void TimeStretchPitchScaleReader::setTimeRatio(double timeRatio)
{
	if(timeRatio >= 1.0 / 256.0 && timeRatio <= 256.0)
	{
		m_stretcher->setTimeRatio(timeRatio);
	}
}

double TimeStretchPitchScaleReader::getPitchScale() const
{
	return m_stretcher->getPitchScale();
}

void TimeStretchPitchScaleReader::setPitchScale(double pitchScale)
{
	if(pitchScale >= 1.0 / 256.0 && pitchScale <= 256.0)
	{
		m_stretcher->setPitchScale(pitchScale);
	}
}

void TimeStretchPitchScaleReader::seek(int position)
{
	m_reader->seek(int(position / getTimeRatio()));
	m_finishedReader = false;
	m_stretcher->reset();
	m_position = position;
}

int TimeStretchPitchScaleReader::getLength() const
{
	return m_reader->getLength() * getTimeRatio();
}

int TimeStretchPitchScaleReader::getPosition() const
{
	return m_position;
}

AUD_NAMESPACE_END