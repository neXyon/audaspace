#include "SoundList.h"

#include <chrono>

SoundList::SoundList(std::shared_ptr<IDevice> device){
	this->_device = device;
	_simulRepr = 0;
	srand(time(NULL));
}

void SoundList::addSound(std::shared_ptr<File> sound){
	this->_list.push_back(sound);
}

int SoundList::play(int num, int id){
	auto changeSound = [](void* data){
		int num = 0;
		pDataStruct* datastruct = reinterpret_cast<pDataStruct*>(data);
		do{
			num = rand() % datastruct->sound->getNumberOfSounds();
		} while (num == datastruct->soundNumber);
		datastruct->sound->play(num, datastruct->id); 
		delete datastruct;
	};

	pDataStruct* data = new pDataStruct;
	data->sound = this;
	data->id = id;
	data->soundNumber = num;

	_device->lock();
	auto handle = _device->play(_list[num]);
	_handlers[id] = handle;
	handle->setStopCallback(changeSound, data);
	_device->unlock();

	return id;
}

int SoundList::play(int num){
	_simulRepr++;
	return play(num, _simulRepr);
}

int SoundList::play(){
	return play(rand() % getNumberOfSounds());
}

int SoundList::getNumberOfSounds(){
	return _list.size();
}