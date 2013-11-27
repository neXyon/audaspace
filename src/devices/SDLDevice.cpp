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

#include "devices/SDLDevice.h"
#include "IReader.h"

AUD_NAMESPACE_BEGIN

void SDLDevice::SDL_mix(void *data, Uint8* buffer, int length)
{
	SDLDevice* device = (SDLDevice*)data;

	device->mix((data_t*)buffer,length/AUD_DEVICE_SAMPLE_SIZE(device->m_specs));
}

static const char* open_error = "SDLDevice: Device couldn't be opened.";
static const char* format_error = "SDLDevice: Obtained unsupported sample "
								  "format.";

SDLDevice::SDLDevice(DeviceSpecs specs, int buffersize)
{
	if(specs.channels == CHANNELS_INVALID)
		specs.channels = CHANNELS_STEREO;
	if(specs.format == FORMAT_INVALID)
		specs.format = FORMAT_S16;
	if(specs.rate == RATE_INVALID)
		specs.rate = RATE_44100;

	m_specs = specs;

	SDL_AudioSpec format, obtained;

	format.freq = m_specs.rate;
	if(m_specs.format == FORMAT_U8)
		format.format = AUDIO_U8;
	else
		format.format = AUDIO_S16SYS;
	format.channels = m_specs.channels;
	format.samples = buffersize;
	format.callback = SDLDevice::SDL_mix;
	format.userdata = this;

	if(SDL_OpenAudio(&format, &obtained) != 0)
		AUD_THROW(ERROR_SDL, open_error);

	m_specs.rate = (SampleRate)obtained.freq;
	m_specs.channels = (Channels)obtained.channels;
	if(obtained.format == AUDIO_U8)
		m_specs.format = FORMAT_U8;
	else if(obtained.format == AUDIO_S16LSB || obtained.format == AUDIO_S16MSB)
		m_specs.format = FORMAT_S16;
	else
	{
		SDL_CloseAudio();
		AUD_THROW(ERROR_SDL, format_error);
	}

	create();
}

SDLDevice::~SDLDevice()
{
	lock();
	SDL_CloseAudio();
	unlock();

	destroy();
}

void SDLDevice::playing(bool playing)
{
	SDL_PauseAudio(playing ? 0 : 1);
}

AUD_NAMESPACE_END
