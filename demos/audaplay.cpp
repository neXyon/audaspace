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

#include <iostream>
#include <thread>
#include <chrono>

#include "devices/OpenALDevice.h"
#include "file/File.h"

using namespace aud;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
		return 1;
	}

	DeviceSpecs specs;
	specs.format = FORMAT_FLOAT32;
	std::shared_ptr<IReader> reader;

	File file(argv[1]);
	try
	{
		reader = file.createReader();
	}
	catch(Exception& e)
	{
		std::cerr << "Error opening file " << argv[1] << " - " << e.str << std::endl;
		return 2;
	}

	specs.specs = reader->getSpecs();

	OpenALDevice device(specs);
	auto handle = device.play(reader);

	auto duration = std::chrono::seconds(reader->getLength()) / specs.rate;

	std::cout << "Duration: " << duration.count() << " seconds" << std::endl;

	std::this_thread::sleep_for(duration);

	while(handle->getStatus() == STATUS_PLAYING)
	{
		std::this_thread::yield();
	}

	return 0;
}
