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

#include "AUD_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Possible distance models for the 3D device.
typedef enum
{
	AUD_DISTANCE_MODEL_INVALID = 0,
	AUD_DISTANCE_MODEL_INVERSE,
	AUD_DISTANCE_MODEL_INVERSE_CLAMPED,
	AUD_DISTANCE_MODEL_LINEAR,
	AUD_DISTANCE_MODEL_LINEAR_CLAMPED,
	AUD_DISTANCE_MODEL_EXPONENT,
	AUD_DISTANCE_MODEL_EXPONENT_CLAMPED
} AUD_DistanceModel;

typedef void (*AUD_syncFunction)(void*, int, float);

/**
 * Plays back a sound file.
 * \param sound The handle of the sound file.
 * \param keep When keep is true the sound source will not be deleted but set to
 *             paused when its end has been reached.
 * \return A handle to the played back sound.
 */
extern AUD_Handle *AUD_play(AUD_Sound *sound, int keep);

extern void AUD_stopAll(void);

/**
 * Sets the listener location.
 * \param location The new location.
 */
extern int AUD_setListenerLocation(const float location[3]);

/**
 * Sets the listener velocity.
 * \param velocity The new velocity.
 */
extern int AUD_setListenerVelocity(const float velocity[3]);

/**
 * Sets the listener orientation.
 * \param orientation The new orientation as quaternion.
 */
extern int AUD_setListenerOrientation(const float orientation[4]);

/**
 * Sets the speed of sound.
 * This value is needed for doppler effect calculation.
 * \param speed The new speed of sound.
 */
extern int AUD_setSpeedOfSound(float speed);

/**
 * Sets the doppler factor.
 * This value is a scaling factor for the velocity vectors of sources and
 * listener which is used while calculating the doppler effect.
 * \param factor The new doppler factor.
 */
extern int AUD_setDopplerFactor(float factor);

/**
 * Sets the distance model.
 * \param model distance model.
 */
extern int AUD_setDistanceModel(AUD_DistanceModel model);

/**
 * Locks the playback device.
 */
extern void AUD_lock(void);

/**
 * Unlocks the device.
 */
extern void AUD_unlock(void);

/**
 * Sets the main volume of a device.
 * \param device The device.
 * \param volume The new volume, must be between 0.0 and 1.0.
 * \return Whether the action succeeded.
 */
extern int AUD_setDeviceVolume(AUD_Device *device, float volume);

/**
 * Plays back a sound file through a read device.
 * \param device The read device.
 * \param sound The handle of the sound file.
 * \param seek The position where the sound should be seeked to.
 * \return A handle to the played back sound.
 */
extern AUD_Handle *AUD_playDevice(AUD_Device *device, AUD_Sound *sound, float seek);

/**
 * Opens a read device, with which audio data can be read.
 * \param specs The specification of the audio data.
 * \return A device handle.
 */
extern AUD_Device *AUD_openReadDevice(AUD_DeviceSpecs specs);

/**
 * Reads the next samples into the supplied buffer.
 * \param device The read device.
 * \param buffer The target buffer.
 * \param length The length in samples to be filled.
 * \return True if the reading succeeded, false if there are no sounds
 *         played back currently, in that case the buffer is filled with
 *         silence.
 */
extern int AUD_readDevice(AUD_Device *device, unsigned char *buffer, int length);

/**
 * Closes a read device.
 * \param device The read device.
 */
extern void AUD_closeReadDevice(AUD_Device *device);

/**
 * Seeks sequenced sound scene playback.
 * \param handle Playback handle.
 * \param time Time in seconds to seek to.
 */
extern void AUD_seekSynchronizer(AUD_Handle *handle, float time);

/**
 * Returns the current sound scene playback time.
 * \param handle Playback handle.
 * \return The playback time in seconds.
 */
extern float AUD_getSynchronizerPosition(AUD_Handle *handle);

/**
 * Starts the playback of jack transport if possible.
 */
extern void AUD_playSynchronizer();

/**
 * Stops the playback of jack transport if possible.
 */
extern void AUD_stopSynchronizer();

/**
 * Sets the sync callback for jack transport.
 * \param function The callback function.
 * \param data The data parameter for the callback.
 */
extern void AUD_setSynchronizerCallback(AUD_syncFunction function, void *data);

/**
 * Returns whether jack transport is currently playing.
 * \return Whether jack transport is currently playing.
 */
extern int AUD_isSynchronizerPlaying();

#ifdef __cplusplus
}
#endif
