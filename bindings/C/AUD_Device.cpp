/*******************************************************************************
 * Copyright 2009-2013 Jörg Müller
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

#include "devices/DeviceManager.h"
#include "devices/I3DDevice.h"
#include "devices/ReadDevice.h"
#include "Exception.h"

#include <cassert>

using namespace aud;

#define AUD_CAPI_IMPLEMENTATION
#include "AUD_Device.h"

void AUD_Device_lock(AUD_Device* device)
{
	auto dev = device ? *device : DeviceManager::getDevice();
	dev->lock();
}

AUD_Handle* AUD_Device_play(AUD_Device* device, AUD_Sound *sound, int keep)
{
	assert(sound);
	auto dev = device ? *device : DeviceManager::getDevice();

	try
	{
		AUD_Handle handle = dev->play(*sound, keep);
		if(handle.get())
		{
			return new AUD_Handle(handle);
		}
	}
	catch(Exception&)
	{
	}
	return nullptr;
}

void AUD_Device_stopAll(AUD_Device* device)
{
	auto dev = device ? *device : DeviceManager::getDevice();
	dev->stopAll();
}

void AUD_Device_unlock(AUD_Device* device)
{
	auto dev = device ? *device : DeviceManager::getDevice();
	dev->unlock();
}

AUD_Channels AUD_Device_getChannels(AUD_Device* device)
{
	auto dev = device ? *device : DeviceManager::getDevice();
	return static_cast<AUD_Channels>(dev->getSpecs().channels);
}

AUD_DistanceModel AUD_Device_getDistanceModel(AUD_Device* device)
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	return static_cast<AUD_DistanceModel>(dev->getDistanceModel());
}

void AUD_Device_setDistanceModel(AUD_Device* device, AUD_DistanceModel value)
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	dev->setDistanceModel(static_cast<DistanceModel>(value));
}

float AUD_Device_getDopplerFactor(AUD_Device* device)
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	return dev->getDopplerFactor();
}

void AUD_Device_setDopplerFactor(AUD_Device* device, float value)
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	dev->setDopplerFactor(value);
}

AUD_SampleFormat AUD_Device_getFormat(AUD_Device* device)
{
	auto dev = device ? *device : DeviceManager::getDevice();
	return static_cast<AUD_SampleFormat>(dev->getSpecs().format);
}

void AUD_Device_getListenerLocation(AUD_Device* device, float value[3])
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	Vector3 v = dev->getListenerLocation();
	value[0] = v.x();
	value[1] = v.y();
	value[2] = v.z();
}

void AUD_Device_setListenerLocation(AUD_Device* device, const float value[3])
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	Vector3 v(value[0], value[1], value[2]);
	dev->setListenerLocation(v);
}

void AUD_Device_getListenerOrientation(AUD_Device* device, float value[4])
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	Quaternion v = dev->getListenerOrientation();
	value[0] = v.x();
	value[1] = v.y();
	value[2] = v.z();
	value[3] = v.w();
}

void AUD_Device_setListenerOrientation(AUD_Device* device, const float value[4])
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	Quaternion v(value[3], value[0], value[1], value[2]);
	dev->setListenerOrientation(v);
}

void AUD_Device_getListenerVelocity(AUD_Device* device, float value[3])
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	Vector3 v = dev->getListenerVelocity();
	value[0] = v.x();
	value[1] = v.y();
	value[2] = v.z();
}

void AUD_Device_setListenerVelocity(AUD_Device* device, const float value[3])
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	Vector3 v(value[0], value[1], value[2]);
	dev->setListenerVelocity(v);
}

double AUD_Device_getRate(AUD_Device* device)
{
	auto dev = device ? *device : DeviceManager::getDevice();
	return dev->getSpecs().rate;
}

float AUD_Device_getSpeedOfSound(AUD_Device* device)
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	return dev->getSpeedOfSound();
}

void AUD_Device_setSpeedOfSound(AUD_Device* device, float value)
{
	auto dev = device ? std::dynamic_pointer_cast<I3DDevice>(*device) : DeviceManager::get3DDevice();
	dev->setSpeedOfSound(value);
}

float AUD_Device_getVolume(AUD_Device* device)
{
	auto dev = device ? *device : DeviceManager::getDevice();
	return dev->getVolume();
}

void AUD_Device_setVolume(AUD_Device* device, float value)
{
	auto dev = device ? *device : DeviceManager::getDevice();
	dev->setVolume(value);
}

static inline aud::Specs convCToSpec(AUD_Specs specs)
{
	aud::Specs s;
	s.channels = static_cast<Channels>(specs.channels);
	s.rate = static_cast<SampleRate>(specs.rate);
	return s;
}

static inline aud::DeviceSpecs convCToDSpec(AUD_DeviceSpecs specs)
{
	aud::DeviceSpecs s;
	s.specs = convCToSpec(specs.specs);
	s.format = static_cast<SampleFormat>(specs.format);
	return s;
}

AUD_Device *AUD_openReadDevice(AUD_DeviceSpecs specs)
{
	try
	{
		return new AUD_Device(new ReadDevice(convCToDSpec(specs)));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

int AUD_readDevice(AUD_Device *device, unsigned char *buffer, int length)
{
	assert(device);
	assert(buffer);

	auto readDevice = std::dynamic_pointer_cast<ReadDevice>(*device);
	if(!readDevice)
		return false;

	try
	{
		return readDevice->read(buffer, length);
	}
	catch(Exception&)
	{
		return false;
	}
}

void AUD_closeReadDevice(AUD_Device *device)
{
	assert(device);

	try
	{
		delete device;
	}
	catch(Exception&)
	{
	}
}

void AUD_seekSynchronizer(AUD_Handle *handle, float time)
{
	auto synchronizer = DeviceManager::getDevice()->getSynchronizer();
	if(synchronizer)
		synchronizer->seek(*reinterpret_cast<std::shared_ptr<IHandle>*>(handle), time);
}

float AUD_getSynchronizerPosition(AUD_Handle *handle)
{
	auto synchronizer = DeviceManager::getDevice()->getSynchronizer();
	if(synchronizer)
		return synchronizer->getPosition(*reinterpret_cast<std::shared_ptr<IHandle>*>(handle));
	return (*reinterpret_cast<std::shared_ptr<IHandle>*>(handle))->getPosition();
}

void AUD_playSynchronizer()
{
	auto synchronizer = DeviceManager::getDevice()->getSynchronizer();
	if(synchronizer)
		synchronizer->play();
}

void AUD_stopSynchronizer()
{
	auto synchronizer = DeviceManager::getDevice()->getSynchronizer();
	if(synchronizer)
		synchronizer->stop();
}

void AUD_setSynchronizerCallback(AUD_syncFunction function, void *data)
{
	auto synchronizer = DeviceManager::getDevice()->getSynchronizer();
	if(synchronizer)
		synchronizer->setSyncCallback(function, data);
}

int AUD_isSynchronizerPlaying()
{
	auto synchronizer = DeviceManager::getDevice()->getSynchronizer();
	if(synchronizer)
		return synchronizer->isPlaying();
	return false;
}

