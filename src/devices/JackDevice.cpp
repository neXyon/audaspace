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

#include "devices/JackDevice.h"
#include "IReader.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

void* JackDevice::runMixingThread(void* device)
{
	((JackDevice*)device)->updateRingBuffers();
	return nullptr;
}

void JackDevice::updateRingBuffers()
{
	size_t size, temp;
	unsigned int samplesize = AUD_SAMPLE_SIZE(m_specs);
	unsigned int i, j;
	unsigned int channels = m_specs.channels;
	sample_t* buffer = m_buffer.getBuffer();
	float* deinterleave = m_deinterleavebuf.getBuffer();
	jack_transport_state_t state;
	jack_position_t position;

	pthread_mutex_lock(&m_mixingLock);
	while(m_valid)
	{
		if(m_sync > 1)
		{
			if(m_syncFunc)
			{
				state = aud_jack_transport_query(m_client, &position);
				m_syncFunc(m_syncFuncData, state != JackTransportStopped, position.frame / (float) m_specs.rate);
			}

			for(i = 0; i < channels; i++)
				aud_jack_ringbuffer_reset(m_ringbuffers[i]);
		}

		size = aud_jack_ringbuffer_write_space(m_ringbuffers[0]);
		for(i = 1; i < channels; i++)
			if((temp = aud_jack_ringbuffer_write_space(m_ringbuffers[i])) < size)
				size = temp;

		while(size > samplesize)
		{
			size /= samplesize;
			mix((data_t*)buffer, size);
			for(i = 0; i < channels; i++)
			{
				for(j = 0; j < size; j++)
					deinterleave[i * size + j] = buffer[i + j * channels];
				aud_jack_ringbuffer_write(m_ringbuffers[i], (char*)(deinterleave + i * size), size * sizeof(float));
			}

			size = aud_jack_ringbuffer_write_space(m_ringbuffers[0]);
			for(i = 1; i < channels; i++)
				if((temp = aud_jack_ringbuffer_write_space(m_ringbuffers[i])) < size)
					size = temp;
		}

		if(m_sync > 1)
		{
			m_sync = 3;
		}

		pthread_cond_wait(&m_mixingCondition, &m_mixingLock);
	}
	pthread_mutex_unlock(&m_mixingLock);
}

int JackDevice::jack_mix(jack_nframes_t length, void *data)
{
	JackDevice* device = (JackDevice*)data;
	unsigned int i;
	int count = device->m_specs.channels;
	char* buffer;

	if(device->m_sync)
	{
		// play silence while syncing
		for(unsigned int i = 0; i < count; i++)
			std::memset(aud_jack_port_get_buffer(device->m_ports[i], length), 0, length * sizeof(float));
	}
	else
	{
		size_t temp;
		size_t readsamples = aud_jack_ringbuffer_read_space(device->m_ringbuffers[0]);
		for(i = 1; i < count; i++)
			if((temp = aud_jack_ringbuffer_read_space(device->m_ringbuffers[i])) < readsamples)
				readsamples = temp;

		readsamples = AUD_MIN(readsamples / sizeof(float), length);

		for(unsigned int i = 0; i < count; i++)
		{
			buffer = (char*)aud_jack_port_get_buffer(device->m_ports[i], length);
			aud_jack_ringbuffer_read(device->m_ringbuffers[i], buffer, readsamples * sizeof(float));
			if(readsamples < length)
				std::memset(buffer + readsamples * sizeof(float), 0, (length - readsamples) * sizeof(float));
		}

		if(pthread_mutex_trylock(&(device->m_mixingLock)) == 0)
		{
			pthread_cond_signal(&(device->m_mixingCondition));
			pthread_mutex_unlock(&(device->m_mixingLock));
		}
	}

	return 0;
}

int JackDevice::jack_sync(jack_transport_state_t state, jack_position_t* pos, void* data)
{
	JackDevice* device = (JackDevice*)data;

	if(state == JackTransportStopped)
		return 1;

	if(pthread_mutex_trylock(&(device->m_mixingLock)) == 0)
	{
		if(device->m_sync > 2)
		{
			if(device->m_sync == 3)
			{
				device->m_sync = 0;
				pthread_mutex_unlock(&(device->m_mixingLock));
				return 1;
			}
		}
		else
		{
			device->m_sync = 2;
			pthread_cond_signal(&(device->m_mixingCondition));
		}
		pthread_mutex_unlock(&(device->m_mixingLock));
	}
	else if(!device->m_sync)
		device->m_sync = 1;

	return 0;
}

void JackDevice::jack_shutdown(void *data)
{
	JackDevice* device = (JackDevice*)data;
	device->m_valid = false;
}

static const char* clientopen_error = "JackDevice: Couldn't connect to "
									  "jack server.";
static const char* port_error = "JackDevice: Couldn't create output port.";
static const char* activate_error = "JackDevice: Couldn't activate the "
									"client.";

