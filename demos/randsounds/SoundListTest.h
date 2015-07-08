# pragma once

#include "devices/IDevice.h"
#include "file/File.h"
#include "devices/IHandle.h"

#include <vector>
#include <unordered_map>
#include <mutex>

using namespace aud;

class SoundListTest{
private: std::vector<std::shared_ptr<File>> _list;
		 std::shared_ptr<IDevice> _device;
		 int _simulRepr;
		 std::unordered_map<int, std::shared_ptr<IHandle>> _handlers;
		 std::mutex _mutex;
public:
	SoundListTest(std::shared_ptr<IDevice> device);
	void addSound(std::shared_ptr<File> sound);
	int play();
	int play(int num);
	int play(int num, int id);
	int getNumberOfSounds();
};

typedef struct RandomPlayData{
	SoundListTest* sound;
	int id;
	int soundNumber;
} pDataStruct;