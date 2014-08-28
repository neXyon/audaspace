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

/// Possible animatable properties for Sequence Factories and Entries.
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
extern AUD_Sound* AUD_Sequence_create(float fps, int muted);

/**
 * Deletes a sound scene.
 * \param sequence The sound scene.
 */
extern void AUD_Sequence_free(AUD_Sound* sequence);

/**
 * Adds a new entry to the scene.
 * \param sequence The sound scene.
 * \param sound The sound this entry should play.
 * \param begin The start time.
 * \param end The end time or a negative value if determined by the sound.
 * \param skip How much seconds should be skipped at the beginning.
 * \return The entry added.
 */
extern AUD_SEntry* AUD_Sequence_addEntry(AUD_Sound* sequence, AUD_Sound* sound, float begin, float end, float skip);

/**
 * Removes an entry from the scene.
 * \param sequence The sound scene.
 * \param entry The entry to remove.
 */
extern void AUD_Sequence_removeEntry(AUD_Sound* sequence, AUD_SEntry* entry);

/**
 * Writes animation data to a sequenced entry.
 * \param sequence The sound scene.
 * \param type The type of animation data.
 * \param frame The frame this data is for.
 * \param data The data to write.
 * \param animated Whether the attribute is animated.
 */
extern void AUD_Sequence_setAnimationData(AUD_Sound* sequence, AUD_AnimateablePropertyType type, int frame, float* data, char animated);

/**
 * Retrieves the distance model of a sequence.
 * param sequence The sequence to get the distance model from.
 * return The distance model of the sequence.
 */
extern AUD_DistanceModel AUD_Sequence_getDistanceModel(AUD_Sound* sequence);

/**
 * Sets the distance model of a sequence.
 * param sequence The sequence to set the distance model from.
 * param value The new distance model to set.
 */
extern void AUD_Sequence_setDistanceModel(AUD_Sound* sequence, AUD_DistanceModel value);

/**
 * Retrieves the doppler factor of a sequence.
 * param sequence The sequence to get the doppler factor from.
 * return The doppler factor of the sequence.
 */
extern float AUD_Sequence_getDopplerFactor(AUD_Sound* sequence);

/**
 * Sets the doppler factor of a sequence.
 * param sequence The sequence to set the doppler factor from.
 * param value The new doppler factor to set.
 */
extern void AUD_Sequence_setDopplerFactor(AUD_Sound* sequence, float value);

/**
 * Retrieves the fps of a sequence.
 * param sequence The sequence to get the fps from.
 * return The fps of the sequence.
 */
extern float AUD_Sequence_getFPS(AUD_Sound* sequence);

/**
 * Sets the fps of a sequence.
 * param sequence The sequence to set the fps from.
 * param value The new fps to set.
 */
extern void AUD_Sequence_setFPS(AUD_Sound* sequence, float value);

/**
 * Retrieves the muted of a sequence.
 * param sequence The sequence to get the muted from.
 * return The muted of the sequence.
 */
extern int AUD_Sequence_getMuted(AUD_Sound* sequence);

/**
 * Sets the muted of a sequence.
 * param sequence The sequence to set the muted from.
 * param value The new muted to set.
 */
extern void AUD_Sequence_setMuted(AUD_Sound* sequence, int value);

/**
 * Retrieves the specs of a sequence.
 * param sequence The sequence to get the specs from.
 * return The specs of the sequence.
 */
extern AUD_Specs AUD_Sequence_getSpecs(AUD_Sound* sequence);

/**
 * Sets the specs of a sequence.
 * param sequence The sequence to set the specs from.
 * param value The new specs to set.
 */
extern void AUD_Sequence_setSpecs(AUD_Sound* sequence, AUD_Specs value);

/**
 * Retrieves the speed of sound of a sequence.
 * param sequence The sequence to get the speed of sound from.
 * return The speed of sound of the sequence.
 */
extern float AUD_Sequence_getSpeedOfSound(AUD_Sound* sequence);

/**
 * Sets the speed of sound of a sequence.
 * param sequence The sequence to set the speed of sound from.
 * param value The new speed of sound to set.
 */
extern void AUD_Sequence_setSpeedOfSound(AUD_Sound* sequence, float value);




/**
 * Moves the entry.
 * \param entry The sequenced entry.
 * \param begin The new start time.
 * \param end The new end time or a negative value if unknown.
 * \param skip How many seconds to skip at the beginning.
 */
extern void AUD_moveSequence(AUD_SEntry* entry, float begin, float end, float skip);

/**
 * Sets the muting state of the entry.
 * \param entry The sequenced entry.
 * \param mute Whether the entry should be muted or not.
 */
extern void AUD_muteSequence(AUD_SEntry* entry, char mute);

/**
 * Sets whether the entrie's location, velocity and orientation are relative
 * to the listener.
 * \param entry The sequenced entry.
 * \param relative Whether the source is relative.
 * \return Whether the action succeeded.
 */
extern void AUD_setRelativeSequence(AUD_SEntry* entry, char relative);

/**
 * Sets the sound of the entry.
 * \param entry The sequenced entry.
 * \param sound The new sound.
 */
extern void AUD_updateSequenceSound(AUD_SEntry* entry, AUD_Sound* sound);

/**
 * Writes animation data to a sequenced entry.
 * \param entry The sequenced entry.
 * \param type The type of animation data.
 * \param frame The frame this data is for.
 * \param data The data to write.
 * \param animated Whether the attribute is animated.
 */
extern void AUD_setSequenceAnimData(AUD_SEntry* entry, AUD_AnimateablePropertyType type, int frame, float* data, char animated);

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
extern void AUD_updateSequenceData(AUD_SEntry* entry, float volume_max, float volume_min,
								   float distance_max, float distance_reference, float attenuation,
								   float cone_angle_outer, float cone_angle_inner, float cone_volume_outer);

#ifdef __cplusplus
}
#endif
