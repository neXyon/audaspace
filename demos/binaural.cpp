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

#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <mutex>

using namespace aud;

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
	std::shared_ptr<Source> source = std::make_shared<Source>(0, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e000a.wav")), 0, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e090a.wav")), 90, 0);
	std::shared_ptr<BinauralSound> convolver(std::make_shared<BinauralSound>(file1, hrtfs, source, threadPool, plan));

	device->lock();
	auto handle = device->play(convolver);
	handle->setVolume(0.2);
	handle->setLoopCount(-1);
	device->unlock();

	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	source->setAzimuth(90);

	std::this_thread::sleep_for(std::chrono::milliseconds(500000));

	return 0;
}
