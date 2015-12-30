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

#include <Windows.h>

using namespace aud;

void loadSounds(const std::string& path, const std::string& ext, std::vector<std::shared_ptr<ISound>>& outList);
int main(int argc, char* argv[])
{
	srand(time(NULL));

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();

	auto fftPlan(std::make_shared<FFTPlan>(2048, true));
	auto threadPool(std::make_shared<ThreadPool>(std::thread::hardware_concurrency()));
	auto hrtfs = HRTFLoader::loadRightHRTFs(fftPlan, ".wav", "hrtfs");
	auto i_ir(std::make_shared<ImpulseResponse>(std::make_shared<StreamBuffer>(std::make_shared<File>("Opti-inverse.wav")), fftPlan));
	auto source1 = std::make_shared<Source>(210, 0, 0.5);
	auto source2 = std::make_shared<Source>(190, 0, 0.5);
	auto source3 = std::make_shared<Source>(200, 0, 0.5);
	auto sourceBlaster = std::make_shared<Source>(10, 0, 0.8);
	std::vector<std::shared_ptr<Source>> sourceBullets;
	sourceBullets.push_back(std::make_shared<Source>(10, 0, 0.9));
	sourceBullets.push_back(std::make_shared<Source>(10, 0, 0.9));
	sourceBullets.push_back(std::make_shared<Source>(10, 0, 0.9));
	sourceBullets.push_back(std::make_shared<Source>(10, 0, 0.9));
	sourceBullets.push_back(std::make_shared<Source>(10, 0, 0.9));

	DeviceSpecs specs;
	specs.channels = CHANNELS_MONO;
	specs.rate = hrtfs->getSpecs().rate;

	std::vector<std::shared_ptr<ISound>> saberClash;
	std::vector<std::shared_ptr<ISound>> saberWield;
	loadSounds("sw/saberClash", ".wav", saberClash);
	loadSounds("sw/saberWield", ".wav", saberWield);
	auto saberSList = std::make_shared<SoundList>(true);
	for(auto s : saberClash)
	{
		saberSList->addSound(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(s, specs), hrtfs, source3, threadPool, fftPlan));
	}
	int i = 0;
	for(auto s : saberWield)
	{
		if(i % 2 == 0)
			saberSList->addSound(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(s, specs), hrtfs, source1, threadPool, fftPlan));
		if(i % 2 != 0)
			saberSList->addSound(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(s, specs), hrtfs, source2, threadPool, fftPlan));
		i++;
	}
	auto saberMutableSound = std::make_shared<ConvolverSound>(std::make_shared<MutableSound>(saberSList), i_ir, threadPool, fftPlan);
	auto blasterShotSound = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<JOSResample>(std::make_shared<File>("sw/blaster/repeat-1.wav"), specs), hrtfs, sourceBlaster, threadPool, fftPlan), i_ir, threadPool, fftPlan);

	device->lock();
	auto handleSaber = device->play(saberMutableSound);
	handleSaber->setLoopCount(-1);
	device->unlock();

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	int millis = 0;
	for(int i = 0; i < 1000; i++)
	{
		auto handleBlaster = device->play(blasterShotSound);
		millis = (rand() % 20) + 140;
		std::this_thread::sleep_for(std::chrono::milliseconds(millis));
		if(i % 5 == 0 && i != 0)
		{
			millis = (rand() % 160) + 140;
			std::this_thread::sleep_for(std::chrono::milliseconds(millis));
		}
	}
	//DynamicMusic dynamicSound(device);
	//int saberScene1 = dynamicSound.addScene(clashMutableSound);
	//int saberScene2 = dynamicSound.addScene(clashMutableSound);
	//dynamicSound.addTransition(saberScene1, saberScene2, wieldMutableSound);
	//dynamicSound.addTransition(saberScene2, saberScene1, wieldMutableSound);

	//int millis = 0;
	//while(true)
	//{
	//	dynamicSound.changeScene(saberScene1);
	//	millis = (rand() % 5000) + 5000;
	//	std::this_thread::sleep_for(std::chrono::milliseconds(millis));
	//	//dynamicSound.changeScene(saberScene2);
	//	millis = (rand() % 5000) + 5000;
	//	//std::this_thread::sleep_for(std::chrono::milliseconds(millis));
	//}


	std::this_thread::sleep_for(std::chrono::milliseconds(500000));

	return 0;
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
