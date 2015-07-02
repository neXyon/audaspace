/*******************************************************************************
* Copyright 2009-2015 Jörg Müller
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

#include "SoundList.h"

using namespace aud;

int main(int argc, char* argv[])
{
	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDefaultDeviceFactory();
	auto device = factory->openDevice();

	SoundList slist(device);

	std::shared_ptr<File> file;
	file.reset(new File("a.ogg"));
	slist.addSound(file);
	file.reset(new File("b.ogg"));
	slist.addSound(file);
	file.reset(new File("c.ogg"));
	slist.addSound(file);
	file.reset(new File("d.ogg"));
	slist.addSound(file);

	slist.play(0);
	slist.play(3);

	std::condition_variable condition;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);

	condition.wait(lock);

	return 0;
}
