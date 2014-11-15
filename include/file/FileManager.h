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

/**
 * @file FileManager.h
 * @ingroup file
 * The FileManager class.
 */

#include "respec/Specification.h"
#include "IWriter.h"

#include <list>
#include <memory>

AUD_NAMESPACE_BEGIN

class IFileInput;
class IFileOutput;
class IReader;
class Buffer;

/**
 * This class manages all file input and output plugins.
 */
class FileManager
{
private:
	static std::list<std::shared_ptr<IFileInput>> m_inputs;
	static std::list<std::shared_ptr<IFileOutput>> m_outputs;

	// delete copy constructor and operator=
	FileManager(const FileManager&) = delete;
	FileManager& operator=(const FileManager&) = delete;
	FileManager() = delete;

public:
	static void registerInput(std::shared_ptr<IFileInput> input);
	static void registerOutput(std::shared_ptr<IFileOutput> output);

	static std::shared_ptr<IReader> createReader(std::string filename);
	static std::shared_ptr<IReader> createReader(std::shared_ptr<Buffer> buffer);
	static std::shared_ptr<IWriter> createWriter(std::string filename, DeviceSpecs specs, Container format, Codec codec, unsigned int bitrate);
};

AUD_NAMESPACE_END
