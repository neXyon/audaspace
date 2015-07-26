#include "fx/DynamicMusicPlayer.h"
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
	if (argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <scene 1>" << " <scene 2>" << " [<transition 1-2>]" << std::endl;
		return 1;
	}

	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();
	DynamicMusicPlayer manager (device);
	std::shared_ptr<File> file;
	for (int i = 1; i < argc; i++)
	{
		file = (std::make_shared<File>(argv[i]));
		if (i == 3)
			manager.addTransition(2, 1, file);
		else
			manager.addScene(file);
	}
	manager.setFadeTime(2.0f);
	manager.setVolume(0.5f);
	manager.changeScene(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.changeScene(2);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.changeScene(0);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.changeScene(2);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.changeScene(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(500000));

	return 0;
}
