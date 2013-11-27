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

#include "file/FFMPEGReader.h"
#include "file/SndFileReader.h"

#include "file/File.h"
#include <cstring>

AUD_NAMESPACE_BEGIN

File::File(std::string filename) :
	m_filename(filename)
{
}

File::File(const data_t* buffer, int size) :
	m_buffer(new Buffer(size))
{
	memcpy(m_buffer->getBuffer(), buffer, size);
}

static const char* read_error = "File: File couldn't be read.";

std::shared_ptr<IReader> File::createReader()
{
	try
	{
		if(m_buffer.get())
			return std::shared_ptr<IReader>(new SndFileReader(m_buffer));
		else
			return std::shared_ptr<IReader>(new SndFileReader(m_filename));
	}
	catch(Exception &e) {
		e.error = e.error;
	}

	try
	{
		if(m_buffer.get())
			return std::shared_ptr<IReader>(new FFMPEGReader(m_buffer));
		else
			return std::shared_ptr<IReader>(new FFMPEGReader(m_filename));
	}
	catch(Exception&) {}

	AUD_THROW(ERROR_FILE, read_error);
}

AUD_NAMESPACE_END
