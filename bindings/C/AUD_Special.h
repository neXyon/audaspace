/*******************************************************************************
 * Copyright 2009-2015 Jörg Müller
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
 * Returns information about a sound.
 * \param sound The sound to get the info about.
 * \return The AUD_SoundInfo structure with filled in data.
 */
extern AUD_SoundInfo AUD_getInfo(AUD_Sound* sound);

/**
 * Reads a sound file into a newly created float buffer.
 * The sound is therefore bandpassed, rectified and resampled.
 */
extern float* AUD_readSoundBuffer(const char* filename, float low, float high,
								  float attack, float release, float threshold,
								  int accumulate, int additive, int square,
								  float sthreshold, double samplerate,
								  int* length);

/**
 * Pauses a playing sound after a specific amount of time.
 * \param handle The handle to the sound.
 * \param seconds The time in seconds.
 * \return The silence handle.
 */
extern AUD_Handle* AUD_pauseAfter(AUD_Handle* handle, float seconds);

/**
 * Reads a sound into a buffer for drawing at a specific sampling rate.
 * \param sound The sound to read.
 * \param buffer The buffer to write to. Must have a size of 3*4*length.
 * \param length How many samples to read from the sound.
 * \param samples_per_second How many samples to read per second of the sound.
 * \return How many samples really have been read. Always <= length.
 */
extern int AUD_readSound(AUD_Sound* sound, float* buffer, int length, int samples_per_second, short* interrupt);

/**
 * Mixes a sound down into a file.
 * \param sound The sound scene to mix down.
 * \param start The start frame.
 * \param length The count of frames to write.
 * \param buffersize How many samples should be written at once.
 * \param filename The file to write to.
 * \param specs The file's audio specification.
 * \param format The file's container format.
 * \param codec The codec used for encoding the audio data.
 * \param bitrate The bitrate for encoding.
 * \return An error message or NULL in case of success.
 */
extern const char* AUD_mixdown(AUD_Sound* sound, unsigned int start, unsigned int length,
							   unsigned int buffersize, const char* filename,
							   AUD_DeviceSpecs specs, AUD_Container format,
							   AUD_Codec codec, unsigned int bitrate);

/**
 * Mixes a sound down into multiple files.
 * \param sound The sound scene to mix down.
 * \param start The start frame.
 * \param length The count of frames to write.
 * \param buffersize How many samples should be written at once.
 * \param filename The file to write to, the channel number and an underscore are added at the beginning.
 * \param specs The file's audio specification.
 * \param format The file's container format.
 * \param codec The codec used for encoding the audio data.
 * \param bitrate The bitrate for encoding.
 * \return An error message or NULL in case of success.
 */
extern const char* AUD_mixdown_per_channel(AUD_Sound* sound, unsigned int start, unsigned int length,
										   unsigned int buffersize, const char* filename,
										   AUD_DeviceSpecs specs, AUD_Container format,
										   AUD_Codec codec, unsigned int bitrate);

/**
 * Opens a read device and prepares it for mixdown of the sound scene.
 * \param specs Output audio specifications.
 * \param sequencer The sound scene to mix down.
 * \param volume The overall mixdown volume.
 * \param start The start time of the mixdown in the sound scene.
 * \return The read device for the mixdown.
 */
extern AUD_Device* AUD_openMixdownDevice(AUD_DeviceSpecs specs, AUD_Sound* sequencer, float volume, float start);

/**
 * Initializes audio rutines (FFMPEG/Jack if it is enabled).
 */
extern void AUD_initOnce();

/**
 * Unitinitializes an audio routines.
 */
extern void AUD_exitOnce();

/**
 * Initializes an audio device.
 * \param device The device type that should be used.
 * \param specs The audio specification to be used.
 * \param buffersize The buffersize for the device.
 * \return Whether the device has been initialized.
 */
extern AUD_Device* AUD_init(const char* device, AUD_DeviceSpecs specs, int buffersize, const char* name);

/**
 * Unitinitializes an audio device.
 */
extern void AUD_exit(AUD_Device* device);

#ifdef __cplusplus
}
#endif
