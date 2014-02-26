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

#ifndef AUD_CAPI_IMPLEMENTATION
/// Possible distance models for the 3D device.
enum DistanceModel
{
	DISTANCE_MODEL_INVALID = 0,
	DISTANCE_MODEL_INVERSE,
	DISTANCE_MODEL_INVERSE_CLAMPED,
	DISTANCE_MODEL_LINEAR,
	DISTANCE_MODEL_LINEAR_CLAMPED,
	DISTANCE_MODEL_EXPONENT,
	DISTANCE_MODEL_EXPONENT_CLAMPED
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Plays back a sound file.
 * \param sound The handle of the sound file.
 * \param keep When keep is true the sound source will not be deleted but set to
 *             paused when its end has been reached.
 * \return A handle to the played back sound.
 */
extern AUD_Handle *AUD_play(AUD_Sound *sound, int keep);

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
extern int AUD_setDistanceModel(DistanceModel model);

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
extern AUD_Device *AUD_openReadDevice(DeviceSpecs specs);

/**
 * Reads the next samples into the supplied buffer.
 * \param device The read device.
 * \param buffer The target buffer.
 * \param length The length in samples to be filled.
 * \return True if the reading succeeded, false if there are no sounds
 *         played back currently, in that case the buffer is filled with
 *         silence.
 */
extern int AUD_readDevice(AUD_Device *device, data_t *buffer, int length);

/**
 * Closes a read device.
 * \param device The read device.
 */
extern void AUD_closeReadDevice(AUD_Device *device);

#ifdef __cplusplus
}
#endif
