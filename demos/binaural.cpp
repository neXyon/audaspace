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
#include "util/StreamBuffer.h"
#include "util/ThreadPool.h"
#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "file/File.h"
#include "IReader.h"
#include "Exception.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace aud;

void loadHRTFs(std::shared_ptr<HRTF> hrtfs);

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <sound>"  << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();

	std::shared_ptr<FFTPlan> plan(std::make_shared<FFTPlan>(4096, true));
	std::shared_ptr<ThreadPool> threadPool(std::make_shared<ThreadPool>(std::thread::hardware_concurrency()));
	std::shared_ptr<File> file1(std::make_shared<File>(argv[1]));
	std::shared_ptr<HRTF> hrtfs(std::make_shared<HRTF>(plan));
	std::shared_ptr<Source> source = std::make_shared<Source>(0, 40);

	loadHRTFs(hrtfs);

	std::shared_ptr<BinauralSound> binaural(std::make_shared<BinauralSound>(file1, hrtfs, source, threadPool, plan));
	std::shared_ptr<ImpulseResponse> ir = std::make_shared<ImpulseResponse>(std::make_shared<StreamBuffer>(std::make_shared<File>("full/headphones+spkr/Opti-inverse.wav")));
	std::shared_ptr<ConvolverSound> convolver = std::make_shared<ConvolverSound>(binaural, ir, threadPool, plan);

	device->lock();
	auto handle = device->play(convolver);
	handle->setLoopCount(-1);
	device->unlock();

	float x = 0;
	float y = 0;
	float yInc = 1;
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		x += 3;
		y += yInc;
		if (y >= 80)
		{
			y = 80;
			yInc = -1;
		}
		else if (y <= -40)
		{
			y = -40;
			yInc = 1;
		}
		if (x >= 360)
			x = 0;
		source->setAzimuth(x);
		source->setElevation(y);
	}

	return 0;
}

void loadHRTFs(std::shared_ptr<HRTF> hrtfs)
{
	std::stringstream ss;

	int step = 5;
	int az = 0; 
	int azF = 355;
	while (azF >= 0)
	{
		ss << std::setw(3) << std::setfill('0') << azF;
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e" + ss.str() + "a.wav")), az, 0);
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e" + ss.str() + "a.wav")), az, 10);
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e" + ss.str() + "a.wav")), az, -10);
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e" + ss.str() + "a.wav")), az, 20);
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e" + ss.str() + "a.wav")), az, -20);
		az += step;
		azF -= step;
		ss.str("");
		ss.clear();
	}

	step = 6;
	az = 0;
	azF = 354;
	while (azF >= 0)
	{
		ss << std::setw(3) << std::setfill('0') << azF;
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev30/L30e" + ss.str() + "a.wav")), az, 30);
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-30/L-30e" + ss.str() + "a.wav")), az, -30);
		az += step;
		azF -= step;
		ss.str("");
		ss.clear();
	}

	step = 1;
	az = 0;
	azF = 360;
	while (azF >= 0)
	{
		ss << std::setw(3) << std::setfill('0') << azF;
		try
		{
			hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev40/L40e" + ss.str() + "a.wav")), az, 40);
			hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-40/L-40e" + ss.str() + "a.wav")), az, -40);
		}
		catch (Exception& e)
		{
		}

		az += step;
		azF -= step;
		ss.str("");
		ss.clear();
	}

	step = 8;
	az = 0;
	azF = 352;
	while (azF >= 0)
	{
		ss << std::setw(3) << std::setfill('0') << azF;
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev50/L50e" + ss.str() + "a.wav")), az, 50);
		az += step;
		azF -= step;
		ss.str("");
		ss.clear();
	}

	step = 10;
	az = 0;
	azF = 350;
	while (azF >= 0)
	{
		ss << std::setw(3) << std::setfill('0') << azF;
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev60/L60e" + ss.str() + "a.wav")), az, 60);
		az += step;
		azF -= step;
		ss.str("");
		ss.clear();
	}

	step = 15;
	az = 0;
	azF = 345;
	while (azF >= 0)
	{
		ss << std::setw(3) << std::setfill('0') << azF;
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev70/L70e" + ss.str() + "a.wav")), az, 70);
		az += step;
		azF -= step;
		ss.str("");
		ss.clear();
	}

	step = 30;
	az = 0;
	azF = 330;
	while (azF >= 0)
	{
		ss << std::setw(3) << std::setfill('0') << azF;
		hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev80/L80e" + ss.str() + "a.wav")), az, 80);
		az += step;
		azF -= step;
		ss.str("");
		ss.clear();
	}

	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev90/L90e000a.wav")), 0, 90);
}
