/*******************************************************************************
* Copyright 2015-2016 Juan Francisco Crespo Galán
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

#include "fx/PlaybackManager.h"
#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "file/File.h"

#include <iostream>
#include <thread>
#include <chrono>

using namespace aud;

int main(int argc, char* argv[])
{
	if(argc != 5)
	{
		std::cerr << "Usage: " << argv[0] << " <sound 1 category 1>" << " <sound 2 category 1>" << " <sound 1 category 2>" << " <sound 2 category 2>" << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();
	PlaybackManager manager(device);
	std::shared_ptr<File> file;
	for(int i = 1; i < argc; i++)
	{
		file = (std::make_shared<File>(argv[i]));
		if(i <= 2)
			manager.play(file, 0);
		else
			manager.play(file, 1);
	}
	manager.setVolume(0.2f, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.setVolume(0.0f, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.pause(0);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.setVolume(0.5f, 1);
	manager.setVolume(1.0f, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.stop(1);
	manager.resume(0);
	std::this_thread::sleep_for(std::chrono::milliseconds(500000));

	return 0;
}
