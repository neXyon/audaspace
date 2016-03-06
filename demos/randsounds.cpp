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

#include "fx/SoundList.h"
#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "file/File.h"
#include "fx/MutableSound.h"

#include <iostream>
#include <condition_variable>
#include <mutex>

using namespace aud;

int main(int argc, char* argv[])
{
	if(argc == 1){
		std::cerr << "Usage: " << argv[0] << " <filename>" << " <filename>" << " ..." << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();

	std::shared_ptr<SoundList> list(std::make_shared<SoundList>());
	std::shared_ptr<File> file;
	for(int i = 1; i < argc; i++)
	{
		file = std::make_shared<File>(argv[i]);
		list->addSound(file);
	}
	list->setRandomMode(true);
	std::shared_ptr<MutableSound> mtSound = std::make_shared<MutableSound>(list);

	std::condition_variable condition;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);

	auto release = [](void* condition){reinterpret_cast<std::condition_variable*>(condition)->notify_all(); };

	device->lock(); 
	auto handle = device->play(mtSound);
	handle->setStopCallback(release, &condition);
	handle->setLoopCount(-1);
	device->unlock();

	condition.wait(lock);

	return 0;
}
