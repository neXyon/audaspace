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

#pragma once

/**
 * @file TimeStretch.h
 * @ingroup fx
 * The TimeStretch class.
 */

#include "fx/Effect.h"

AUD_NAMESPACE_BEGIN

enum class TimeStretchQuality
{
	FASTEST = 0, // Use the high speed
	HIGH = 1,    /// Use high quality pitch option
};

/**
 * This sound allows a sound to be time-stretched by the specified ratio
 * \note The reader has to be seekable.
 */
class AUD_API TimeStretch : public Effect
{
private:
	/**
	 * The time ratio to stretch by.
	 */
	double m_timeRatio;

	/**
	 * The quality of the pitch correction when time-stretching
	 */
	TimeStretchQuality m_quality;

	// delete copy constructor and operator=
	TimeStretch(const TimeStretch&) = delete;
	TimeStretch& operator=(const TimeStretch&) = delete;

public:
	/**
	 * Creates a new pitch-corrected sound after stretching by the given ratio.
	 * \param sound The input sound.
	 * \param ratio The ratio for the stretcher
	 */
	TimeStretch(std::shared_ptr<ISound> sound, double m_timeRatio, TimeStretchQuality quality);

	/**
	 * Returns the time ratio.
	 */
	double getTimeRatio() const;

	virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END
