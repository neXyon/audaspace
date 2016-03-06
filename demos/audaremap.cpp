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

#include "file/File.h"
#include "plugin/PluginManager.h"
#include "respec/ChannelMapperReader.h"
#include "Exception.h"
#include "IReader.h"

#include <string>
#include <iostream>

using namespace aud;

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <input_file> <channels>" << std::endl;
		return 1;
	}

	std::string infile = argv[1];
	Channels channels;

	try
	{
		channels = Channels(std::stoi(argv[2]));
	}
	catch(std::exception&)
	{
		std::cerr << "Usage: " << argv[0] << " <input_file> <channels>" << std::endl;
		return 1;
	}

	if(channels < CHANNELS_MONO || channels > CHANNELS_SURROUND71)
	{
		std::cerr << "Error: invalid channel count " << channels << std::endl;
		return 3;
	}

	PluginManager::loadPlugins("");

	File file(infile);
	std::shared_ptr<ChannelMapperReader> reader;

	try
	{
		reader = std::shared_ptr<ChannelMapperReader>(new ChannelMapperReader(file.createReader(), channels));
	}
	catch(Exception& e)
	{
		std::cerr << "Error opening file " << argv[1] << " - " << e.getMessage() << std::endl;
		return 2;
	}

	Channels source_channels = reader->getSourceChannels();

	for(int source = 0; source < source_channels; source++)
	{
		for(int target = 0; target < channels; target++)
		{
			std::cout << reader->getMapping(source, target) << " ";
		}
		std::cout << std::endl;
	}

	return 0;
}
