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

#include "fx/Effect.h"

AUD_NAMESPACE_BEGIN

/**
 * This factory changes the volume of another factory.
 * The set volume should be a value between 0.0 and 1.0, higher values at your
 * own risk!
 */
class Volume : public Effect
{
private:
	/**
	 * The volume.
	 */
	const float m_volume;

	// hide copy constructor and operator=
	Volume(const Volume&);
	Volume& operator=(const Volume&);

public:
	/**
	 * Creates a new volume factory.
	 * \param factory The input factory.
	 * \param volume The desired volume.
	 */
	Volume(std::shared_ptr<ISound> factory, float volume);

	/**
	 * Returns the volume.
	 * \return The current volume.
	 */
	float getVolume() const;

	virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END
