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

#include "plugin/PluginManager.h"
#include "file/File.h"
#include "Exception.h"

#include <iostream>
#include <vector>

using namespace aud;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	File file(argv[1]);

	std::vector<StreamInfo> stream_infos;

	try
	{
		stream_infos = file.queryStreams();
	}
	catch(Exception& e)
	{
		std::cerr << "Error opening file " << argv[1] << " - " << e.getMessage() << std::endl;
		return 2;
	}

	int stream_index = 1;

	for(const auto& stream_info : stream_infos)
	{
		std::cout << "Stream: " << stream_index << std::endl;
		std::cout << "\tChannels: " << stream_info.specs.channels << std::endl;
		std::cout << "\tRate: " << stream_info.specs.rate << " Hz" << std::endl;
		std::cout << "\tFormat: " << (stream_info.specs.format & 0x0F) * 8 << " bit ";

		switch((stream_info.specs.format & 0xF0) >> 4)
		{
		case 1:
			std::cout << "signed integer";
			break;
		case 2:
			std::cout << "floating point";
			break;
		default:
			std::cout << "unsigned integer";
			break;
		}

		std::cout << std::endl;
		std::cout << "\tStart: " << stream_info.start << " seconds" << std::endl;
		std::cout << "\tEstimated duration: " << stream_info.duration << " seconds" << std::endl;

		stream_index++;
	}

	return 0;
}
