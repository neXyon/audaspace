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

/**
 * Loads a sound file.
 * \param filename The filename of the sound file.
 * \return A handle of the sound file.
 */
extern AUD_Sound *AUD_load(const char *filename);

/**
 * Loads a sound file.
 * \param buffer The buffer which contains the sound file.
 * \param size The size of the buffer.
 * \return A handle of the sound file.
 */
extern AUD_Sound *AUD_loadBuffer(unsigned char *buffer, int size);

/**
 * Buffers a sound.
 * \param sound The sound to buffer.
 * \return A handle of the sound buffer.
 */
extern AUD_Sound *AUD_bufferSound(AUD_Sound *sound);

/**
 * Rechannels the sound to be mono.
 * \param sound The sound to rechannel.
 * \return The mono sound.
 */
extern AUD_Sound *AUD_monoSound(AUD_Sound *sound);

/**
 * Delays a sound.
 * \param sound The sound to dealy.
 * \param delay The delay in seconds.
 * \return A handle of the delayed sound.
 */
extern AUD_Sound *AUD_delaySound(AUD_Sound *sound, float delay);

/**
 * Limits a sound.
 * \param sound The sound to limit.
 * \param start The start time in seconds.
 * \param end The stop time in seconds.
 * \return A handle of the limited sound.
 */
extern AUD_Sound *AUD_limitSound(AUD_Sound *sound, float start, float end);

/**
 * Ping pongs a sound.
 * \param sound The sound to ping pong.
 * \return A handle of the ping pong sound.
 */
extern AUD_Sound *AUD_pingpongSound(AUD_Sound *sound);

/**
 * Loops a sound.
 * \param sound The sound to loop.
 * \return A handle of the looped sound.
 */
extern AUD_Sound *AUD_loopSound(AUD_Sound *sound);

/**
 * Rectifies a sound.
 * \param sound The sound to rectify.
 * \return A handle of the rectified sound.
 */
extern AUD_Sound *AUD_squareSound(AUD_Sound *sound);

/**
 * Unloads a sound of any type.
 * \param sound The handle of the sound.
 */
extern void AUD_unload(AUD_Sound *sound);

/**
 * Copies a sound.
 * \param sound Sound to copy.
 * \return Copied sound.
 */
extern AUD_Sound *AUD_copy(AUD_Sound *sound);

#ifdef __cplusplus
}
#endif
