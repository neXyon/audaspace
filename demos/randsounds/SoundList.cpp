#include "SoundList.h"

#include <chrono>

SoundList::SoundList(std::shared_ptr<IDevice> device){
	this->_device = device;
	_currentSound = 0;
	srand(time(NULL));
}

void SoundList::addSound(std::shared_ptr<File> sound){
	this->_list.push_back(sound);
	_currentSound = rand() % getNumberOfSounds();
}

std::shared_ptr<IHandle> SoundList::play(int num){ 
	auto changeSound = [](void* current){
		int num = 0;
		do{
			num = rand() % ((SoundList*)current)->getNumberOfSounds();
		} while (num == ((SoundList*)current)->getCurrentSound());
		((SoundList*)current)->play(num);
	};
	_currentSound = num;
	_device->lock();
	auto handle = _device->play(_list[_currentSound]);
	handle->setStopCallback(changeSound, this);
	_device->unlock();
	return handle;
}

std::shared_ptr<IHandle> SoundList::play(){
	return play(_currentSound);
}

int SoundList::getCurrentSound(){
	return _currentSound;
}

void SoundList::setCurrentSound(int num){
	_currentSound = num;
}

int SoundList::getNumberOfSounds(){
	return _list.size();
}