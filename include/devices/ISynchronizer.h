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
 * @file ISynchronizer.h
 * @ingroup devices
 * The ISynchronizer interface.
 */

#include "Audaspace.h"

#include <memory>

AUD_NAMESPACE_BEGIN

/**
 * @interface ISynchronizer
 * This class enables global synchronization of several audio applications if supported.
 * JACK for example supports synchronization through JACK Transport.
 */
class AUD_API ISynchronizer
{
public:
	/**
	 * Destroys the synchronizer.
	 */
	virtual ~ISynchronizer() {}

	/**
	 * The syncFunction is called when a synchronization event happens.
	 * The function awaits three parameters. The first one is a user defined
	 * pointer, the second informs about whether playback is on and the third
	 * is the current playback time in seconds.
	 */
	typedef void (*syncFunction)(void*, int, float);

	/**
	 * Sets the playback position of the synchronizer to a specific time.
	 * @param time The absolute time to synchronize to.
	 */
	virtual void seek(double time) = 0;

	/**
	 * Retrieves the position of the synchronizer.
	 * @return The position in seconds.
	 */
	virtual double getPosition() = 0;

	/**
	 * Starts the synchronizer playback.
	 */
	virtual void play() = 0;

	/**
	 * Stops the synchronizer playback.
	 */
	virtual void stop() = 0;

	/**
	 * Sets the callback function that is called when a synchronization event happens.
	 * @param function The function to be called.
	 * @param data User data to be passed to the callback.
	 */
	virtual void setSyncCallback(syncFunction function, void* data) = 0;

	/**
	 * Retrieves whether the synchronizer is playing back.
	 * @return Whether the synchronizer plays back.
	 */
	virtual int isPlaying() = 0;
};

class AUD_API ISynchronizerDevice
{
public:
	/**
	 * Destroys the synchronizer device.
	 */
	virtual ~ISynchronizerDevice() {}

	virtual void seekSynchronizer(double time) = 0;
	virtual double getSynchronizerPosition() = 0;
	virtual void playSynchronizer() = 0;
	virtual void stopSynchronizer() = 0;
	virtual void setSyncCallback(ISynchronizer::syncFunction function, void* data) = 0;
	virtual int isSynchronizerPlaying() = 0;
};

AUD_NAMESPACE_END
