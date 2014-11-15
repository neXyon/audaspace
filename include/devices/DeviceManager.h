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

#pragma once

/**
 * @file DeviceManager.h
 * @ingroup devices
 * The DeviceManager class.
 */

#include "Audaspace.h"

#include <memory>
#include <unordered_map>

AUD_NAMESPACE_BEGIN

class IDevice;
class IDeviceFactory;
class I3DDevice;

/**
 * This class manages all device plugins and maintains a device if asked to do so.
 */
class DeviceManager
{
private:
	static std::unordered_map<std::string, std::shared_ptr<IDeviceFactory>> m_factories;

	static std::shared_ptr<IDevice> m_device;

	// delete copy constructor and operator=
	DeviceManager(const DeviceManager&) = delete;
	DeviceManager& operator=(const DeviceManager&) = delete;
	DeviceManager() = delete;

public:
	static void registerDevice(std::string name, std::shared_ptr<IDeviceFactory> factory);
	static std::shared_ptr<IDeviceFactory> getDeviceFactory(std::string name);
	static std::shared_ptr<IDeviceFactory> getDefaultDeviceFactory();

	static void setDevice(std::shared_ptr<IDevice> device);
	static void openDevice(std::string name);
	static void openDefaultDevice();
	static void releaseDevice();

	static std::shared_ptr<IDevice> getDevice();
	static std::shared_ptr<I3DDevice> get3DDevice();
};

AUD_NAMESPACE_END
