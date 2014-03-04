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

AUD_Handle *AUD_play(AUD_Sound *sound, int keep)
{
	assert(sound);
	try
	{
		AUD_Handle handle = DeviceManager::getDevice()->play(*sound, keep);
		if(handle.get())
		{
			return new AUD_Handle(handle);
		}
	}
	catch(Exception&)
	{
	}
	return NULL;
}

void AUD_stopAll()
{
	DeviceManager::getDevice()->stopAll();
}

int AUD_setListenerLocation(const float location[3])
{
	auto device = DeviceManager::get3DDevice();
	if(device)
	{
		Vector3 v(location[0], location[1], location[2]);
		device->setListenerLocation(v);
		return true;
	}

	return false;
}

int AUD_setListenerVelocity(const float velocity[3])
{
	auto device = DeviceManager::get3DDevice();
	if(device)
	{
		Vector3 v(velocity[0], velocity[1], velocity[2]);
		device->setListenerVelocity(v);
		return true;
	}

	return false;
}

int AUD_setListenerOrientation(const float orientation[4])
{
	auto device = DeviceManager::get3DDevice();
	if(device)
	{
		Quaternion q(orientation[3], orientation[0], orientation[1], orientation[2]);
		device->setListenerOrientation(q);
		return true;
	}

	return false;
}

int AUD_setSpeedOfSound(float speed)
{
	auto device = DeviceManager::get3DDevice();
	if(device)
	{
		device->setSpeedOfSound(speed);
		return true;
	}

	return false;
}

int AUD_setDopplerFactor(float factor)
{
	auto device = DeviceManager::get3DDevice();
	if(device)
	{
		device->setDopplerFactor(factor);
		return true;
	}

	return false;
}

int AUD_setDistanceModel(AUD_DistanceModel model)
{
	auto device = DeviceManager::get3DDevice();
	if(device)
	{
		device->setDistanceModel(static_cast<DistanceModel>(model));
		return true;
	}

	return false;
}

void AUD_lock()
{
	DeviceManager::getDevice()->lock();
}

void AUD_unlock()
{
	DeviceManager::getDevice()->unlock();
}

AUD_Handle *AUD_playDevice(AUD_Device *device, AUD_Sound *sound, float seek)
{
	assert(device);
	assert(sound);

	try
	{
		AUD_Handle handle = (*device)->play(*sound);
		if(handle.get())
		{
			handle->seek(seek);
			return new AUD_Handle(handle);
		}
	}
	catch(Exception&)
	{
	}
	return NULL;
}

int AUD_setDeviceVolume(AUD_Device *device, float volume)
{
	assert(device);

	try
	{
		(*device)->setVolume(volume);
		return true;
	}
	catch(Exception&)
	{
	}

	return false;
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
		return NULL;
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

