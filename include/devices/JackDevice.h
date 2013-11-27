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

#include "devices/SoftwareDevice.h"
#include "util/Buffer.h"

#include <string>

#include "devices/JackLibrary.h"

AUD_NAMESPACE_BEGIN

typedef void (*syncFunction)(void*, int, float);

/**
 * This device plays back through Jack.
 */
class JackDevice : public SoftwareDevice
{
private:
	/**
	 * The output ports of jack.
	 */
	jack_port_t** m_ports;

	/**
	 * The jack client.
	 */
	jack_client_t* m_client;

	/**
	 * The output buffer.
	 */
	Buffer m_buffer;

	/**
	 * The deinterleaving buffer.
	 */
	Buffer m_deinterleavebuf;

	jack_ringbuffer_t** m_ringbuffers;

	/**
	 * Whether the device is valid.
	 */
	bool m_valid;

	/**
	 * Invalidates the jack device.
	 * \param data The jack device that gets invalidet by jack.
	 */
	static void jack_shutdown(void *data);

	/**
	 * Mixes the next bytes into the buffer.
	 * \param length The length in samples to be filled.
	 * \param data A pointer to the jack device.
	 * \return 0 what shows success.
	 */
	static int jack_mix(jack_nframes_t length, void *data);

	static int jack_sync(jack_transport_state_t state, jack_position_t* pos, void* data);

	/**
	 * Next Jack Transport state (-1 if not expected to change).
	 */
	jack_transport_state_t m_nextState;

	/**
	 * Current jack transport status.
	 */
	jack_transport_state_t m_state;

	/**
	 * Syncronisation state.
	 */
	int m_sync;

	/**
	 * External syncronisation callback function.
	 */
	syncFunction m_syncFunc;

	/**
	 * Data for the sync function.
	 */
	void* m_syncFuncData;

	/**
	 * The mixing thread.
	 */
	pthread_t m_mixingThread;

	/**
	 * Mutex for mixing.
	 */
	pthread_mutex_t m_mixingLock;

	/**
	 * Condition for mixing.
	 */
	pthread_cond_t m_mixingCondition;

	/**
	 * Mixing thread function.
	 * \param device The this pointer.
	 * \return NULL.
	 */
	static void* runMixingThread(void* device);

	/**
	 * Updates the ring buffers.
	 */
	void updateRingBuffers();

	// hide copy constructor and operator=
	JackDevice(const JackDevice&);
	JackDevice& operator=(const JackDevice&);

protected:
	virtual void playing(bool playing);

public:
	/**
	 * Creates a Jack client for audio output.
	 * \param name The client name.
	 * \param specs The wanted audio specification, where only the channel count
	 *              is important.
	 * \param buffersize The size of the internal buffer.
	 * \exception Exception Thrown if the audio device cannot be opened.
	 */
	JackDevice(std::string name, DeviceSpecs specs, int buffersize = AUD_DEFAULT_BUFFER_SIZE);

	/**
	 * Closes the Jack client.
	 */
	virtual ~JackDevice();

	/**
	 * Starts jack transport playback.
	 */
	void startPlayback();

	/**
	 * Stops jack transport playback.
	 */
	void stopPlayback();

	/**
	 * Seeks jack transport playback.
	 * \param time The time to seek to.
	 */
	void seekPlayback(float time);

	/**
	 * Sets the sync callback for jack transport playback.
	 * \param sync The callback function.
	 * \param data The data for the function.
	 */
	void setSyncCallback(syncFunction sync, void* data);

	/**
	 * Retrieves the jack transport playback time.
	 * \return The current time position.
	 */
	float getPlaybackPosition();

	/**
	 * Returns whether jack transport plays back.
	 * \return Whether jack transport plays back.
	 */
	bool doesPlayback();
};

AUD_NAMESPACE_END
