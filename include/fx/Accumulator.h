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

class CallbackIIRFilterReader;

/**
 * This factory creates an accumulator reader.
 *
 * The accumulator adds the difference at the input to the last output in case
 * it's positive. In additive mode it additionaly adds the difference always.
 * So in case the difference is positive, it's added twice.
 */
class Accumulator : public Effect
{
private:
	/**
	 * Whether the accumulator is additive.
	 */
	const bool m_additive;

	// delete copy constructor and operator=
	Accumulator(const Accumulator&) = delete;
	Accumulator& operator=(const Accumulator&) = delete;

public:
	/**
	 * Creates a new accumulator factory.
	 * \param factory The input factory.
	 * \param additive Whether the accumulator is additive.
	 */
	Accumulator(std::shared_ptr<ISound> factory, bool additive = false);

	virtual std::shared_ptr<IReader> createReader();

	static sample_t accumulatorFilterAdditive(CallbackIIRFilterReader* reader, void* useless);
	static sample_t accumulatorFilter(CallbackIIRFilterReader* reader, void* useless);
};

AUD_NAMESPACE_END
