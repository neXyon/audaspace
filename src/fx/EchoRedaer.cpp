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

#include "fx/EchoReader.h"

#include <cstring>

#include "Exception.h"
#include "IReader.h"

#include "util/Buffer.h"

AUD_NAMESPACE_BEGIN


EchoReader::EchoReader(std::shared_ptr<IReader> reader, float delay, float feedback, float mix, bool resetBuffer) :
    EffectReader(reader),
    m_delay(delay),
    m_feedback(feedback),
    m_mix(mix),
    m_resetBuffer(resetBuffer)
{
    m_channels = m_reader->getSpecs().channels;
    int rate = m_reader->getSpecs().rate;

    m_delaySamples = static_cast<int>(m_delay * rate);
    m_delayBuffer.resize(m_channels);
    m_writePos = 0;
    m_delayBuffer.assureSize(m_delaySamples * sizeof(sample_t));
}

void EchoReader::read(int& length, bool& eos, sample_t* buffer)
{
    m_inBuffer.assureSize(length * m_channels * sizeof(sample_t));

    bool readerEos = false;
    m_reader->read(length, readerEos, m_inBuffer.getBuffer());
    eos = readerEos;

    sample_t* delayBuf = m_delayBuffer.getBuffer();

    for (int i = 0; i < length; i++)
    {
        for (int c = 0; c < m_channels; c++)
        {
            int pos = ((m_writePos + i) % m_delaySamples) * m_channels + c;

            sample_t inSample = m_inBuffer.getBuffer()[i * m_channels + c];
            sample_t delayedSample = delayBuf[pos];

            sample_t outSample = inSample * (1.0f - m_mix) + delayedSample * m_mix;
            buffer[i * m_channels + c] = outSample;

            // Update delay buffer with feedback
            delayBuf[pos] = inSample + delayedSample * m_feedback;
        }
    }

    m_writePos = (m_writePos + length) % m_delaySamples;
}

void EchoReader::seek(int position)
{
    m_reader->seek(position);

    if (m_resetBuffer) {
        std::memset(m_delayBuffer.getBuffer(), 0, m_delayBuffer.getSize());
        m_writePos = 0;
    }
}

AUD_NAMESPACE_END