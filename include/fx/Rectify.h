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
 * This factory rectifies another factory.
 */
class Rectify : public Effect
{
private:
	// delete copy constructor and operator=
	Rectify(const Rectify&) = delete;
	Rectify& operator=(const Rectify&) = delete;

public:
	/**
	 * Creates a new rectify factory.
	 * \param factory The input factory.
	 */
	Rectify(std::shared_ptr<ISound> factory);

	virtual std::shared_ptr<IReader> createReader();

	static sample_t rectifyFilter(CallbackIIRFilterReader* reader, void* useless);
};

AUD_NAMESPACE_END
