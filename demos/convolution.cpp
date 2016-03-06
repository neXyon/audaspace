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

#include "fx/ConvolverSound.h"
#include "fx/ImpulseResponse.h"
#include "util/ThreadPool.h"
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
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <sound>" << " <impulse response>" << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();

	std::shared_ptr<FFTPlan> plan(std::make_shared<FFTPlan>(4096, 1.0));
	std::shared_ptr<ThreadPool> threadPool(std::make_shared<ThreadPool>(std::thread::hardware_concurrency()));
	std::shared_ptr<File> file1(std::make_shared<File>(argv[1]));
	std::shared_ptr<File> file2(std::make_shared<File>(argv[2]));
	std::shared_ptr<ImpulseResponse> impulseResponse(std::make_shared<ImpulseResponse>(std::make_shared<StreamBuffer>(file2), plan));
	std::shared_ptr<ConvolverSound> convolver(std::make_shared<ConvolverSound>(file1, impulseResponse, threadPool, plan));

	device->lock();	
	auto handle = device->play(convolver);
	handle->setVolume(0.2);
	handle->setLoopCount(-1);	
	device->unlock();

	std::this_thread::sleep_for(std::chrono::milliseconds(500000));

	return 0;
}
