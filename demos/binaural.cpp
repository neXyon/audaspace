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
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e005a.wav")), 5, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e010a.wav")), 10, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e015a.wav")), 15, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e020a.wav")), 20, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e025a.wav")), 25, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e030a.wav")), 30, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e035a.wav")), 35, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e040a.wav")), 40, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e045a.wav")), 45, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e050a.wav")), 50, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e055a.wav")), 55, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e060a.wav")), 60, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e065a.wav")), 65, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e070a.wav")), 70, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e075a.wav")), 75, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e080a.wav")), 80, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e085a.wav")), 85, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e090a.wav")), 90, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e095a.wav")), 95, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e100a.wav")), 100, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e105a.wav")), 105, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e110a.wav")), 110, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e115a.wav")), 115, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e120a.wav")), 120, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e125a.wav")), 125, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e130a.wav")), 130, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e135a.wav")), 135, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e140a.wav")), 140, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e145a.wav")), 145, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e150a.wav")), 150, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e155a.wav")), 155, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e160a.wav")), 160, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e165a.wav")), 165, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e170a.wav")), 170, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e175a.wav")), 175, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("compact/elev0/H0e180a.wav")), 180, 0);

	std::shared_ptr<BinauralSound> convolver(std::make_shared<BinauralSound>(file1, hrtfs, source, threadPool, plan));

	device->lock();
	auto handle = device->play(convolver);
	handle->setLoopCount(-1);
	device->unlock();

	float x = 0;
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		x += 3;
		if (x > 180)
			return 0;
		source->setAzimuth(x);
	}

	return 0;
}
