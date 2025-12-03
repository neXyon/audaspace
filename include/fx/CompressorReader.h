/*******************************************************************************
 * Copyright 2015-2025 Ketsebaot Gizachew
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

#pragma once

#include <vector>

#include "fx/EffectReader.h"
#include "util/Buffer.h"

AUD_NAMESPACE_BEGIN

class CompressorReader : public EffectReader
{
public:
    // All time units are now in seconds, gains are ratios (1.0 = 0 dB)
    CompressorReader(std::shared_ptr<IReader> reader, float thresholdRatio, float ratio, float attackSec, float releaseSec, float makeupGainRatio, float kneeWidthDb, float lookaheadSec);
    virtual void read(int& length, bool& eos, sample_t* buffer) override;

private:
    float m_thresholdRatio;      // Threshold as ratio (1.0 = 0 dB)
    float m_ratio;               // Compression ratio
    float m_attackSec;           // Attack time in seconds
    float m_releaseSec;          // Release time in seconds
    float m_makeupGainRatio;     // Makeup gain as ratio (1.0 = 0 dB)
    float m_kneeWidthDb;         // Knee width in dB
    int m_lookaheadSamples;      // Lookahead in samples
    aud::Buffer m_delayBuffer;
    aud::Buffer m_sidechainBuffer;
    int m_delayBufferWritePos;

    float m_attackCoeff;
    float m_releaseCoeff;
    std::vector<float> m_rmsState;

    int m_channels;
    int m_windowSize;
    std::vector<float> m_envelope;

    CompressorReader(const CompressorReader&) = delete;
    CompressorReader& operator=(const CompressorReader&) = delete;
};

AUD_NAMESPACE_END