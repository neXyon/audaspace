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

EchoReader::EchoReader(std::shared_ptr<IReader> reader, double delaySec, double feedback, double mix) :
    EffectReader(reader),
    m_delaySec(delaySec),
    m_feedback(feedback),
    m_mix(mix)
{
    int channels = m_reader->getSpecs().channels;
    int rate = m_reader->getSpecs().rate;

    m_delaySamples = static_cast<int>(m_delaySec * rate);
    m_delayBuffer.resize(channels);
    m_writePos.resize(channels, 0);
    
    for (int c = 0; c < channels; c++)
    {
        m_delayBuffer[c] = std::make_unique<Buffer>();
        m_delayBuffer[c]->assureSize(m_delaySamples * sizeof(sample_t));
    }
}

void EchoReader::read(int& length, bool& eos, sample_t* buffer)
{
    int channels = m_reader->getSpecs().channels;
    Buffer inBuffer;
    inBuffer.assureSize(length * channels * sizeof(sample_t));

    bool readerEos = false;
    m_reader->read(length, readerEos, inBuffer.getBuffer());
    eos = readerEos;

    for (int i = 0; i < length; i++)
    {
        for (int c = 0; c < channels; c++)
        {
            sample_t* delayBuf = m_delayBuffer[c]->getBuffer(); 
            int pos = m_writePos[c];

            sample_t inSample = inBuffer.getBuffer()[i * channels + c];
            sample_t delayedSample = delayBuf[pos];

            sample_t outSample = static_cast<sample_t>(inSample * (1.0 - m_mix) + delayedSample * m_mix);
            buffer[i * channels + c] = outSample;
            
            // Update delay buffer with feedback
            delayBuf[pos] = static_cast<sample_t>(inSample + delayedSample * m_feedback);
            
            m_writePos[c] = (pos + 1) % m_delaySamples;
        }
    }
}

void EchoReader::seek(int position)
{
    m_reader->seek(position);
    // Reset delay buffer
    for (auto& buf : m_delayBuffer)
        std::memset(buf->getBuffer(), 0, buf->getSize());
    std::fill(m_writePos.begin(), m_writePos.end(), 0);
}

int EchoReader::getLength() const { return m_reader->getLength(); }
int EchoReader::getPosition() const { return 0; } // Could track position if needed

AUD_NAMESPACE_END