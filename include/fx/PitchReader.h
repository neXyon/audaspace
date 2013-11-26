/*******************************************************************************
 * Copyright 2009-2013 Jörg Müller
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

#include "EffectReader.h"

AUD_NAMESPACE_BEGIN

/**
 * This class reads another reader and changes it's pitch.
 */
class PitchReader : public EffectReader
{
private:
	/**
	 * The pitch level.
	 */
	float m_pitch;

	// hide copy constructor and operator=
	PitchReader(const PitchReader&);
	PitchReader& operator=(const PitchReader&);

public:
	/**
	 * Creates a new pitch reader.
	 * \param reader The reader to read from.
	 * \param pitch The pitch value.
	 */
	PitchReader(std::shared_ptr<IReader> reader, float pitch);

	virtual Specs getSpecs() const;

	/**
	 * Retrieves the pitch.
	 * \return The current pitch value.
	 */
	float getPitch() const;

	/**
	 * Sets the pitch.
	 * \param pitch The new pitch value.
	 */
	void setPitch(float pitch);
};

AUD_NAMESPACE_END
