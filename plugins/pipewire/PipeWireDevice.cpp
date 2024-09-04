/*******************************************************************************
 * Copyright 2009-2024 Jörg Müller
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#include "PipeWireDevice.h"

#include <spa/param/audio/format-utils.h>

#include "Exception.h"
#include "IReader.h"
#include "PipeWireLibrary.h"

#include "devices/DeviceManager.h"
#include "devices/IDeviceFactory.h"

AUD_NAMESPACE_BEGIN

PipeWireDevice::PipeWireSynchronizer::PipeWireSynchronizer(PipeWireDevice* device) : m_device(device)
{
}

void PipeWireDevice::PipeWireSynchronizer::update_tick_start()
{
	pw_time tm;
	AUD_pw_stream_get_time_n(m_device->m_stream, &tm, sizeof(tm));
	m_tick_start = tm.ticks;
	m_seek_pos = m_timeline_pos;
}

void PipeWireDevice::PipeWireSynchronizer::seek(std::shared_ptr<IHandle> handle, double time)
{
	pw_time tm;
	AUD_pw_stream_get_time_n(m_device->m_stream, &tm, sizeof(tm));
	m_tick_start = tm.ticks;
	m_seek_pos = m_timeline_pos = time;
	handle->seek(time);
}

double PipeWireDevice::PipeWireSynchronizer::getPosition(std::shared_ptr<IHandle> handle)
{
	pw_time tm;
	AUD_pw_stream_get_time_n(m_device->m_stream, &tm, sizeof(tm));
	uint64_t now = AUD_pw_stream_get_nsec(m_device->m_stream);
	int64_t diff = now - tm.now;
	int64_t elapsed = (tm.rate.denom * diff) / (tm.rate.num * SPA_NSEC_PER_SEC);

	/* Calculate the elapsed time in seconds from the last seek position. */
	double elapsed_time = (tm.ticks - m_tick_start + elapsed) * tm.rate.num / double(tm.rate.denom);
	/* Convert to seconds and add the last seek position timestamp. */
	m_timeline_pos = elapsed_time + m_seek_pos;
	return m_timeline_pos;
}

void PipeWireDevice::handle_state_changed(void* device_ptr, enum pw_stream_state old, enum pw_stream_state state, const char* error)
{
	PipeWireDevice* device = (PipeWireDevice*) device_ptr;
	// fprintf(stderr, "stream state: \"%s\"\n", pw_stream_state_as_string(state));
	if(state == PW_STREAM_STATE_STREAMING)
	{
		/* When we activate/unpause the stream, we need to make sure that
		 * we update the refrence tick number as the ticks we get from PipeWire
		 * might have continued to count up after we paused the stream.
		 */
		device->m_synchronizer.update_tick_start();
	}
}

void PipeWireDevice::mix_audio_buffer(void* device_ptr)
{
	PipeWireDevice* device = (PipeWireDevice*) device_ptr;
	pw_buffer* pw_buf = AUD_pw_stream_dequeue_buffer(device->m_stream);
	if(!pw_buf)
	{
		/* Couldn't get any buffer from PipeWire...*/
		return;
	}

	spa_data& data = pw_buf->buffer->datas[0];
	if(!data.data)
	{
		return;
	}

	spa_chunk* chunk = data.chunk;
	if(!chunk)
	{
		return;
	}

	chunk->offset = 0;
	chunk->stride = AUD_DEVICE_SAMPLE_SIZE(device->m_specs);
	int n_frames = data.maxsize / chunk->stride;
	if(pw_buf->requested)
	{
		n_frames = SPA_MIN(pw_buf->requested, n_frames);
	}
	chunk->size = n_frames * chunk->stride;

	if(!device->m_playback)
	{
		memset(data.data, 0, AUD_FORMAT_SIZE(device->m_specs.format) * chunk->size);
	}
	else
	{
		device->mix((data_t*) data.data, n_frames);
	}

	AUD_pw_stream_queue_buffer(device->m_stream, pw_buf);
}

void PipeWireDevice::playing(bool playing)
{
	AUD_pw_thread_loop_lock(m_thread);
	AUD_pw_stream_set_active(m_stream, playing);
	AUD_pw_thread_loop_unlock(m_thread);

	m_playback = playing;
}