JackDevice::JackDevice(std::string name, DeviceSpecs specs, int buffersize)
{
	if(specs.channels == CHANNELS_INVALID)
		specs.channels = CHANNELS_STEREO;

	// jack uses floats
	m_specs = specs;
	m_specs.format = FORMAT_FLOAT32;

	jack_options_t options = JackNullOption;
	jack_status_t status;

	// open client
	m_client = aud_jack_client_open(name.c_str(), options, &status);
	if(m_client == nullptr)
		AUD_THROW(ERROR_JACK, clientopen_error);

	// set callbacks
	aud_jack_set_process_callback(m_client, JackDevice::jack_mix, this);
	aud_jack_on_shutdown(m_client, JackDevice::jack_shutdown, this);
	aud_jack_set_sync_callback(m_client, JackDevice::jack_sync, this);

	// register our output channels which are called ports in jack
	m_ports = new jack_port_t*[m_specs.channels];

	try
	{
		char portname[64];
		for(int i = 0; i < m_specs.channels; i++)
		{
			sprintf(portname, "out %d", i+1);
			m_ports[i] = aud_jack_port_register(m_client, portname,
											JACK_DEFAULT_AUDIO_TYPE,
											JackPortIsOutput, 0);
			if(m_ports[i] == nullptr)
				AUD_THROW(ERROR_JACK, port_error);
		}
	}
	catch(Exception&)
	{
		aud_jack_client_close(m_client);
		delete[] m_ports;
		throw;
	}

	m_specs.rate = (SampleRate)aud_jack_get_sample_rate(m_client);

	buffersize *= sizeof(sample_t);
	m_ringbuffers = new jack_ringbuffer_t*[specs.channels];
	for(unsigned int i = 0; i < specs.channels; i++)
		m_ringbuffers[i] = aud_jack_ringbuffer_create(buffersize);
	buffersize *= specs.channels;
	m_deinterleavebuf.resize(buffersize);
	m_buffer.resize(buffersize);

	create();

	m_valid = true;
	m_sync = 0;
	m_syncFunc = nullptr;
	m_nextState = m_state = aud_jack_transport_query(m_client, nullptr);

	pthread_mutex_init(&m_mixingLock, nullptr);
	pthread_cond_init(&m_mixingCondition, nullptr);

	// activate the client
	if(aud_jack_activate(m_client))
	{
		aud_jack_client_close(m_client);
		delete[] m_ports;
		for(unsigned int i = 0; i < specs.channels; i++)
			aud_jack_ringbuffer_free(m_ringbuffers[i]);
		delete[] m_ringbuffers;
		pthread_mutex_destroy(&m_mixingLock);
		pthread_cond_destroy(&m_mixingCondition);
		destroy();

		AUD_THROW(ERROR_JACK, activate_error);
	}

	const char** ports = aud_jack_get_ports(m_client, nullptr, nullptr,
										JackPortIsPhysical | JackPortIsInput);
	if(ports != nullptr)
	{
		for(int i = 0; i < m_specs.channels && ports[i]; i++)
			aud_jack_connect(m_client, aud_jack_port_name(m_ports[i]), ports[i]);

		free(ports);
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pthread_create(&m_mixingThread, &attr, runMixingThread, this);

	pthread_attr_destroy(&attr);
}

JackDevice::~JackDevice()
{
	if(m_valid)
		aud_jack_client_close(m_client);
	m_valid = false;

	delete[] m_ports;

	pthread_mutex_lock(&m_mixingLock);
	pthread_cond_signal(&m_mixingCondition);
	pthread_mutex_unlock(&m_mixingLock);
	pthread_join(m_mixingThread, nullptr);

	pthread_cond_destroy(&m_mixingCondition);
	pthread_mutex_destroy(&m_mixingLock);
	for(unsigned int i = 0; i < m_specs.channels; i++)
		aud_jack_ringbuffer_free(m_ringbuffers[i]);
	delete[] m_ringbuffers;

	destroy();
}

void JackDevice::playing(bool playing)
{
	// Do nothing.
}

void JackDevice::startPlayback()
{
	aud_jack_transport_start(m_client);
	m_nextState = JackTransportRolling;
}

void JackDevice::stopPlayback()
{
	aud_jack_transport_stop(m_client);
	m_nextState = JackTransportStopped;
}

void JackDevice::seekPlayback(float time)
{
	if(time >= 0.0f)
		aud_jack_transport_locate(m_client, time * m_specs.rate);
}

void JackDevice::setSyncCallback(syncFunction sync, void* data)
{
	m_syncFunc = sync;
	m_syncFuncData = data;
}

float JackDevice::getPlaybackPosition()
{
	jack_position_t position;
	aud_jack_transport_query(m_client, &position);
	return position.frame / (float) m_specs.rate;
}

bool JackDevice::doesPlayback()
{
	jack_transport_state_t state = aud_jack_transport_query(m_client, nullptr);

	if(state != m_state)
		m_nextState = m_state = state;

	return m_nextState != JackTransportStopped;
}

AUD_NAMESPACE_END
