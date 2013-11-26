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

#include <string>
#include <vector>
#include <memory>

#include "IWriter.h"
#include "IReader.h"

AUD_NAMESPACE_BEGIN

/**
 * This class is able to create IWriter classes as well as write reads to them.
 */
class FileWriter
{
private:
	// hide default constructor, copy constructor and operator=
	FileWriter();
	FileWriter(const FileWriter&);
	FileWriter& operator=(const FileWriter&);

public:
	/**
	 * Creates a new IWriter.
	 * \param filename The file to write to.
	 * \param specs The file's audio specification.
	 * \param format The file's container format.
	 * \param codec The codec used for encoding the audio data.
	 * \param bitrate The bitrate for encoding.
	 * \return The writer to write data to.
	 */
	static std::shared_ptr<IWriter> createWriter(std::string filename, DeviceSpecs specs, Container format, Codec codec, unsigned int bitrate);

	/**
	 * Writes a reader to a writer.
	 * \param reader The reader to read from.
	 * \param writer The writer to write to.
	 * \param length How many samples should be transferred.
	 * \param buffersize How many samples should be transferred at once.
	 */
	static void writeReader(std::shared_ptr<IReader> reader, std::shared_ptr<IWriter> writer, unsigned int length, unsigned int buffersize);

	/**
	 * Writes a reader to several writers.
	 * \param reader The reader to read from.
	 * \param writers The writers to write to.
	 * \param length How many samples should be transferred.
	 * \param buffersize How many samples should be transferred at once.
	 */
	static void writeReader(std::shared_ptr<IReader> reader, std::vector<std::shared_ptr<IWriter> >& writers, unsigned int length, unsigned int buffersize);
};

AUD_NAMESPACE_END
