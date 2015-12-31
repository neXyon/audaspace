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
#include "fx/ImpulseResponse.h"
#include "util/ThreadPool.h"
#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "respec/ChannelMapper.h"
#include "respec/JOSResample.h"
#include "file/File.h"
#include "fx/DynamicMusic.h"
#include "fx/SoundList.h"
#include "fx/MutableSound.h"
#include "fx/HRTFLoader.h"
#include "fx/BinauralSound.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <cmath>

#include <Windows.h>

#define PI 3.14159265

using namespace aud;

void loadSounds(const std::string& path, const std::string& ext, std::vector<std::shared_ptr<ISound>>& outList);
bool playSaber(std::shared_ptr<IDevice> device, std::shared_ptr<ISound> sound, std::shared_ptr<Source> source1, std::shared_ptr<Source> source2, std::shared_ptr<Source> source3);
int main(int argc, char* argv[])
{
	srand(time(NULL));

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();

	auto fftPlan(std::make_shared<FFTPlan>(2048, true));
	auto threadPool(std::make_shared<ThreadPool>(std::thread::hardware_concurrency()));
	auto hrtfs = HRTFLoader::loadRightHRTFs(fftPlan, ".wav", "hrtfs");
	auto i_ir(std::make_shared<ImpulseResponse>(std::make_shared<StreamBuffer>(std::make_shared<File>("In The Silo Revised.wav")), fftPlan));
	DeviceSpecs specs;
	specs.channels = CHANNELS_MONO;
	specs.rate = hrtfs->getSpecs().rate;

	auto sourceDoor = std::make_shared<Source>(150, 0, 0.95);
	auto sourcePeople = std::make_shared<Source>(150, 0, 0.95);
	auto sourcePerson = std::make_shared<Source>(150, -40, 0.85);

	auto soundCreackDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/creaking-door-2.wav"), specs), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundOpenDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/door-2-open.wav"), specs), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundCloseDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/door-2-close.wav"), specs), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundPeople = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/crowd-talking-1.wav"), specs), specs), hrtfs, sourcePeople, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundPersonSteps = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/footsteps-1.wav"), specs), specs), hrtfs, sourcePerson, threadPool, fftPlan), i_ir, threadPool, fftPlan);

	std::shared_ptr<IHandle> handle1;
	std::shared_ptr<IHandle> handle2;
	std::shared_ptr<IHandle> handle3;

	//Person enters scene
	handle1= device->play(soundOpenDoor);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	handle1 = device->play(soundPersonSteps);
	handle2 = device->play(soundCreackDoor);
	device->lock();
	handle3 = device->play(soundPeople);
	handle3->setVolume(0.05);
	handle3->setLoopCount(-1);
	device->unlock();
	for(int i = 0;i < 10;i++)
	{
		float volume = handle3->getVolume();
		volume += 0.1;
		if(volume > 1)
			volume = 1.0;
		handle3->setVolume(volume);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}
	handle1->stop();
	handle2 = device->play(soundCreackDoor);
	for(int i = 0;i < 10;i++)
	{
		float volume = handle3->getVolume();
		volume -= 0.1;
		if(volume < 0.05)
			volume = 0.05;
		handle3->setVolume(volume);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}
	handle1 = device->play(soundCloseDoor);
	std::this_thread::sleep_for(std::chrono::milliseconds(700));
	//Person moves
	handle1 = device->play(soundPersonSteps);
	for(int i = 0;i < 50;i++)
	{
		sourcePerson->setAzimuth(sourcePerson->getAzimuth() - 2.8);
		sourcePerson->setDistance(0.4 / sin(sourcePerson->getAzimuth()*PI / 180.0));
		if(sourcePerson->getDistance() > 0.85)
			sourcePerson->setDistance(0.85);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	handle1->stop();
	//Person does things
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	//Person moves
	handle1 = device->play(soundPersonSteps);
	for(int i = 0;i < 40;i++)
	{
		sourcePerson->setAzimuth(sourcePerson->getAzimuth() - 2.25);
		sourcePerson->setDistance(0.4 / abs(cos((sourcePerson->getAzimuth()+45)*PI / 180.0)));
		if(sourcePerson->getDistance() > 0.85)
			sourcePerson->setDistance(0.85);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	handle1->stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(500000));
	return 0;
}

bool playSaber(std::shared_ptr<IDevice> device, std::shared_ptr<ISound> sound, std::shared_ptr<Source> source1, std::shared_ptr<Source> source2, std::shared_ptr<Source> source3)
{
	static std::shared_ptr<IHandle> handle=nullptr;
	static float inc = 1;
	if(handle != nullptr)
	{
		if(source2->getAzimuth() == 50)
			inc = -1;
		if(source2->getAzimuth() == 310)
			inc = 1;
		if(handle->getPosition() >= 9.643)
			handle=device->play(sound);
		source1->setAzimuth(source1->getAzimuth() + inc);
		source2->setAzimuth(source2->getAzimuth() + inc);
		source3->setAzimuth(source3->getAzimuth() + inc);
		source1->setDistance(10 / cos(source1->getAzimuth()*PI / 180.0)-9.70);
		source2->setDistance(0.5 / cos(source2->getAzimuth()*PI / 180.0));
		source3->setDistance(10 / cos(source3->getAzimuth()*PI / 180.0)-9.70);
	}
	else
		handle = device->play(sound);
	return false;
}

void loadSounds(const std::string& path, const std::string& ext, std::vector<std::shared_ptr<ISound>>& outList)
{
	std::string readpath = path;
	if(path == "")
		readpath = ".";

	WIN32_FIND_DATA entry;
	bool found_file = true;
	std::string search = readpath + "\\*";
	HANDLE dir = FindFirstFile(search.c_str(), &entry);
	if(dir == INVALID_HANDLE_VALUE)
		return;

	float azim, elev;

	while(found_file)
	{
		std::string filename = entry.cFileName;
		if(filename.length() >= ext.length() && filename.substr(filename.length() - ext.length()) == ext)
			outList.push_back(std::make_shared<File>(readpath + "/" + filename));
		found_file = FindNextFile(dir, &entry);
	}
	FindClose(dir);
	return;
}
