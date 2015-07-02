# pragma once

#include "devices/IDevice.h"
#include "devices/IHandle.h"
#include "file/File.h"

#include <vector>

using namespace aud;

class SoundList{
private: std::vector<std::shared_ptr<File>> _list;
		 std::shared_ptr<IDevice> _device;
		 int _currentSound;
public:
	SoundList(std::shared_ptr<IDevice> device);
	void addSound(std::shared_ptr<File> sound);
	std::shared_ptr<IHandle> play();
	std::shared_ptr<IHandle> play(int num);
	int getCurrentSound();
	void setCurrentSound(int num); 
	int getNumberOfSounds();
};