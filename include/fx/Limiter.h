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

#include "Effect.h"

AUD_NAMESPACE_BEGIN

/**
 * This factory limits another factory in start and end time.
 */
class Limiter : public Effect
{
private:
	/**
	 * The start time.
	 */
	const float m_start;

	/**
	 * The end time.
	 */
	const float m_end;

	// hide copy constructor and operator=
	Limiter(const Limiter&);
	Limiter& operator=(const Limiter&);

public:
	/**
	 * Creates a new limiter factory.
	 * \param factory The input factory.
	 * \param start The desired start time.
	 * \param end The desired end time, a negative value signals that it should
	 *            play to the end.
	 */
	Limiter(std::shared_ptr<ISound> factory,
					   float start = 0, float end = -1);

	/**
	 * Returns the start time.
	 */
	float getStart() const;

	/**
	 * Returns the end time.
	 */
	float getEnd() const;

	virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END
