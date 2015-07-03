#include "SoundList.h"

#include "devices/IDeviceFactory.h"
#include "devices/DeviceManager.h"
#include "plugin/PluginManager.h"

#include <iostream>
#include <condition_variable>
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

	SoundList slist(device);

	std::shared_ptr<File> file;
	for (int i = 1; i < argc; i++){
		file.reset(new File(argv[i]));
		slist.addSound(file);
	}

	slist.play();
	slist.play();

	std::condition_variable condition;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);

	condition.wait(lock);

	return 0;
}
