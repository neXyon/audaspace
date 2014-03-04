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

/// Status of a playback handle.
typedef enum
{
	AUD_STATUS_INVALID = 0,			/// Invalid handle. Maybe due to stopping.
	AUD_STATUS_PLAYING,				/// Sound is playing.
	AUD_STATUS_PAUSED,				/// Sound is being paused.
	AUD_STATUS_STOPPED				/// Sound is stopped but kept in the device.
} AUD_Status;

/**
 * Sets a remaining loop count of a looping sound that currently plays.
 * \param handle The playback handle.
 * \param loops The count of remaining loops, -1 for infinity.
 * \return Whether the handle is valid.
 */
extern int AUD_setLoop(AUD_Handle *handle, int loops);

/**
 * Pauses a played back sound.
 * \param handle The handle to the sound.
 * \return Whether the handle has been playing or not.
 */
extern int AUD_pause(AUD_Handle *handle);

/**
 * Resumes a paused sound.
 * \param handle The handle to the sound.
 * \return Whether the handle has been paused or not.
 */
extern int AUD_resume(AUD_Handle *handle);

/**
 * Stops a playing or paused sound.
 * \param handle The handle to the sound.
 * \return Whether the handle has been valid or not.
 */
extern int AUD_stop(AUD_Handle *handle);

/**
 * Sets the end behaviour of a playing or paused sound.
 * \param handle The handle to the sound.
 * \param keep When keep is true the sound source will not be deleted but set to
 *             paused when its end has been reached.
 * \return Whether the handle has been valid or not.
 */
extern int AUD_setKeep(AUD_Handle *handle, int keep);

/**
 * Seeks a playing or paused sound.
 * \param handle The handle to the sound.
 * \param seekTo From where the sound file should be played back in seconds.
 * \return Whether the handle has been valid or not.
 */
extern int AUD_seek(AUD_Handle *handle, float seekTo);

/**
 * Retrieves the playback position of a handle.
 * \param handle The handle to the sound.
 * \return The current playback position in seconds or 0.0 if the handle is
 *         invalid.
 */
extern float AUD_getPosition(AUD_Handle *handle);

/**
 * Returns the status of a playing, paused or stopped sound.
 * \param handle The handle to the sound.
 * \return The status of the sound behind the handle.
 */
extern AUD_Status AUD_getStatus(AUD_Handle *handle);

/**
 * Sets the location of a source.
 * \param handle The handle of the source.
 * \param location The new location.
 * \return Whether the action succeeded.
 */
extern int AUD_setSourceLocation(AUD_Handle *handle, const float location[3]);

/**
 * Sets the velocity of a source.
 * \param handle The handle of the source.
 * \param velocity The new velocity.
 * \return Whether the action succeeded.
 */
extern int AUD_setSourceVelocity(AUD_Handle *handle, const float velocity[3]);

/**
 * Sets the orientation of a source.
 * \param handle The handle of the source.
 * \param orientation The new orientation as quaternion.
 * \return Whether the action succeeded.
 */
extern int AUD_setSourceOrientation(AUD_Handle *handle, const float orientation[4]);

/**
 * Sets whether the source location, velocity and orientation are relative
 * to the listener.
 * \param handle The handle of the source.
 * \param relative Whether the source is relative.
 * \return Whether the action succeeded.
 */
extern int AUD_setRelative(AUD_Handle *handle, int relative);

/**
 * Sets the maximum volume of a source.
 * \param handle The handle of the source.
 * \param volume The new maximum volume.
 * \return Whether the action succeeded.
 */
extern int AUD_setVolumeMaximum(AUD_Handle *handle, float volume);

/**
 * Sets the minimum volume of a source.
 * \param handle The handle of the source.
 * \param volume The new minimum volume.
 * \return Whether the action succeeded.
 */
extern int AUD_setVolumeMinimum(AUD_Handle *handle, float volume);

/**
 * Sets the maximum distance of a source.
 * If a source is further away from the reader than this distance, the
 * volume will automatically be set to 0.
 * \param handle The handle of the source.
 * \param distance The new maximum distance.
 * \return Whether the action succeeded.
 */
extern int AUD_setDistanceMaximum(AUD_Handle *handle, float distance);

/**
 * Sets the reference distance of a source.
 * \param handle The handle of the source.
 * \param distance The new reference distance.
 * \return Whether the action succeeded.
 */
extern int AUD_setDistanceReference(AUD_Handle *handle, float distance);

/**
 * Sets the attenuation of a source.
 * This value is used for distance calculation.
 * \param handle The handle of the source.
 * \param factor The new attenuation.
 * \return Whether the action succeeded.
 */
extern int AUD_setAttenuation(AUD_Handle *handle, float factor);

/**
 * Sets the outer angle of the cone of a source.
 * \param handle The handle of the source.
 * \param angle The new outer angle of the cone.
 * \return Whether the action succeeded.
 */
extern int AUD_setConeAngleOuter(AUD_Handle *handle, float angle);

/**
 * Sets the inner angle of the cone of a source.
 * \param handle The handle of the source.
 * \param angle The new inner angle of the cone.
 * \return Whether the action succeeded.
 */
extern int AUD_setConeAngleInner(AUD_Handle *handle, float angle);

/**
 * Sets the outer volume of the cone of a source.
 * The volume between inner and outer angle is interpolated between inner
 * volume and this value.
 * \param handle The handle of the source.
 * \param volume The new outer volume of the cone.
 * \return Whether the action succeeded.
 */
extern int AUD_setConeVolumeOuter(AUD_Handle *handle, float volume);

/**
 * Sets the volume of a played back sound.
 * \param handle The handle to the sound.
 * \param volume The new volume, must be between 0.0 and 1.0.
 * \return Whether the action succeeded.
 */
extern int AUD_setSoundVolume(AUD_Handle *handle, float volume);

/**
 * Sets the pitch of a played back sound.
 * \param handle The handle to the sound.
 * \param pitch The new pitch.
 * \return Whether the action succeeded.
 */
extern int AUD_setSoundPitch(AUD_Handle *handle, float pitch);

/**
 * Frees a handle.
 * \param channel Handle to free.
 */
extern void AUD_freeHandle(AUD_Handle *channel);

#ifdef __cplusplus
}
#endif
