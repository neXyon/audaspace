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

#include "IReader.h"
#include "IDevice.h"
#include "IHandle.h"

AUD_NAMESPACE_BEGIN

/**
 * This device plays nothing.
 */
class NULLDevice : public IDevice
{
private:
	class NULLHandle : public IHandle
	{
	public:

		NULLHandle();

		virtual ~NULLHandle() {}
		virtual bool pause();
		virtual bool resume();
		virtual bool stop();
		virtual bool getKeep();
		virtual bool setKeep(bool keep);
		virtual bool seek(float position);
		virtual float getPosition();
		virtual Status getStatus();
		virtual float getVolume();
		virtual bool setVolume(float volume);
		virtual float getPitch();
		virtual bool setPitch(float pitch);
		virtual int getLoopCount();
		virtual bool setLoopCount(int count);
		virtual bool setStopCallback(stopCallback callback = 0, void* data = 0);
	};
public:
	/**
	 * Creates a new NULL device.
	 */
	NULLDevice();

	virtual ~NULLDevice();

	virtual DeviceSpecs getSpecs() const;
	virtual std::shared_ptr<IHandle> play(std::shared_ptr<IReader> reader, bool keep = false);
	virtual std::shared_ptr<IHandle> play(std::shared_ptr<ISound> factory, bool keep = false);
	virtual void stopAll();
	virtual void lock();
	virtual void unlock();
	virtual float getVolume() const;
	virtual void setVolume(float volume);
};

AUD_NAMESPACE_END
