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

/// Container formats for writers.
enum AUD_Container
{
	AUD_CONTAINER_INVALID = 0,
	AUD_CONTAINER_AC3,
	AUD_CONTAINER_FLAC,
	AUD_CONTAINER_MATROSKA,
	AUD_CONTAINER_MP2,
	AUD_CONTAINER_MP3,
	AUD_CONTAINER_OGG,
	AUD_CONTAINER_WAV
};

/// Audio codecs for writers.
enum AUD_Codec
{
	AUD_CODEC_INVALID = 0,
	AUD_CODEC_AAC,
	AUD_CODEC_AC3,
	AUD_CODEC_FLAC,
	AUD_CODEC_MP2,
	AUD_CODEC_MP3,
	AUD_CODEC_PCM,
	AUD_CODEC_VORBIS,
	AUD_CODEC_OPUS
};

/**
 * The format of a sample.
 * The last 4 bit save the byte count of the format.
 */
typedef enum
{
	AUD_FORMAT_INVALID = 0x00,		/// Invalid sample format.
	AUD_FORMAT_U8      = 0x01,		/// 1 byte unsigned byte.
	AUD_FORMAT_S16     = 0x12,		/// 2 byte signed integer.
	AUD_FORMAT_S24     = 0x13,		/// 3 byte signed integer.
	AUD_FORMAT_S32     = 0x14,		/// 4 byte signed integer.
	AUD_FORMAT_FLOAT32 = 0x24,		/// 4 byte float.
	AUD_FORMAT_FLOAT64 = 0x28		/// 8 byte float.
} AUD_SampleFormat;

/// The channel count.
typedef enum
{
	AUD_CHANNELS_INVALID    = 0,	/// Invalid channel count.
	AUD_CHANNELS_MONO       = 1,	/// Mono.
	AUD_CHANNELS_STEREO     = 2,	/// Stereo.
	AUD_CHANNELS_STEREO_LFE = 3,	/// Stereo with LFE channel.
	AUD_CHANNELS_SURROUND4  = 4,	/// 4 channel surround sound.
	AUD_CHANNELS_SURROUND5  = 5,	/// 5 channel surround sound.
	AUD_CHANNELS_SURROUND51 = 6,	/// 5.1 surround sound.
	AUD_CHANNELS_SURROUND61 = 7,	/// 6.1 surround sound.
	AUD_CHANNELS_SURROUND71 = 8	/// 7.1 surround sound.
} AUD_Channels;

/**
 * The sample rate tells how many samples are played back within one second.
 * Some exotic formats may use other sample rates than provided here.
 */
typedef enum
{
	AUD_RATE_INVALID = 0,			/// Invalid sample rate.
	AUD_RATE_8000    = 8000,		/// 8000 Hz.
	AUD_RATE_16000   = 16000,		/// 16000 Hz.
	AUD_RATE_11025   = 11025,		/// 11025 Hz.
	AUD_RATE_22050   = 22050,		/// 22050 Hz.
	AUD_RATE_32000   = 32000,		/// 32000 Hz.
	AUD_RATE_44100   = 44100,		/// 44100 Hz.
	AUD_RATE_48000   = 48000,		/// 48000 Hz.
	AUD_RATE_88200   = 88200,		/// 88200 Hz.
	AUD_RATE_96000   = 96000,		/// 96000 Hz.
	AUD_RATE_192000  = 192000		/// 192000 Hz.
} AUD_DefaultSampleRate;

/// Sample rate type.
typedef double AUD_SampleRate;

/// Specification of a sound source.
typedef struct
{
	/// Sample rate in Hz.
	AUD_SampleRate rate;

	/// Channel count.
	AUD_Channels channels;
} AUD_Specs;

/// Specification of a sound device.
typedef struct
{
	/// Sample format.
	AUD_SampleFormat format;

	union
	{
		struct
		{
			/// Sample rate in Hz.
			AUD_SampleRate rate;

			/// Channel count.
			AUD_Channels channels;
		};
		AUD_Specs specs;
	};
} AUD_DeviceSpecs;

/// Sound information structure.
typedef struct
{
	AUD_Specs specs;
	float length;
} AUD_SoundInfo;

/**
 * Returns information about a sound.
 * \param sound The sound to get the info about.
 * \return The AUD_SoundInfo structure with filled in data.
 */
extern AUD_SoundInfo AUD_getInfo(AUD_Sound *sound);

/**
 * Reads a sound file into a newly created float buffer.
 * The sound is therefore bandpassed, rectified and resampled.
 */
extern float *AUD_readSoundBuffer(const char *filename, float low, float high,
								  float attack, float release, float threshold,
								  int accumulate, int additive, int square,
								  float sthreshold, double samplerate,
								  int *length);

/**
 * Pauses a playing sound after a specific amount of time.
 * \param handle The handle to the sound.
 * \param seconds The time in seconds.
 * \return The silence handle.
 */
extern AUD_Handle *AUD_pauseAfter(AUD_Handle *handle, float seconds);

/**
 * Reads a sound into a buffer for drawing at a specific sampling rate.
 * \param sound The sound to read.
 * \param buffer The buffer to write to. Must have a size of 3*4*length.
 * \param length How many samples to read from the sound.
 * \param samples_per_second How many samples to read per second of the sound.
 * \return How many samples really have been read. Always <= length.
 */
extern int AUD_readSound(AUD_Sound *sound, sample_t *buffer, int length, int samples_per_second);

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
extern const char *AUD_mixdown(AUD_Sound *sound, unsigned int start, unsigned int length,
							   unsigned int buffersize, const char *filename,
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
extern const char *AUD_mixdown_per_channel(AUD_Sound *sound, unsigned int start, unsigned int length,
										   unsigned int buffersize, const char *filename,
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
extern AUD_Device *AUD_openMixdownDevice(AUD_DeviceSpecs specs, AUD_Sound *sequencer, float volume, float start);

/**
 * Initializes audio rutines (FFMPEG/Jack if it is enabled).
 */
extern void AUD_initOnce(void);

/**
 * Unitinitializes an audio routines.
 */
extern void AUD_exitOnce(void);

/**
 * Initializes an audio device.
 * \param device The device type that should be used.
 * \param specs The audio specification to be used.
 * \param buffersize The buffersize for the device.
 * \return Whether the device has been initialized.
 */
extern int AUD_init(const char* device, const char* name, AUD_DeviceSpecs specs, int buffersize);

/**
 * Unitinitializes an audio device.
 */
extern void AUD_exit(void);

#ifdef __cplusplus
}
#endif
