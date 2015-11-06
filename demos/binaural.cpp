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

#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <mutex>

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
	std::shared_ptr<Source> source = std::make_shared<Source>(30, 0);

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
		if (y >= 20)
		{
			y = 20;
			yInc = -1;
		}
		else if (y <= -20)
		{
			y = -20;
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

	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e000a.wav")), 0, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e355a.wav")), 5, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e350a.wav")), 10, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e345a.wav")), 15, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e340a.wav")), 20, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e335a.wav")), 25, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e330a.wav")), 30, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e325a.wav")), 35, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e320a.wav")), 40, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e315a.wav")), 45, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e310a.wav")), 50, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e305a.wav")), 55, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e300a.wav")), 60, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e295a.wav")), 65, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e290a.wav")), 70, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e285a.wav")), 75, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e280a.wav")), 80, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e275a.wav")), 85, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e270a.wav")), 90, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e265a.wav")), 95, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e260a.wav")), 100, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e255a.wav")), 105, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e250a.wav")), 110, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e245a.wav")), 115, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e240a.wav")), 120, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e235a.wav")), 125, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e230a.wav")), 130, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e225a.wav")), 135, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e220a.wav")), 140, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e215a.wav")), 145, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e210a.wav")), 150, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e205a.wav")), 155, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e200a.wav")), 160, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e195a.wav")), 165, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e190a.wav")), 170, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e185a.wav")), 175, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e180a.wav")), 180, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e175a.wav")), 185, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e170a.wav")), 190, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e165a.wav")), 195, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e160a.wav")), 200, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e155a.wav")), 205, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e150a.wav")), 210, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e145a.wav")), 215, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e140a.wav")), 220, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e135a.wav")), 225, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e130a.wav")), 230, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e125a.wav")), 235, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e120a.wav")), 240, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e115a.wav")), 245, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e110a.wav")), 250, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e105a.wav")), 255, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e100a.wav")), 260, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e095a.wav")), 265, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e090a.wav")), 270, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e085a.wav")), 275, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e080a.wav")), 280, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e075a.wav")), 285, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e070a.wav")), 290, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e065a.wav")), 295, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e060a.wav")), 300, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e055a.wav")), 305, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e050a.wav")), 310, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e045a.wav")), 315, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e040a.wav")), 320, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e035a.wav")), 325, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e030a.wav")), 330, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e025a.wav")), 335, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e020a.wav")), 340, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e015a.wav")), 345, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e010a.wav")), 350, 20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev20/L20e005a.wav")), 355, 20);

	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e000a.wav")), 0, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e355a.wav")), 5, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e350a.wav")), 10, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e345a.wav")), 15, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e340a.wav")), 20, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e335a.wav")), 25, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e330a.wav")), 30, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e325a.wav")), 35, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e320a.wav")), 40, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e315a.wav")), 45, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e310a.wav")), 50, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e305a.wav")), 55, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e300a.wav")), 60, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e295a.wav")), 65, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e290a.wav")), 70, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e285a.wav")), 75, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e280a.wav")), 80, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e275a.wav")), 85, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e270a.wav")), 90, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e265a.wav")), 95, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e260a.wav")), 100, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e255a.wav")), 105, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e250a.wav")), 110, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e245a.wav")), 115, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e240a.wav")), 120, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e235a.wav")), 125, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e230a.wav")), 130, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e225a.wav")), 135, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e220a.wav")), 140, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e215a.wav")), 145, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e210a.wav")), 150, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e205a.wav")), 155, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e200a.wav")), 160, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e195a.wav")), 165, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e190a.wav")), 170, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e185a.wav")), 175, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e180a.wav")), 180, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e175a.wav")), 185, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e170a.wav")), 190, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e165a.wav")), 195, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e160a.wav")), 200, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e155a.wav")), 205, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e150a.wav")), 210, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e145a.wav")), 215, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e140a.wav")), 220, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e135a.wav")), 225, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e130a.wav")), 230, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e125a.wav")), 235, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e120a.wav")), 240, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e115a.wav")), 245, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e110a.wav")), 250, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e105a.wav")), 255, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e100a.wav")), 260, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e095a.wav")), 265, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e090a.wav")), 270, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e085a.wav")), 275, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e080a.wav")), 280, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e075a.wav")), 285, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e070a.wav")), 290, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e065a.wav")), 295, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e060a.wav")), 300, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e055a.wav")), 305, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e050a.wav")), 310, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e045a.wav")), 315, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e040a.wav")), 320, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e035a.wav")), 325, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e030a.wav")), 330, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e025a.wav")), 335, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e020a.wav")), 340, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e015a.wav")), 345, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e010a.wav")), 350, 10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev10/L10e005a.wav")), 355, 10);

	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e000a.wav")), 0, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e355a.wav")), 5, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e350a.wav")), 10, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e345a.wav")), 15, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e340a.wav")), 20, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e335a.wav")), 25, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e330a.wav")), 30, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e325a.wav")), 35, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e320a.wav")), 40, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e315a.wav")), 45, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e310a.wav")), 50, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e305a.wav")), 55, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e300a.wav")), 60, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e295a.wav")), 65, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e290a.wav")), 70, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e285a.wav")), 75, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e280a.wav")), 80, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e275a.wav")), 85, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e270a.wav")), 90, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e265a.wav")), 95, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e260a.wav")), 100, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e255a.wav")), 105, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e250a.wav")), 110, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e245a.wav")), 115, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e240a.wav")), 120, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e235a.wav")), 125, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e230a.wav")), 130, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e225a.wav")), 135, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e220a.wav")), 140, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e215a.wav")), 145, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e210a.wav")), 150, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e205a.wav")), 155, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e200a.wav")), 160, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e195a.wav")), 165, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e190a.wav")), 170, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e185a.wav")), 175, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e180a.wav")), 180, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e175a.wav")), 185, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e170a.wav")), 190, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e165a.wav")), 195, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e160a.wav")), 200, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e155a.wav")), 205, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e150a.wav")), 210, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e145a.wav")), 215, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e140a.wav")), 220, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e135a.wav")), 225, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e130a.wav")), 230, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e125a.wav")), 235, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e120a.wav")), 240, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e115a.wav")), 245, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e110a.wav")), 250, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e105a.wav")), 255, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e100a.wav")), 260, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e095a.wav")), 265, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e090a.wav")), 270, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e085a.wav")), 275, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e080a.wav")), 280, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e075a.wav")), 285, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e070a.wav")), 290, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e065a.wav")), 295, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e060a.wav")), 300, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e055a.wav")), 305, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e050a.wav")), 310, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e045a.wav")), 315, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e040a.wav")), 320, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e035a.wav")), 325, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e030a.wav")), 330, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e025a.wav")), 335, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e020a.wav")), 340, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e015a.wav")), 345, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e010a.wav")), 350, -20);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-20/L-20e005a.wav")), 355, -20);

	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e000a.wav")), 0, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e355a.wav")), 5, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e350a.wav")), 10, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e345a.wav")), 15, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e340a.wav")), 20, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e335a.wav")), 25, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e330a.wav")), 30, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e325a.wav")), 35, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e320a.wav")), 40, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e315a.wav")), 45, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e310a.wav")), 50, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e305a.wav")), 55, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e300a.wav")), 60, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e295a.wav")), 65, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e290a.wav")), 70, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e285a.wav")), 75, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e280a.wav")), 80, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e275a.wav")), 85, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e270a.wav")), 90, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e265a.wav")), 95, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e260a.wav")), 100, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e255a.wav")), 105, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e250a.wav")), 110, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e245a.wav")), 115, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e240a.wav")), 120, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e235a.wav")), 125, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e230a.wav")), 130, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e225a.wav")), 135, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e220a.wav")), 140, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e215a.wav")), 145, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e210a.wav")), 150, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e205a.wav")), 155, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e200a.wav")), 160, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e195a.wav")), 165, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e190a.wav")), 170, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e185a.wav")), 175, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e180a.wav")), 180, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e175a.wav")), 185, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e170a.wav")), 190, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e165a.wav")), 195, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e160a.wav")), 200, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e155a.wav")), 205, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e150a.wav")), 210, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e145a.wav")), 215, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e140a.wav")), 220, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e135a.wav")), 225, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e130a.wav")), 230, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e125a.wav")), 235, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e120a.wav")), 240, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e115a.wav")), 245, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e110a.wav")), 250, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e105a.wav")), 255, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e100a.wav")), 260, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e095a.wav")), 265, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e090a.wav")), 270, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e085a.wav")), 275, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e080a.wav")), 280, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e075a.wav")), 285, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e070a.wav")), 290, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e065a.wav")), 295, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e060a.wav")), 300, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e055a.wav")), 305, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e050a.wav")), 310, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e045a.wav")), 315, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e040a.wav")), 320, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e035a.wav")), 325, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e030a.wav")), 330, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e025a.wav")), 335, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e020a.wav")), 340, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e015a.wav")), 345, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e010a.wav")), 350, -10);
	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev-10/L-10e005a.wav")), 355, -10);

	hrtfs->addImpulseResponse(std::make_shared<StreamBuffer>(std::make_shared<File>("full/elev90/L90e000a.wav")), 0, 90);
}
