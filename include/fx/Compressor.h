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

#include "fx/Effect.h"

AUD_NAMESPACE_BEGIN

/*
 * This effect compresses the audio signal. based on the specified threshold and ratio and other parameters.
 */

class AUD_API Compressor : public Effect
{
private:
	float m_threshold;
	float m_ratio;
	float m_attack;
	float m_release;
	float m_makeupGain;
	float m_knee;
	float m_lookahead;

	// delete copy constructor and assignment operator
	Compressor(const Compressor&) = delete;
	Compressor& operator=(const Compressor&) = delete;

public:
	/**
	 * @brief Construct a new Compressor object
	 *
	 * @param threshold The threshold ratio (0.0 to 1.0) over which to start compression.
	 * @param ratio The compression ratio (e.g., 4.0 for 4:1).
	 * @param attack The attack time in seconds.
	 * @param release The release time in seconds.
	 * @param makeupGain The output gain ratio to apply after compression (1.0 for no gain).
	 * @param knee The knee ratio (1.0 for hard knee, higher for soft knee).
	 * @param lookahead The lookahead time in seconds.
	 */
	Compressor(std::shared_ptr<ISound> sound, float threshold, float ratio, float attack, float release, float makeupGain, float knee, float lookahead);

	float getThreshold() const;

	float getRatio() const;

	float getAttack() const;

	float getRelease() const;

	float getGain() const;

	float getLookahead() const;

	virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END
