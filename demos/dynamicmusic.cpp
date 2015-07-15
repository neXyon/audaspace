#include "fx/SoundList.h"
#include "fx/DynamicMusicPlayer.h"
#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "file/File.h"
#include "Exception.h"
#include "IReader.h"
#include "fx/MutableSound.h"

#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <mutex>

using namespace aud;

int main(int argc, char* argv[])
{
	if (argc == 1){
		std::cerr << "Usage: " << argv[0] << " <filename>" << " <filename>" << " ..." << std::endl;
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
		manager.addScene(file);
	}
	file.reset(new File("effect.ogg"));
	manager.addTransition(2, 1, file);
	manager.setFadeTime(2.0f);

	std::condition_variable condition;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);

	auto release = [](void* condition){reinterpret_cast<std::condition_variable*>(condition)->notify_all(); };

	manager.changeScene(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	manager.changeScene(2);
	std::this_thread::sleep_for(std::chrono::milliseconds(20000));
	manager.changeScene(1);

	condition.wait(lock);

	return 0;
}