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

/// The size of a format in bytes.
#define AUD_FORMAT_SIZE(format) (format & 0x0F)
/// The size of a sample in the specified device format in bytes.
#define AUD_DEVICE_SAMPLE_SIZE(specs) (specs.channels * (specs.format & 0x0F))
/// The size of a sample in the specified format in bytes.
#define AUD_SAMPLE_SIZE(specs) (specs.channels * sizeof(sample_t))
/// Throws a Exception with the provided error code.
#define AUD_THROW(exception, errorstr) { Exception e; e.error = exception; e.str = errorstr; throw e; }

/// Compares two audio data specifications.
#define AUD_COMPARE_SPECS(s1, s2) ((s1.rate == s2.rate) && (s1.channels == s2.channels))

/// Returns the bit for a channel mask.
#define AUD_CHANNEL_BIT(channel) (0x01 << channel)

/// Returns the smaller of the two values.
#define AUD_MIN(a, b) (((a) < (b)) ? (a) : (b))
/// Returns the bigger of the two values.
#define AUD_MAX(a, b) (((a) > (b)) ? (a) : (b))

// 5 sec * 44100 samples/sec * 4 bytes/sample * 6 channels
/// The size by which a buffer should be resized if the final extent is unknown.
#define AUD_BUFFER_RESIZE_BYTES 5292000

/// The default playback buffer size of a device.
#define AUD_DEFAULT_BUFFER_SIZE 1024

#define AUD_NAMESPACE_BEGIN namespace aud {
#define AUD_NAMESPACE_END }

AUD_NAMESPACE_BEGIN

/**
 * The format of a sample.
 * The last 4 bit save the byte count of the format.
 */
typedef enum
{
	FORMAT_INVALID = 0x00,		/// Invalid sample format.
	FORMAT_U8      = 0x01,		/// 1 byte unsigned byte.
	FORMAT_S16     = 0x12,		/// 2 byte signed integer.
	FORMAT_S24     = 0x13,		/// 3 byte signed integer.
	FORMAT_S32     = 0x14,		/// 4 byte signed integer.
	FORMAT_FLOAT32 = 0x24,		/// 4 byte float.
	FORMAT_FLOAT64 = 0x28		/// 8 byte float.
} SampleFormat;

/// The channel count.
typedef enum
{
	CHANNELS_INVALID    = 0,	/// Invalid channel count.
	CHANNELS_MONO       = 1,	/// Mono.
	CHANNELS_STEREO     = 2,	/// Stereo.
	CHANNELS_STEREO_LFE = 3,	/// Stereo with LFE channel.
	CHANNELS_SURROUND4  = 4,	/// 4 channel surround sound.
	CHANNELS_SURROUND5  = 5,	/// 5 channel surround sound.
	CHANNELS_SURROUND51 = 6,	/// 5.1 surround sound.
	CHANNELS_SURROUND61 = 7,	/// 6.1 surround sound.
	CHANNELS_SURROUND71 = 8	/// 7.1 surround sound.
} Channels;

/// The channel names.
typedef enum
{
	CHANNEL_FRONT_LEFT = 0,
	CHANNEL_FRONT_RIGHT,
	CHANNEL_FRONT_CENTER,
	CHANNEL_LFE,
	CHANNEL_REAR_LEFT,
	CHANNEL_REAR_RIGHT,
	CHANNEL_REAR_CENTER,
	CHANNEL_SIDE_LEFT,
	CHANNEL_SIDE_RIGHT,
	CHANNEL_MAX
} Channel;

/**
 * The sample rate tells how many samples are played back within one second.
 * Some exotic formats may use other sample rates than provided here.
 */
typedef enum
{
	RATE_INVALID = 0,			/// Invalid sample rate.
	RATE_8000    = 8000,		/// 8000 Hz.
	RATE_16000   = 16000,		/// 16000 Hz.
	RATE_11025   = 11025,		/// 11025 Hz.
	RATE_22050   = 22050,		/// 22050 Hz.
	RATE_32000   = 32000,		/// 32000 Hz.
	RATE_44100   = 44100,		/// 44100 Hz.
	RATE_48000   = 48000,		/// 48000 Hz.
	RATE_88200   = 88200,		/// 88200 Hz.
	RATE_96000   = 96000,		/// 96000 Hz.
	RATE_192000  = 192000		/// 192000 Hz.
} DefaultSampleRate;

/// Status of a playback handle.
typedef enum
{
	STATUS_INVALID = 0,			/// Invalid handle. Maybe due to stopping.
	STATUS_PLAYING,				/// Sound is playing.
	STATUS_PAUSED,				/// Sound is being paused.
	STATUS_STOPPED				/// Sound is stopped but kept in the device.
} Status;

/// Error codes for exceptions (C++ library) or for return values (C API).
typedef enum
{
	NO_ERROR = 0,
	ERROR_SPECS,
	ERROR_PROPS,
	ERROR_FILE,
	ERROR_SRC,
	ERROR_FFMPEG,
	ERROR_OPENAL,
	ERROR_SDL,
	ERROR_JACK
} Error;

/// Fading types.
typedef enum
{
	FADE_IN,
	FADE_OUT
} FadeType;

/// Possible distance models for the 3D device.
typedef enum
{
	DISTANCE_MODEL_INVALID = 0,
	DISTANCE_MODEL_INVERSE,
	DISTANCE_MODEL_INVERSE_CLAMPED,
	DISTANCE_MODEL_LINEAR,
	DISTANCE_MODEL_LINEAR_CLAMPED,
	DISTANCE_MODEL_EXPONENT,
	DISTANCE_MODEL_EXPONENT_CLAMPED
} DistanceModel;

/// Possible animatable properties for Sequencer Factories and Entries.
typedef enum
{
	AP_VOLUME,
	AP_PANNING,
	AP_PITCH,
	AP_LOCATION,
	AP_ORIENTATION
} AnimateablePropertyType;

/// Container formats for writers.
typedef enum
{
	CONTAINER_INVALID = 0,
	CONTAINER_AC3,
	CONTAINER_FLAC,
	CONTAINER_MATROSKA,
	CONTAINER_MP2,
	CONTAINER_MP3,
	CONTAINER_OGG,
	CONTAINER_WAV
} Container;

/// Audio codecs for writers.
typedef enum
{
	CODEC_INVALID = 0,
	CODEC_AAC,
	CODEC_AC3,
	CODEC_FLAC,
	CODEC_MP2,
	CODEC_MP3,
	CODEC_PCM,
	CODEC_VORBIS,
	CODEC_OPUS
} Codec;

/// Sample type.(float samples)
typedef float sample_t;

/// Sample data type (format samples)
typedef unsigned char data_t;

/// Sample rate type.
typedef double SampleRate;

/// Specification of a sound source.
typedef struct
{
	/// Sample rate in Hz.
	SampleRate rate;

	/// Channel count.
	Channels channels;
} Specs;

/// Specification of a sound device.
typedef struct
{
	/// Sample format.
	SampleFormat format;

	union
	{
		struct
		{
			/// Sample rate in Hz.
			SampleRate rate;

			/// Channel count.
			Channels channels;
		};
		Specs specs;
	};
} DeviceSpecs;

/// Exception structure.
typedef struct
{
	/**
	 * Error code.
	 * \see Error
	 */
	Error error;

	/**
	 * Error string.
	 */
	const char* str;

	// void* userData; - for the case it is needed someday
} Exception;

AUD_NAMESPACE_END
