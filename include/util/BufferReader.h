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

#include <memory>
#include "IReader.h"

AUD_NAMESPACE_BEGIN

class Buffer;

/**
 * This class represents a simple reader from a buffer that exists in memory.
 * \warning Notice that the buffer used for creating the reader must exist as
 *          long as the reader exists.
 */
class BufferReader : public IReader
{
private:
	/**
	 * The current position in the buffer.
	 */
	int m_position;

	/**
	 * The buffer that is read.
	 */
	std::shared_ptr<Buffer> m_buffer;

	/**
	 * The specification of the sample data in the buffer.
	 */
	Specs m_specs;

	// hide copy constructor and operator=
	BufferReader(const BufferReader&);
	BufferReader& operator=(const BufferReader&);

public:
	/**
	 * Creates a new buffer reader.
	 * \param buffer The buffer to read from.
	 * \param specs The specification of the sample data in the buffer.
	 */
	BufferReader(std::shared_ptr<Buffer> buffer, Specs specs);

	virtual bool isSeekable() const;
	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;
	virtual Specs getSpecs() const;
	virtual void read(int& length, bool& eos, sample_t* buffer);
};

AUD_NAMESPACE_END
