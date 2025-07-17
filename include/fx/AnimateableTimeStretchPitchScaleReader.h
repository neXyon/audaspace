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
 * @file AnimateableTimeStretchPitchScaleReader.h
 * @ingroup fx
 * The AnimateableTimeStretchPitchScaleReader class.
 */

#include "fx/EffectReader.h"
#include "fx/TimeStretchPitchScaleReader.h"
#include "sequence/AnimateableProperty.h"
#include "util/Buffer.h"

using namespace RubberBand;

AUD_NAMESPACE_BEGIN

/**
 * This class reads from another reader and applies time-stretching and pitch scaling with support for animating both properties.
 */
class AUD_API AnimateableTimeStretchPitchScaleReader : public EffectReader
{
private:
	/**
	 * The time stretch and pitch scale reader
	 */
	std::shared_ptr<TimeStretchPitchScaleReader> m_reader;

	/**
	 * The animateable time stretch factor
	 */
	AnimateableProperty m_time_stretch;

	/**
	 * The animateable pitch scale factor
	 */
	AnimateableProperty m_pitch_scale;

	/**
	 * The current position.
	 */
	int m_position;

	// delete copy constructor and operator=
	AnimateableTimeStretchPitchScaleReader(const AnimateableTimeStretchPitchScaleReader&) = delete;
	AnimateableTimeStretchPitchScaleReader& operator=(const AnimateableTimeStretchPitchScaleReader&) = delete;

public:
	/**
	 * Creates a new animateable time-stretch, pitch scale reader.
	 * \param reader The input reader.
	 * \param timeRatio The initial time ratio.
	 * \param pitchScale The initial pitch scale.
	 * \param quality The quality options.
	 * \param preserveFormant Whether to preserve vocal formants.
	 */
	AnimateableTimeStretchPitchScaleReader(std::shared_ptr<IReader> reader, float timeRatio, float pitchScale, StretcherQualityOptions quality, bool preserveFormant);

	/**
	 * Retrieves one of the animated properties of the entry.
	 * \param type Which animated property to retrieve.
	 * \return A pointer to the animated property, valid as long as the
	 *         entry is.
	 */
	AnimateableProperty* getAnimProperty(AnimateablePropertyType type);

	virtual void read(int& length, bool& eos, sample_t* buffer) override;

	virtual void seek(int position) override;
	virtual int getLength() const override;
	virtual int getPosition() const override;
};

AUD_NAMESPACE_END