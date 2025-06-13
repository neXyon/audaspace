/*******************************************************************************
 * Copyright 2009-2016 Jörg Müller
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
#include "IReader.h"
#include "util/Buffer.h"

#include <iostream>
#include <chrono>
AUD_NAMESPACE_BEGIN
 
TimeStretchReader::TimeStretchReader(std::shared_ptr<IReader> reader, double time_ratio, int buffersize) :
    EffectReader(reader), m_ratio(time_ratio), m_buffersize(buffersize), m_position(0), m_length(0), m_specs(reader->getSpecs()), m_stretcher(reader->getSpecs().rate, reader->getSpecs().channels, RubberBandStretcher::OptionPitchHighQuality |
    RubberBandStretcher::OptionWindowStandard |
    RubberBandStretcher::OptionProcessOffline |
    RubberBandStretcher::OptionThreadingAlways, time_ratio)
{
  Buffer buffer(buffersize * AUD_SAMPLE_SIZE(m_specs));
  sample_t* buf = buffer.getBuffer();

  int channels = m_specs.channels;
  int length = reader->getLength();

  m_processedData = new sample_t[int(reader->getLength() * time_ratio * channels)];
 
  sample_t** input = new sample_t*[channels];
  for (int channel = 0; channel < channels; channel++) 
  {
    input[channel] = new sample_t[length];  
  }

  const sample_t** studyInput = new const sample_t*[channels];


  int len;
  bool eos = false;
  
  for(unsigned int pos = 0; ((pos < length) || (length <= 0)) && !eos; pos += len)
  {
    len = buffersize;
    if((len > length - pos) && (length > 0))
      len = length - pos;
    reader->read(len, eos, buf);

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
      for(int i = 0; i < len; i++)
      {
        input[channel][pos + i] = buf[i * channels + channel];
      }
    }

    for(int channel = 0; channel < channels; channel++) 
    {
      studyInput[channel] = input[channel] + pos;
    }

    m_stretcher.study(studyInput, len, eos);
  }

  // note: should the process method be called on buffers size instead of the entire audio data?
  m_stretcher.process(input, length, true);


  // Interleave the audio into m_processedData
  while (m_stretcher.available() > 0) 
  {
    float* output[channels];
  
    for(int channel = 0; channel < channels; channel++)
    {
      output[channel] = new float[buffersize];
    }
  
    size_t frameRetrieved = m_stretcher.retrieve(output, buffersize);

    for(int i = 0; i < frameRetrieved; i++) 
    {
      for(int channel = 0; channel < channels; channel++) 
      {
        m_processedData[(m_length + i) * channels + channel] = output[channel][i];
      }
    }
    m_length += frameRetrieved;
  }
 
  for(int channel = 0; channel < channels; channel++)
  {
    delete[] input[channel];
  }
  delete[] input;
  delete[] studyInput;
}
  

TimeStretchReader::~TimeStretchReader() {
  delete[] m_processedData;
}

void TimeStretchReader::read(int& length, bool& eos, sample_t* buffer)
{
  int channels = m_specs.channels;

  int available = m_length - m_position;
  int readAmt = std::min(length, available);

  std::memcpy(buffer, m_processedData + m_position * channels, readAmt * AUD_SAMPLE_SIZE(m_specs));

  m_position += readAmt;
  length = readAmt;
  eos = (m_position >= m_length);
}
 
float TimeStretchReader::getRatio() const
{
  return m_ratio;
}

void TimeStretchReader::seek(int position)
{
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

AUD_NAMESPACE_END
 