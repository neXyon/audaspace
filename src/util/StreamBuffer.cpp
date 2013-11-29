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

#include "util/StreamBuffer.h"
#include "util/BufferReader.h"
#include "util/Buffer.h"

AUD_NAMESPACE_BEGIN

StreamBuffer::StreamBuffer(std::shared_ptr<ISound> factory) :
	m_buffer(new Buffer())
{
	std::shared_ptr<IReader> reader = factory->createReader();

	m_specs = reader->getSpecs();

	int sample_size = AUD_SAMPLE_SIZE(m_specs);
	int length;
	int index = 0;
	bool eos = false;

	// get an approximated size if possible
	int size = reader->getLength();

	if(size <= 0)
		size = AUD_BUFFER_RESIZE_BYTES / sample_size;
	else
		size += m_specs.rate;

	// as long as the end of the stream is not reached
	while(!eos)
	{
		// increase
		m_buffer->resize(size*sample_size, true);

		// read more
		length = size-index;
		reader->read(length, eos, m_buffer->getBuffer() + index * m_specs.channels);
		if(index == m_buffer->getSize() / sample_size)
			size += AUD_BUFFER_RESIZE_BYTES / sample_size;
		index += length;
	}

	m_buffer->resize(index * sample_size, true);
}

std::shared_ptr<IReader> StreamBuffer::createReader()
{
	return std::shared_ptr<IReader>(new BufferReader(m_buffer, m_specs));
}

AUD_NAMESPACE_END
