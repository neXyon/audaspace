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

#include "AUD_Device.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Possible animatable properties for Sequencer Factories and Entries.
typedef enum
{
	AUD_AP_VOLUME,
	AUD_AP_PANNING,
	AUD_AP_PITCH,
	AUD_AP_LOCATION,
	AUD_AP_ORIENTATION
} AUD_AnimateablePropertyType;

/**
 * Creates a new sequenced sound scene.
 * \param fps The FPS of the scene.
 * \param muted Whether the scene is muted.
 * \return The new sound scene.
 */
extern AUD_Sound *AUD_createSequencer(float fps, int muted);

/**
 * Deletes a sound scene.
 * \param sequencer The sound scene.
 */
extern void AUD_destroySequencer(AUD_Sound *sequencer);

/**
 * Sets the muting state of the scene.
 * \param sequencer The sound scene.
 * \param muted Whether the scene is muted.
 */
extern void AUD_setSequencerMuted(AUD_Sound *sequencer, int muted);

/**
 * Sets the scene's FPS.
 * \param sequencer The sound scene.
 * \param fps The new FPS.
 */
extern void AUD_setSequencerFPS(AUD_Sound *sequencer, float fps);

/**
 * Adds a new entry to the scene.
 * \param sequencer The sound scene.
 * \param sound The sound this entry should play.
 * \param begin The start time.
 * \param end The end time or a negative value if determined by the sound.
 * \param skip How much seconds should be skipped at the beginning.
 * \return The entry added.
 */
extern AUD_SEntry *AUD_addSequence(AUD_Sound *sequencer, AUD_Sound *sound,
								   float begin, float end, float skip);

/**
 * Removes an entry from the scene.
 * \param sequencer The sound scene.
 * \param entry The entry to remove.
 */
extern void AUD_removeSequence(AUD_Sound *sequencer, AUD_SEntry *entry);

/**
 * Moves the entry.
 * \param entry The sequenced entry.
 * \param begin The new start time.
 * \param end The new end time or a negative value if unknown.
 * \param skip How many seconds to skip at the beginning.
 */
extern void AUD_moveSequence(AUD_SEntry *entry, float begin, float end, float skip);

/**
 * Sets the muting state of the entry.
 * \param entry The sequenced entry.
 * \param mute Whether the entry should be muted or not.
 */
extern void AUD_muteSequence(AUD_SEntry *entry, char mute);

/**
 * Sets whether the entrie's location, velocity and orientation are relative
 * to the listener.
 * \param entry The sequenced entry.
 * \param relative Whether the source is relative.
 * \return Whether the action succeeded.
 */
extern void AUD_setRelativeSequence(AUD_SEntry *entry, char relative);

/**
 * Sets the sound of the entry.
 * \param entry The sequenced entry.
 * \param sound The new sound.
 */
extern void AUD_updateSequenceSound(AUD_SEntry *entry, AUD_Sound *sound);

/**
 * Writes animation data to a sequenced entry.
 * \param entry The sequenced entry.
 * \param type The type of animation data.
 * \param frame The frame this data is for.
 * \param data The data to write.
 * \param animated Whether the attribute is animated.
 */
extern void AUD_setSequenceAnimData(AUD_SEntry *entry, AUD_AnimateablePropertyType type, int frame, float *data, char animated);

/**
 * Writes animation data to a sequenced entry.
 * \param sequencer The sound scene.
 * \param type The type of animation data.
 * \param frame The frame this data is for.
 * \param data The data to write.
 * \param animated Whether the attribute is animated.
 */
extern void AUD_setSequencerAnimData(AUD_Sound *sequencer, AUD_AnimateablePropertyType type, int frame, float *data, char animated);

/**
 * Updates all non-animated parameters of the entry.
 * \param entry The sequenced entry.
 * \param volume_max The maximum volume.
 * \param volume_min The minimum volume.
 * \param distance_max The maximum distance.
 * \param distance_reference The reference distance.
 * \param attenuation The attenuation.
 * \param cone_angle_outer The outer cone opening angle.
 * \param cone_angle_inner The inner cone opening angle.
 * \param cone_volume_outer The volume outside the outer cone.
 */
extern void AUD_updateSequenceData(AUD_SEntry *entry, float volume_max, float volume_min,
								   float distance_max, float distance_reference, float attenuation,
								   float cone_angle_outer, float cone_angle_inner, float cone_volume_outer);

/**
 * Updates all non-animated parameters of the entry.
 * \param sequencer The sound scene.
 * \param speed_of_sound The speed of sound for doppler calculation.
 * \param factor The doppler factor to control the effect's strength.
 * \param model The distance model for distance calculation.
 */
extern void AUD_updateSequencerData(AUD_Sound *sequencer, float speed_of_sound,
									float factor, AUD_DistanceModel model);

/**
 * Sets the audio output specification of the sound scene to the specs of the
 * current playback device.
 * \param sequencer The sound scene.
 */
extern void AUD_setSequencerDeviceSpecs(AUD_Sound *sequencer);

/**
 * Sets the audio output specification of the sound scene.
 * \param sequencer The sound scene.
 * \param specs The new specification.
 */
extern void AUD_setSequencerSpecs(AUD_Sound *sequencer, AUD_Specs specs);

#ifdef __cplusplus
}
#endif
