#include "fx/ConvolverSound.h"
#include "fx/ImpulseResponse.h"
#include "util/StreamBuffer.h"
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
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <sound>" << " <impulse response>" << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();

	std::shared_ptr<File> file1(std::make_shared<File>(argv[1]));
	std::shared_ptr<File> file2(std::make_shared<File>(argv[2]));
	std::shared_ptr<ImpulseResponse> impulseResponse(std::make_shared<ImpulseResponse>(std::make_shared<StreamBuffer>(file2)));
	std::shared_ptr<ConvolverSound> convolver(std::make_shared<ConvolverSound>(file1, impulseResponse, 1, 2));

	device->lock();
	auto handle = device->play(convolver);
	handle->setVolume(0.2);
	handle->setLoopCount(-1);
	device->unlock();

	std::this_thread::sleep_for(std::chrono::milliseconds(500000));

	return 0;
}
