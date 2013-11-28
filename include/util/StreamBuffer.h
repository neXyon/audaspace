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

#include <memory>

AUD_NAMESPACE_BEGIN

class Buffer;

/**
 * This factory creates a buffer out of a reader. This way normally streamed
 * sound sources can be loaded into memory for buffered playback.
 */
class StreamBuffer : public ISound
{
private:
	/**
	 * The buffer that holds the audio data.
	 */
	std::shared_ptr<Buffer> m_buffer;

	/**
	 * The specification of the samples.
	 */
	Specs m_specs;

	// delete constructor and operator=
	StreamBuffer(const StreamBuffer&) = delete;
	StreamBuffer& operator=(const StreamBuffer&) = delete;

public:
	/**
	 * Creates the factory and reads the reader created by the factory supplied
	 * to the buffer.
	 * \param factory The factory that creates the reader for buffering.
	 * \exception Exception Thrown if the reader cannot be created.
	 */
	StreamBuffer(std::shared_ptr<ISound> factory);

	virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END
