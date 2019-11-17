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

#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "file/File.h"
#include "Exception.h"
#include "IReader.h"

#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>

using namespace aud;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	DeviceSpecs specs;
	specs.format = FORMAT_FLOAT32;

	File file(argv[1]);
	std::shared_ptr<IReader> reader;

	try
	{
		reader = file.createReader();
	}
	catch(Exception& e)
	{
		std::cerr << "Error opening file " << argv[1] << " - " << e.getMessage() << std::endl;
		return 2;
	}

	specs.specs = reader->getSpecs();

	auto factory = DeviceManager::getDefaultDeviceFactory();
	factory->setSpecs(specs);
	auto device = factory->openDevice();

	auto duration = std::chrono::seconds(reader->getLength()) / specs.rate;
	std::cout << "Estimated duration: " << duration.count() << " seconds" << std::endl;

	std::condition_variable condition;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);

	auto release = [](void* condition){reinterpret_cast<std::condition_variable*>(condition)->notify_all();};

	device->lock();
	auto handle = device->play(reader);

	if(!handle)
	{
		device->unlock();

		std::cout << "Device could not play the file." << std::endl;

		return 1;
	}

	handle->setStopCallback(release, &condition);
	device->unlock();

	condition.wait(lock);

	return 0;
}
