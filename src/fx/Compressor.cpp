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

#include "fx/Compressor.h"

#include "fx/CompressorReader.h"

AUD_NAMESPACE_BEGIN

Compressor::Compressor(std::shared_ptr<ISound> sound, float threshold, float ratio, float attack, float release, float makeupGain, float kneeWidth, float lookaheadMs) :
    Effect(sound), m_threshold(threshold), m_ratio(ratio), m_attack(attack), m_release(release), m_makeupGain(makeupGain), m_kneeWidth(kneeWidth), m_lookaheadMs(lookaheadMs)
{
}

float Compressor::getThreshold() const
{
	return m_threshold;
}

float Compressor::getRatio() const
{
	return m_ratio;
}

float Compressor::getAttack() const
{
	return m_attack;
}

float Compressor::getRelease() const
{
	return m_release;
}

float Compressor::getGain() const
{
	return m_makeupGain;
}

std::shared_ptr<IReader> Compressor::createReader()
{
	// Implementation for creating a reader specific to the compressor effect
	return std::shared_ptr<IReader>(new CompressorReader(m_sound->createReader(), m_threshold, m_ratio, m_attack, m_release, m_makeupGain, m_kneeWidth, m_lookaheadMs));
}

AUD_NAMESPACE_END
