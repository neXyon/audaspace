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

#include "fx/CompressorReader.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

AUD_NAMESPACE_BEGIN

CompressorReader::CompressorReader(std::shared_ptr<IReader> reader,
                                   float thresholdRatio,
                                   float ratio,
                                   float attackSec,
                                   float releaseSec,
                                   float makeupGainRatio,
                                   float kneeWidthDb,
                                   float lookaheadSec) :
    EffectReader(reader),
    m_thresholdRatio(thresholdRatio),
    m_ratio(ratio),
    m_attackSec(attackSec),
    m_releaseSec(releaseSec),
    m_makeupGainRatio(makeupGainRatio),
    m_kneeWidthDb(kneeWidthDb)
{
  Specs specs = m_reader->getSpecs();
  m_channels = specs.channels;
  m_lookaheadSamples = 0;
  m_delayBufferWritePos = 0;

  // Envelope follower coefficients calculation
  m_attackCoeff = std::exp(-1.0f / (m_attackSec * specs.rate));
  m_releaseCoeff = std::exp(-1.0f / (m_releaseSec * specs.rate));

  m_envelope.resize(m_channels, 0.0f);
  m_rmsState.resize(m_channels, 0.0f);

  if (lookaheadSec > 0.0f && m_channels > 0) {
    m_lookaheadSamples = static_cast<int>(lookaheadSec * specs.rate);
    if (m_lookaheadSamples > 0) {
      // Buffer needs to be larger than lookahead to avoid read/write collision
      int safeSize = (m_lookaheadSamples + 1024) * m_channels * sizeof(sample_t);
      m_delayBuffer.assureSize(safeSize, 0.0f);
    }
  }
}

void CompressorReader::read(int &length, bool &eos, sample_t *buffer)
{
  if (m_lookaheadSamples > 0) {
    // --- LOOKAHEAD PATH ---
    m_reader->read(length, eos, buffer);

    if (length == 0) {
      return;
    }

    Specs specs = m_reader->getSpecs();
    const int channels = specs.channels;
    const int total_samples = length * channels;

    float thresholdRatio = m_thresholdRatio;
    float ratio = m_ratio;
    float makeup = m_makeupGainRatio;
    float kneeDb = m_kneeWidthDb;

    if (m_rmsState.size() != channels) {
      m_rmsState.assign(channels, 0.0f);
    }
    if (m_envelope.size() != channels) {
      m_envelope.assign(channels, 0.0f);
    }

    float rms_coeff = std::exp(-1.0f / (0.02f * specs.rate));
    float min_rms = 1e-12f;
    float attack_coeff = m_attackCoeff;
    float release_coeff = m_releaseCoeff;

    sample_t *delayBuffer = m_delayBuffer.getBuffer();
    int delayBufferSize = m_delayBuffer.getSize() / sizeof(sample_t);

    for (int i = 0; i < total_samples; ++i) {
      int ch = i % channels;
      
      float sample = buffer[i];

      float sq = sample * sample;
      m_rmsState[ch] = rms_coeff * m_rmsState[ch] + (1.0f - rms_coeff) * sq;
      float detector = std::sqrt(std::max(m_rmsState[ch], min_rms));
      float detector_db = 20.0f * log10(detector + min_rms);
      float threshold_db = 20.0f * log10(thresholdRatio);
      float kneestart = threshold_db - kneeDb / 2.0f;
      float kneeend = threshold_db + kneeDb / 2.0f;
      float gainReduction_db = 0.0f;

      if (detector_db < kneestart) {
        gainReduction_db = 0.0f;
      }
      else if (detector_db > kneeend) {
        gainReduction_db = (detector_db - threshold_db) * (1.0f - 1.0f / ratio);
      }
      else {
        float x = detector_db - kneestart;
        float y = x * x / (2.0f * kneeDb);
        gainReduction_db = y * (1.0f - 1.0f / ratio);
      }

      float env = m_envelope[ch];
      if (gainReduction_db > env) {
        env = attack_coeff * env + (1.0f - attack_coeff) * gainReduction_db;
      }
      else {
        env = release_coeff * env + (1.0f - release_coeff) * gainReduction_db;
      }
      m_envelope[ch] = env;

      int delayIndex = (m_delayBufferWritePos - m_lookaheadSamples * channels + delayBufferSize) %
                       delayBufferSize;
      float delayedSample = delayBuffer[delayIndex];

      delayBuffer[m_delayBufferWritePos] = buffer[i];

      float smoothedGainReduction = std::pow(10.0f, -env / 20.0f);
      float gain = smoothedGainReduction * makeup;
      buffer[i] = delayedSample * gain;

      m_delayBufferWritePos = (m_delayBufferWritePos + 1) % delayBufferSize;
    }
  }
  else {

    m_reader->read(length, eos, buffer);
    if (length == 0) {
      return;
    }

    Specs specs = m_reader->getSpecs();
    const int channels = specs.channels;
    const int total_samples = length * channels;

    float thresholdRatio = m_thresholdRatio;
    float ratio = m_ratio;
    float makeup = m_makeupGainRatio;
    float kneeDb = m_kneeWidthDb;

    if (m_rmsState.size() != channels) {
      m_rmsState.assign(channels, 0.0f);
    }
    if (m_envelope.size() != channels) {
      m_envelope.assign(channels, 0.0f);
    }

    float rms_coeff = std::exp(-1.0f / (0.02f * specs.rate));
    float min_rms = 1e-12f;
    float attack_coeff = m_attackCoeff;
    float release_coeff = m_releaseCoeff;

    for (int i = 0; i < total_samples; ++i) {
      int ch = i % channels;
      float sample = buffer[i];

      float sq = sample * sample;
      m_rmsState[ch] = rms_coeff * m_rmsState[ch] + (1.0f - rms_coeff) * sq;
      float detector = std::sqrt(std::max(m_rmsState[ch], min_rms));
      float detector_db = 20.0f * log10(detector + min_rms);
      float threshold_db = 20.0f * log10(thresholdRatio);
      float kneestart = threshold_db - kneeDb / 2.0f;
      float kneeend = threshold_db + kneeDb / 2.0f;
      float gainReduction_db = 0.0f;

      if (detector_db < kneestart) {
        gainReduction_db = 0.0f;
      }
      else if (detector_db > kneeend) {
        gainReduction_db = (detector_db - threshold_db) * (1.0f - 1.0f / ratio);
      }
      else {
        float x = detector_db - kneestart;
        float y = x * x / (2.0f * kneeDb);
        gainReduction_db = y * (1.0f - 1.0f / ratio);
      }

      float env = m_envelope[ch];
      if (gainReduction_db > env) {
        env = attack_coeff * env + (1.0f - attack_coeff) * gainReduction_db;
      }
      else {
        env = release_coeff * env + (1.0f - release_coeff) * gainReduction_db;
      }
      m_envelope[ch] = env;

      float smoothedGainReduction = std::pow(10.0f, -env / 20.0f);
      float gain = smoothedGainReduction * makeup;
      
      buffer[i] = sample * gain;
    }
  }
}

AUD_NAMESPACE_END