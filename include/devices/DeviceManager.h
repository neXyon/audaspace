/*******************************************************************************
 * Copyright 2009-2016 Jörg Müller
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
#include "respec/Specification.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

AUD_NAMESPACE_BEGIN

class IDevice;
class IDeviceFactory;
class I3DDevice;
class IReader;

typedef std::vector<std::string> (*CaptureDeviceNamesCallback)();
typedef std::shared_ptr<IReader> (*CaptureReaderCallback)(const std::string& name,
                                                          Specs specs,
                                                          int buffersize);

/**
 * This class manages all device plugins and maintains a device if asked to do so.
 *
 * This enables applications to access their output device without having to carry
 * it through the whole application.
 */
class AUD_API DeviceManager
{
private:
	static std::unordered_map<std::string, std::shared_ptr<IDeviceFactory>> m_factories;

	static std::shared_ptr<IDevice> m_device;
	static CaptureDeviceNamesCallback m_captureDeviceNamesCallback;
	static CaptureReaderCallback m_captureReaderCallback;

	// delete copy constructor and operator=
	DeviceManager(const DeviceManager&) = delete;
	DeviceManager& operator=(const DeviceManager&) = delete;
	DeviceManager() = delete;

public:
	/**
	 * Registers a device factory.
	 *
	 * This method is mostly used by plugin developers to add their device implementation
	 * for general use by the library end users.
	 * @param name A representative name for the device.
	 * @param factory The factory that creates the device.
	 */
	static void registerDevice(const std::string &name, std::shared_ptr<IDeviceFactory> factory);

	/**
	 * Returns the factory for a specific device.
	 * @param name The representative name of the device.
	 * @return The factory if it was found, or nullptr otherwise.
	 */
	static std::shared_ptr<IDeviceFactory> getDeviceFactory(const std::string &name);

	/**
	 * Returns the default device based on the priorities of the registered factories.
	 * @return The default device or nullptr if no factory has been registered.
	 */
	static std::shared_ptr<IDeviceFactory> getDefaultDeviceFactory();


	/**
	 * Sets a device that should be handled by the manager.
	 *
	 * If a device is currently being handled it will be released.
	 * @param device The device the manager should take care of.
	 */
	static void setDevice(std::shared_ptr<IDevice> device);

	/**
	 * Opens a device which will then be handled by the manager.
	 *
	 * If a device is currently being handled it will be released.
	 * @param name The representative name of the device.
	 */
	static void openDevice(const std::string &name);

	/**
	 * Opens the default device which will then be handled by the manager.
	 *
	 * The device to open is selected based on the priority of the registered factories.
	 * If a device is currently being handled it will be released.
	 */
	static void openDefaultDevice();

	/**
	 * Releases the currently handled device.
	 */
	static void releaseDevice();

	/**
	 * Returns the currently handled device.
	 * @return The handled device or nullptr if no device has been registered.
	 */
	static std::shared_ptr<IDevice> getDevice();

	/**
	 * Returns the currently handled 3D device.
	 * @return The handled device or nullptr if no device has been registered
	 *         or the registered device is not an I3DDevice.
	 */
	static std::shared_ptr<I3DDevice> get3DDevice();

	/**
	 * Returns a list of available devices.
	 * @return A list of strings with the names of available devices.
	 */
	static std::vector<std::string> getAvailableDeviceNames();

	/**
	 * Returns a list of available capture devices.
	 * @return A list of strings with the names of available capture devices.
	 */
	static std::vector<std::string> getAvailableCaptureDeviceNames();

	/**
	 * Registers a callback for listing capture devices.
	 * @param callback Callback for capture device name retrieval.
	 */
	static void registerCaptureDeviceNamesCallback(CaptureDeviceNamesCallback callback);

	/**
	 * Opens an input capture reader.
	 * @param name The capture device name.
	 * @param specs The desired specification.
	 * @param buffersize The capture buffer size in samples.
	 */
	static std::shared_ptr<IReader> openCaptureDevice(const std::string& name,
	                                                 Specs specs,
	                                                 int buffersize = AUD_DEFAULT_BUFFER_SIZE);

	/**
	 * Registers a callback for opening capture readers.
	 * @param callback Callback that creates a capture reader.
	 */
	static void registerCaptureReaderCallback(CaptureReaderCallback callback);
};

AUD_NAMESPACE_END
