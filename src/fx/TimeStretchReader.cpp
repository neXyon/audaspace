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

AUD_NAMESPACE_BEGIN

TimeStretchReader::TimeStretchReader(std::shared_ptr<IReader> reader, double time_ratio, TimeStretchQuality quality) :
    EffectReader(reader),
    m_timeRatio(time_ratio),
    m_position(0),
    m_length(0),
    m_stretcher(reader->getSpecs().rate, reader->getSpecs().channels,
                (RubberBandStretcher::OptionWindowStandard | RubberBandStretcher::OptionProcessOffline | RubberBandStretcher::OptionThreadingAuto |
                 (quality == TimeStretchQuality::FASTEST ? RubberBandStretcher::OptionPitchHighSpeed | RubberBandStretcher::OptionEngineFaster :
                                                           RubberBandStretcher::OptionPitchHighQuality | RubberBandStretcher::OptionEngineFiner)),
                time_ratio)
{
	study();
}

void TimeStretchReader::study()
{
	Specs specs = m_reader->getSpecs();
	int buffersize = AUD_DEFAULT_BUFFER_SIZE;

	int samplesize = AUD_SAMPLE_SIZE(specs);

	Buffer buffer(buffersize * samplesize);
	sample_t* buf = buffer.getBuffer();

	int channels = specs.channels;
	int length = m_reader->getLength();

	std::vector<std::vector<sample_t>> deinterleaveBlock(channels, std::vector<sample_t>(buffersize));
	std::vector<const sample_t*> studyInput(channels);

	int len = buffersize;
	bool eos = false;

	for(unsigned int pos = 0; ((pos < length) || (length <= 0)) && !eos;)
	{
		len = buffersize;
		if((len > length - pos) && (length > 0))
			len = length - pos;
		m_reader->read(len, eos, buf);

		for(int i = 0; i < len * channels; i++)
		{
			// clamping!
			if(buf[i] > 1)
				buf[i] = 1;
			else if(buf[i] < -1)
				buf[i] = -1;
		}

		// De-interleave the buffer
		for(int channel = 0; channel < channels; channel++)
		{
			for(int i = 0; i < len; ++i)
			{
				deinterleaveBlock[channel][i] = buf[i * channels + channel];
			}
			studyInput[channel] = deinterleaveBlock[channel].data();
		}
		m_stretcher.study(studyInput.data(), len, eos);
		pos += len;
	}
	m_reader->seek(0);
}

void TimeStretchReader::read(int& length, bool& eos, sample_t* buffer)
{
	if(length == 0)
		return;

	int samplesize = AUD_SAMPLE_SIZE(m_reader->getSpecs());

	int channels = m_reader->getSpecs().channels;
	int len;
	sample_t* buf;

	int available = m_stretcher.available();
	bool reader_eos = false;
	while(available < length && !reader_eos)
	{
		size_t need = m_stretcher.getSamplesRequired();
		if(need == 0)
			break;

		len = need;

		m_buffer.assureSize(len * samplesize);
		buf = m_buffer.getBuffer();

		m_reader->read(len, reader_eos, buf);

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

	std::vector<std::vector<sample_t>> output(channels, std::vector<sample_t>(readAmt));
	std::vector<sample_t*> outputData(channels);

	for(int channel = 0; channel < channels; channel++)
	{
		outputData[channel] = output[channel].data();
	}

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
	m_stretcher.reset();
	m_stretcher.setTimeRatio(timeRatio);
	m_reader->seek(0);
	m_length = 0;
	m_position = 0;
	study();
}

void TimeStretchReader::seek(int position)
{
	m_stretcher.reset();
	m_length = 0;
	m_reader->seek(position);
	study();
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
