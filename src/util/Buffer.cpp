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

#include "util/Buffer.h"
#include "Audaspace.h"

#include <cstring>
#include <cstdlib>

#define ALIGN(a) (a + 16 - ((long)a & 15))

AUD_NAMESPACE_BEGIN

Buffer::Buffer(int size)
{
	m_size = size;
	m_buffer = (data_t*) malloc(size+16);
}

Buffer::~Buffer()
{
	free(m_buffer);
}

sample_t* Buffer::getBuffer() const
{
	return (sample_t*) ALIGN(m_buffer);
}

int Buffer::getSize() const
{
	return m_size;
}

void Buffer::resize(int size, bool keep)
{
	if(keep)
	{
		data_t* buffer = (data_t*) malloc(size + 16);

		memcpy(ALIGN(buffer), ALIGN(m_buffer), AUD_MIN(size, m_size));

		free(m_buffer);
		m_buffer = buffer;
	}
	else
		m_buffer = (data_t*) realloc(m_buffer, size + 16);

	m_size = size;
}

void Buffer::assureSize(int size, bool keep)
{
	if(m_size < size)
		resize(size, keep);
}

AUD_NAMESPACE_END
