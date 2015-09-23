#include "fx/PlaybackManager.h"
#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "file/File.h"

#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <mutex>

using namespace aud;

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		std::cerr << "Usage: " << argv[0] << " <sound 1 category 1>" << " <sound 2 category 1>" << " <sound 1 category 2>" << " <sound 2 category 2>" << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();
	PlaybackManager manager(device);
	std::shared_ptr<File> file;
	for (int i = 1; i < argc; i++)
	{
		file = (std::make_shared<File>(argv[i]));
		if (i <= 2)
			manager.play(file, 0);
		else
			manager.play(file, 1);
	}
	manager.setVolume(0.2f, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.setVolume(0.0f, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.pause(0);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.setVolume(0.5f, 1);
	manager.setVolume(1.0f, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.stop(1);
	manager.resume(0);
	std::this_thread::sleep_for(std::chrono::milliseconds(500000));

	return 0;
}
