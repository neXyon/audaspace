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

	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e000a.wav")), 0, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e355a.wav")), 5, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e350a.wav")), 10, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e345a.wav")), 15, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e340a.wav")), 20, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e335a.wav")), 25, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e330a.wav")), 30, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e325a.wav")), 35, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e320a.wav")), 40, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e315a.wav")), 45, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e310a.wav")), 50, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e305a.wav")), 55, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e300a.wav")), 60, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e295a.wav")), 65, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e290a.wav")), 70, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e285a.wav")), 75, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e280a.wav")), 80, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e275a.wav")), 85, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e270a.wav")), 90, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e265a.wav")), 95, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e260a.wav")), 100, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e255a.wav")), 105, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e250a.wav")), 110, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e245a.wav")), 115, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e240a.wav")), 120, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e235a.wav")), 125, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e230a.wav")), 130, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e225a.wav")), 135, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e220a.wav")), 140, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e215a.wav")), 145, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e210a.wav")), 150, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e205a.wav")), 155, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e200a.wav")), 160, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e195a.wav")), 165, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e190a.wav")), 170, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e185a.wav")), 175, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e180a.wav")), 180, 0);

	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e175a.wav")), 185, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e170a.wav")), 190, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e165a.wav")), 195, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e160a.wav")), 200, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e155a.wav")), 205, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e150a.wav")), 210, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e145a.wav")), 215, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e140a.wav")), 220, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e135a.wav")), 225, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e130a.wav")), 230, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e125a.wav")), 235, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e120a.wav")), 240, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e115a.wav")), 245, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e110a.wav")), 250, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e105a.wav")), 255, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e100a.wav")), 260, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e095a.wav")), 265, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e090a.wav")), 270, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e085a.wav")), 275, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e080a.wav")), 280, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e075a.wav")), 285, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e070a.wav")), 290, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e065a.wav")), 295, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e060a.wav")), 300, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e055a.wav")), 305, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e050a.wav")), 310, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e045a.wav")), 315, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e040a.wav")), 320, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e035a.wav")), 325, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e030a.wav")), 330, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e025a.wav")), 335, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e020a.wav")), 340, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e015a.wav")), 345, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e010a.wav")), 350, 0);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev0/L0e005a.wav")), 355, 0);

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
		if (x >= 360)
			x = 0;
		source->setAzimuth(x);
	}

	return 0;
}
