/*******************************************************************************
 * Copyright 2009-2016 Jörg Müller
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

/**
 * @file FileManager.h
 * @ingroup file
 * The FileManager class.
 */

#include "FileInfo.h"
#include "respec/Specification.h"
#include "IWriter.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

AUD_NAMESPACE_BEGIN

class IFileInput;
class IFileOutput;
class IReader;
class Buffer;

/**
 * The FileManager manages all file input and output plugins.
 */
class AUD_API FileManager
{
private:
	static std::list<std::shared_ptr<IFileInput>>& inputs();
	static std::list<std::shared_ptr<IFileOutput>>& outputs();

	// delete copy constructor and operator=
	FileManager(const FileManager&) = delete;
	FileManager& operator=(const FileManager&) = delete;
	FileManager() = delete;

public:
	/**
	 * Registers a file input used to create an IReader to read from a file.
	 * @param input The IFileInput to register.
	 */
	static void registerInput(std::shared_ptr<IFileInput> input);

	/**
	 * Registers a file output used to create an IWriter to write to a file.
	 * @param output The IFileOutput to register.
	 */
	static void registerOutput(std::shared_ptr<IFileOutput> output);

	/**
	 * Creates a file reader for the given filename if a registed IFileInput is able to read it.
	 * @param filename The path to the file.
	 * @param stream The index of the audio stream within the file if it contains multiple audio streams.
	 * @return The reader created.
	 * @exception Exception If no file input can read the file an exception is thrown.
	 */
	static std::shared_ptr<IReader> createReader(const std::string &filename, int stream = 0);

	/**
	 * Creates a file reader for the given buffer if a registed IFileInput is able to read it.
	 * @param buffer The buffer to read the file from.
	 * @param stream The index of the audio stream within the file if it contains multiple audio streams.
	 * @return The reader created.
	 * @exception Exception If no file input can read the file an exception is thrown.
	 */
	static std::shared_ptr<IReader> createReader(std::shared_ptr<Buffer> buffer, int stream = 0);

	/**
	 * Queries the streams of a sound file.
	 * \param filename Path to the file to be read.
	 * \return A vector with as many streams as there are in the file.
	 * \exception Exception Thrown if the file specified cannot be read.
	 */
	static std::vector<StreamInfo> queryStreams(const std::string &filename);

	/**
	 * Queries the streams of a sound file.
	 * \param buffer The in-memory file buffer.
	 * \return A vector with as many streams as there are in the file.
	 * \exception Exception Thrown if the file specified cannot be read.
	 */
	static std::vector<StreamInfo> queryStreams(std::shared_ptr<Buffer> buffer);

	/**
	 * Creates a file writer that writes a sound to the given file path.
	 * Existing files will be overwritten.
	 * @param filename The file path to write to.
	 * @param specs The output specification.
	 * @param format The container format for the file.
	 * @param codec The codec used inside the container.
	 * @param bitrate The bitrate to write with.
	 * @return A writer that creates the file.
	 * @exception Exception If no file output can write the file with the given specification an exception is thrown.
	 */
	static std::shared_ptr<IWriter> createWriter(const std::string &filename, DeviceSpecs specs, Container format, Codec codec, unsigned int bitrate);
};

AUD_NAMESPACE_END
