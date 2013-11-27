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

#include "fx/BaseIIRFilterReader.h"
#include "util/Buffer.h"

AUD_NAMESPACE_BEGIN

class CallbackIIRFilterReader;

typedef sample_t (*doFilterIIR)(CallbackIIRFilterReader*, void*);
typedef void (*endFilterIIR)(void*);

/**
 * This class provides an interface for infinite impulse response filters via a
 * callback filter function.
 */
class CallbackIIRFilterReader : public BaseIIRFilterReader
{
private:
	/**
	 * Filter function.
	 */
	const doFilterIIR m_filter;

	/**
	 * End filter function.
	 */
	const endFilterIIR m_endFilter;

	/**
	 * Data pointer.
	 */
	void* m_data;

	// hide copy constructor and operator=
	CallbackIIRFilterReader(const CallbackIIRFilterReader&);
	CallbackIIRFilterReader& operator=(const CallbackIIRFilterReader&);

public:
	/**
	 * Creates a new callback IIR filter reader.
	 * \param reader The reader to read from.
	 * \param in The count of past input samples needed.
	 * \param out The count of past output samples needed.
	 * \param doFilter The filter callback.
	 * \param endFilter The finishing callback.
	 * \param data Data pointer for the callbacks.
	 */
	CallbackIIRFilterReader(std::shared_ptr<IReader> reader, int in, int out,
								doFilterIIR doFilter,
								endFilterIIR endFilter = 0,
								void* data = NULL);

	virtual ~CallbackIIRFilterReader();

	virtual sample_t filter();
};

AUD_NAMESPACE_END
