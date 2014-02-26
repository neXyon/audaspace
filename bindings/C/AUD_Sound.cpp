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

#include "file/File.h"
#include "util/StreamBuffer.h"
#include "fx/Delay.h"
#include "fx/Limiter.h"
#include "sequence/PingPong.h"
#include "fx/Loop.h"
#include "fx/Envelope.h"
#include "respec/LinearResample.h"
#include "fx/Lowpass.h"
#include "fx/Highpass.h"
#include "fx/Accumulator.h"
#include "fx/Sum.h"
#include "fx/Square.h"
#include "respec/ChannelMapper.h"
#include "util/Buffer.h"
#include "Exception.h"

#include <cassert>

using namespace aud;

#define AUD_CAPI_IMPLEMENTATION
#include "AUD_Sound.h"

AUD_Sound *AUD_load(const char *filename)
{
	assert(filename);
	return new AUD_Sound(new File(filename));
}

AUD_Sound *AUD_loadBuffer(unsigned char *buffer, int size)
{
	assert(buffer);
	return new AUD_Sound(new File(buffer, size));
}

AUD_Sound *AUD_bufferSound(AUD_Sound *sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new StreamBuffer(*sound));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_monoSound(AUD_Sound *sound)
{
	assert(sound);

	try
	{
		DeviceSpecs specs;
		specs.channels = CHANNELS_MONO;
		specs.rate = RATE_INVALID;
		specs.format = FORMAT_INVALID;
		return new AUD_Sound(new ChannelMapper(*sound, specs));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_delaySound(AUD_Sound *sound, float delay)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Delay(*sound, delay));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_limitSound(AUD_Sound *sound, float start, float end)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Limiter(*sound, start, end));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_pingpongSound(AUD_Sound *sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new PingPong(*sound));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_loopSound(AUD_Sound *sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Loop(*sound));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_squareSound(AUD_Sound *sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Square(*sound));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

void AUD_unload(AUD_Sound *sound)
{
	assert(sound);
	delete sound;
}

AUD_Sound *AUD_copy(AUD_Sound *sound)
{
	return new std::shared_ptr<ISound>(*sound);
}