PipeWireDevice::PipeWireDevice(const std::string& name, DeviceSpecs specs, int buffersize) : m_synchronizer(this), m_playback(false)
{
	if(specs.channels == CHANNELS_INVALID)
		specs.channels = CHANNELS_STEREO;
	if(specs.format == FORMAT_INVALID)
		specs.format = FORMAT_FLOAT32;
	if(specs.rate == RATE_INVALID)
		specs.rate = RATE_48000;

	m_specs = specs;
	spa_audio_format format = SPA_AUDIO_FORMAT_F32;
	switch(m_specs.format)
	{
	case FORMAT_U8:
		format = SPA_AUDIO_FORMAT_U8;
		break;
	case FORMAT_S16:
		format = SPA_AUDIO_FORMAT_S16;
		break;
	case FORMAT_S24:
		format = SPA_AUDIO_FORMAT_S24;
		break;
	case FORMAT_S32:
		format = SPA_AUDIO_FORMAT_S32;
		break;
	case FORMAT_FLOAT32:
		format = SPA_AUDIO_FORMAT_F32;
		break;
	case FORMAT_FLOAT64:
		format = SPA_AUDIO_FORMAT_F64;
		break;
	default:
		break;
	}

	AUD_pw_init(nullptr, nullptr);

	m_thread = AUD_pw_thread_loop_new(name.c_str(), nullptr);
	if(!m_thread)
	{
		AUD_THROW(DeviceException, "Could not create PipeWire thread.");
	}

	m_events = std::make_unique<pw_stream_events>();
	m_events->version = PW_VERSION_STREAM_EVENTS;
	m_events->state_changed = PipeWireDevice::handle_state_changed;
	m_events->process = PipeWireDevice::mix_audio_buffer;

	pw_properties *stream_props = AUD_pw_properties_new(
				PW_KEY_MEDIA_TYPE, "Audio",
				PW_KEY_MEDIA_CATEGORY, "Playback",
				PW_KEY_MEDIA_ROLE, "Production",
				NULL);

	/* Set the requested sample rate and latency. */
	AUD_pw_properties_setf(stream_props, PW_KEY_NODE_RATE, "1/%u", uint(m_specs.rate));
	AUD_pw_properties_setf(stream_props, PW_KEY_NODE_LATENCY, "%u/%u", buffersize, uint(m_specs.rate));

	m_stream = AUD_pw_stream_new_simple(
			AUD_pw_thread_loop_get_loop(m_thread),
			name.c_str(),
			stream_props,
			m_events.get(),
			this);
	if(!m_stream)
	{
		AUD_pw_thread_loop_destroy(m_thread);
		AUD_THROW(DeviceException, "Could not create PipeWire stream.");
	}

	spa_audio_info_raw info{};
	info.channels = m_specs.channels;
	info.format = format;
	info.rate = m_specs.rate;

	const spa_pod *params[1];
	uint8_t buffer[1024];
	spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
	params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);

	AUD_pw_stream_connect(m_stream,
			  PW_DIRECTION_OUTPUT,
			  PW_ID_ANY,
			  static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT |
			  PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_INACTIVE |
			  PW_STREAM_FLAG_RT_PROCESS),
			  params, 1);
	AUD_pw_thread_loop_start(m_thread);

	create();
}

PipeWireDevice::~PipeWireDevice()
{
	/* Ensure that we are not playing back anything anymore. */
	destroy();

	/* Destruct all PipeWire data. */
	AUD_pw_thread_loop_stop(m_thread);
	AUD_pw_stream_destroy(m_stream);
	AUD_pw_thread_loop_destroy(m_thread);
	AUD_pw_deinit();
}

ISynchronizer* PipeWireDevice::getSynchronizer()
{
	return &m_synchronizer;
}

class PipeWireDeviceFactory : public IDeviceFactory
{
private:
	DeviceSpecs m_specs;
	int m_buffersize;
	std::string m_name;

public:
	PipeWireDeviceFactory() : m_buffersize(AUD_DEFAULT_BUFFER_SIZE)
	{
		m_specs.format = FORMAT_S16;
		m_specs.channels = CHANNELS_STEREO;
		m_specs.rate = RATE_48000;
	}

	virtual std::shared_ptr<IDevice> openDevice()
	{
		return std::shared_ptr<IDevice>(new PipeWireDevice(m_name, m_specs, m_buffersize));
	}

	virtual int getPriority()
	{
		return 1 << 16;
	}

	virtual void setSpecs(DeviceSpecs specs)
	{
		m_specs = specs;
	}

	virtual void setBufferSize(int buffersize)
	{
		m_buffersize = buffersize;
	}

	virtual void setName(const std::string &name)
	{
		m_name = name;
	}
};

void PipeWireDevice::registerPlugin()
{
	if(loadPipeWire())
		DeviceManager::registerDevice("PipeWire", std::shared_ptr<IDeviceFactory>(new PipeWireDeviceFactory));
}

#ifdef PIPEWIRE_PLUGIN
extern "C" AUD_PLUGIN_API void registerPlugin()
{
	PipeWireDevice::registerPlugin();
}

extern "C" AUD_PLUGIN_API const char* getName()
{
	return "Pipewire";
}
#endif

AUD_NAMESPACE_END
