/*******************************************************************************
* Copyright 2009-2015 Juan Francisco Crespo Galán
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
#include "fx/BinauralSound.h"
#include "fx/Source.h"
#include "fx/HRTF.h"
#include "util/ThreadPool.h"
#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "file/File.h"
#include "Exception.h"

#include <windows.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

using namespace aud;

void loadHRTFs(std::string path, std::shared_ptr<HRTF> hrtfs);

int main(int argc, char* argv[])
{
	/*This demo uses KEMAR HRTFs.*/
	if(argc != 3 && argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " <sound>"  << " <HRTFs path>" << " [inverse speaker impulse response]" << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();

	std::shared_ptr<FFTPlan> plan(std::make_shared<FFTPlan>(4096, true));
	std::shared_ptr<ThreadPool> threadPool(std::make_shared<ThreadPool>(std::thread::hardware_concurrency()));
	std::shared_ptr<File> file1(std::make_shared<File>(argv[1]));
	std::shared_ptr<HRTF> hrtfs(std::make_shared<HRTF>(plan));
	std::shared_ptr<Source> source = std::make_shared<Source>(0, 0);

	loadHRTFs(argv[2], hrtfs);

	std::shared_ptr<BinauralSound> binaural(std::make_shared<BinauralSound>(file1, hrtfs, source, threadPool, plan));

	device->lock();
	if (argc == 4)
	{
		std::shared_ptr<ImpulseResponse> ir = std::make_shared<ImpulseResponse>(std::make_shared<StreamBuffer>(std::make_shared<File>(argv[3])), plan);
		std::shared_ptr<ConvolverSound> convolver = std::make_shared<ConvolverSound>(binaural, ir, threadPool, plan);
		auto handle = device->play(convolver);
		handle->setLoopCount(-1);
	}
	else
	{
		auto handle = device->play(binaural);
		handle->setLoopCount(-1);
	}
	device->unlock();

	float x = 0;
	float y = 0;
	float yInc = 5;
	while(true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		x += 15;
		y += yInc;
		if(y >= 60)
		{
			y = 60;
			yInc = -5;
		}
		else if(y <= -40)
		{
			y = -40;
			yInc = 5;
		}
		if(x >= 360)
			x = 0;

		source->setAzimuth(x);
		source->setElevation(y);
		std::cout << " Azimuth: " << x << " - Elevation: " << y << std::endl;
	}

	return 0;
}

void loadHRTFs(std::string path, std::shared_ptr<HRTF> hrtfs)
{
	std::string readpath = path;

	WIN32_FIND_DATA entry;
	bool found_file = true;
	std::string search = readpath + "\\*";
	HANDLE dir = FindFirstFile(search.c_str(), &entry);
	float azim, elev;

	while(found_file)
	{
		std::string filename = entry.cFileName;
		
		if (filename.front() == 'R')
		{
			elev = std::stof(filename.substr(1, filename.find("e") - 1));
			azim = std::stof(filename.substr(filename.find("e") + 1, filename.find("a") - filename.find("e") - 1));
			hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>(path + "/" + filename)), azim, elev);
		}
		found_file = FindNextFile(dir, &entry);
	}
	FindClose(dir);
}
