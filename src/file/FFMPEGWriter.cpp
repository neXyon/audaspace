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

#include "FFMPEGWriter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
}

AUD_NAMESPACE_BEGIN

static const char* context_error = "FFMPEGWriter: Couldn't allocate context.";
static const char* codec_error = "FFMPEGWriter: Invalid codec or codec not found.";
static const char* stream_error = "FFMPEGWriter: Couldn't allocate stream.";
static const char* format_error = "FFMPEGWriter: Unsupported sample format.";
static const char* file_error = "FFMPEGWriter: File couldn't be written.";
static const char* write_error = "FFMPEGWriter: Error writing packet.";

FFMPEGWriter::FFMPEGWriter(std::string filename, DeviceSpecs specs, Container format, Codec codec, unsigned int bitrate) :
	m_position(0),
	m_specs(specs),
	m_input_samples(0)
{
	av_register_all(); // AUD_XXX

	static const char* formats[] = { NULL, "ac3", "flac", "matroska", "mp2", "mp3", "ogg", "wav" };

	m_formatCtx = avformat_alloc_context();
	if (!m_formatCtx) AUD_THROW(ERROR_FFMPEG, context_error);

	strcpy(m_formatCtx->filename, filename.c_str());
	m_outputFmt = m_formatCtx->oformat = av_guess_format(formats[format], filename.c_str(), NULL);
	if (!m_outputFmt) {
		avformat_free_context(m_formatCtx);
		AUD_THROW(ERROR_FFMPEG, context_error);
	}

	switch(codec)
	{
	case CODEC_AAC:
		m_outputFmt->audio_codec = CODEC_ID_AAC;
		break;
	case CODEC_AC3:
		m_outputFmt->audio_codec = CODEC_ID_AC3;
		break;
	case CODEC_FLAC:
		m_outputFmt->audio_codec = CODEC_ID_FLAC;
		break;
	case CODEC_MP2:
		m_outputFmt->audio_codec = CODEC_ID_MP2;
		break;
	case CODEC_MP3:
		m_outputFmt->audio_codec = CODEC_ID_MP3;
		break;
	case CODEC_PCM:
		switch(specs.format)
		{
		case FORMAT_U8:
			m_outputFmt->audio_codec = CODEC_ID_PCM_U8;
			break;
		case FORMAT_S16:
			m_outputFmt->audio_codec = CODEC_ID_PCM_S16LE;
			break;
		case FORMAT_S24:
			m_outputFmt->audio_codec = CODEC_ID_PCM_S24LE;
			break;
		case FORMAT_S32:
			m_outputFmt->audio_codec = CODEC_ID_PCM_S32LE;
			break;
		case FORMAT_FLOAT32:
			m_outputFmt->audio_codec = CODEC_ID_PCM_F32LE;
			break;
		case FORMAT_FLOAT64:
			m_outputFmt->audio_codec = CODEC_ID_PCM_F64LE;
			break;
		default:
			m_outputFmt->audio_codec = CODEC_ID_NONE;
			break;
		}
		break;
	case CODEC_VORBIS:
		m_outputFmt->audio_codec = CODEC_ID_VORBIS;
		break;
	default:
		m_outputFmt->audio_codec = CODEC_ID_NONE;
		break;
	}

	try
	{
		if(m_outputFmt->audio_codec == CODEC_ID_NONE)
			AUD_THROW(ERROR_SPECS, codec_error);

		m_stream = avformat_new_stream(m_formatCtx, NULL);
		if(!m_stream)
			AUD_THROW(ERROR_FFMPEG, stream_error);

		m_codecCtx = m_stream->codec;
		m_codecCtx->codec_id = m_outputFmt->audio_codec;
		m_codecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
		m_codecCtx->bit_rate = bitrate;
		m_codecCtx->sample_rate = int(m_specs.rate);
		m_codecCtx->channels = m_specs.channels;
		m_codecCtx->time_base.num = 1;
		m_codecCtx->time_base.den = m_codecCtx->sample_rate;

		switch(m_specs.format)
		{
		case FORMAT_U8:
			m_convert = convert_float_u8;
			m_codecCtx->sample_fmt = AV_SAMPLE_FMT_U8;
			break;
		case FORMAT_S16:
			m_convert = convert_float_s16;
			m_codecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
			break;
		case FORMAT_S32:
			m_convert = convert_float_s32;
			m_codecCtx->sample_fmt = AV_SAMPLE_FMT_S32;
			break;
		case FORMAT_FLOAT32:
			m_convert = convert_copy<float>;
			m_codecCtx->sample_fmt = AV_SAMPLE_FMT_FLT;
			break;
		case FORMAT_FLOAT64:
			m_convert = convert_float_double;
			m_codecCtx->sample_fmt = AV_SAMPLE_FMT_DBL;
			break;
		default:
			AUD_THROW(ERROR_FFMPEG, format_error);
		}

		try
		{
			if(m_formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
				m_codecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

			AVCodec* codec = avcodec_find_encoder(m_codecCtx->codec_id);
			if(!codec)
				AUD_THROW(ERROR_FFMPEG, codec_error);

			if(avcodec_open2(m_codecCtx, codec, NULL))
				AUD_THROW(ERROR_FFMPEG, codec_error);

			m_output_buffer.resize(FF_MIN_BUFFER_SIZE);
			int samplesize = AUD_MAX(AUD_SAMPLE_SIZE(m_specs), AUD_DEVICE_SAMPLE_SIZE(m_specs));

			if(m_codecCtx->frame_size <= 1)
				m_input_size = 0;
			else
			{
				m_input_buffer.resize(m_codecCtx->frame_size * samplesize);
				m_input_size = m_codecCtx->frame_size;
			}

			try
			{
				if(avio_open(&m_formatCtx->pb, filename.c_str(), AVIO_FLAG_WRITE))
					AUD_THROW(ERROR_FILE, file_error);

				avformat_write_header(m_formatCtx, NULL);
			}
			catch(Exception&)
			{
				avcodec_close(m_codecCtx);
				av_freep(&m_formatCtx->streams[0]->codec);
				throw;
			}
		}
		catch(Exception&)
		{
			av_freep(&m_formatCtx->streams[0]);
			throw;
		}
	}
	catch(Exception&)
	{
		av_free(m_formatCtx);
		throw;
	}
}

FFMPEGWriter::~FFMPEGWriter()
{
	// writte missing data
	if(m_input_samples)
	{
		sample_t* buf = m_input_buffer.getBuffer();
		memset(buf + m_specs.channels * m_input_samples, 0,
			   (m_input_size - m_input_samples) * AUD_DEVICE_SAMPLE_SIZE(m_specs));

		encode(buf);
	}

	av_write_trailer(m_formatCtx);

	avcodec_close(m_codecCtx);

	av_freep(&m_formatCtx->streams[0]->codec);
	av_freep(&m_formatCtx->streams[0]);

	avio_close(m_formatCtx->pb);
	av_free(m_formatCtx);
}

int FFMPEGWriter::getPosition() const
{
	return m_position;
}

DeviceSpecs FFMPEGWriter::getSpecs() const
{
	return m_specs;
}

void FFMPEGWriter::encode(sample_t* data)
{
	sample_t* outbuf = m_output_buffer.getBuffer();

	// convert first
	if(m_input_size)
		m_convert(reinterpret_cast<data_t*>(data), reinterpret_cast<data_t*>(data), m_input_size * m_specs.channels);

	AVPacket packet;
	av_init_packet(&packet);
	packet.size = avcodec_encode_audio(m_codecCtx, reinterpret_cast<uint8_t*>(outbuf), m_output_buffer.getSize(), reinterpret_cast<short*>(data));
	if(m_codecCtx->coded_frame && m_codecCtx->coded_frame->pts != AV_NOPTS_VALUE)
		packet.pts = av_rescale_q(m_codecCtx->coded_frame->pts, m_codecCtx->time_base, m_stream->time_base);
	packet.flags |= AV_PKT_FLAG_KEY;
	packet.stream_index = m_stream->index;
	packet.data = reinterpret_cast<uint8_t*>(outbuf);

	if(av_interleaved_write_frame(m_formatCtx, &packet))
		AUD_THROW(ERROR_FFMPEG, write_error);
}

void FFMPEGWriter::write(unsigned int length, sample_t* buffer)
{
	unsigned int samplesize = AUD_SAMPLE_SIZE(m_specs);

	if(m_input_size)
	{
		sample_t* inbuf = m_input_buffer.getBuffer();

		while(length)
		{
			unsigned int len = AUD_MIN(m_input_size - m_input_samples, length);

			memcpy(inbuf + m_input_samples * m_specs.channels, buffer, len * samplesize);

			buffer += len * m_specs.channels;
			m_input_samples += len;
			m_position += len;
			length -= len;

			if(m_input_samples == m_input_size)
			{
				encode(inbuf);

				m_input_samples = 0;
			}
		}
	}
	else // PCM data, can write directly!
	{
		int samplesize = AUD_SAMPLE_SIZE(m_specs);
		if(m_output_buffer.getSize() != length * m_specs.channels * m_codecCtx->bits_per_coded_sample / 8)
			m_output_buffer.resize(length * m_specs.channels * m_codecCtx->bits_per_coded_sample / 8);
		m_input_buffer.assureSize(length * AUD_MAX(AUD_DEVICE_SAMPLE_SIZE(m_specs), samplesize));

		sample_t* buf = m_input_buffer.getBuffer();
		m_convert(reinterpret_cast<data_t*>(buf), reinterpret_cast<data_t*>(buffer), length * m_specs.channels);

		encode(buf);

		m_position += length;
	}
}

AUD_NAMESPACE_END
