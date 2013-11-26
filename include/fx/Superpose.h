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

#include "ISound.h"

AUD_NAMESPACE_BEGIN

/**
 * This factory mixes two other factories, playing them the same time.
 * \note Readers from the underlying factories must have the same sample rate
 *       and channel count.
 */
class Superpose : public ISound
{
private:
	/**
	 * First played factory.
	 */
	std::shared_ptr<ISound> m_factory1;

	/**
	 * Second played factory.
	 */
	std::shared_ptr<ISound> m_factory2;

	// hide copy constructor and operator=
	Superpose(const Superpose&);
	Superpose& operator=(const Superpose&);

public:
	/**
	 * Creates a new superpose factory.
	 * \param factory1 The first input factory.
	 * \param factory2 The second input factory.
	 */
	Superpose(std::shared_ptr<ISound> factory1, std::shared_ptr<ISound> factory2);

	virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END
