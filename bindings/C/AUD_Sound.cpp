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

#include "generator/Sawtooth.h"
#include "generator/Sine.h"
#include "generator/Silence.h"
#include "generator/Square.h"
#include "generator/Triangle.h"
#include "file/File.h"
#include "util/StreamBuffer.h"
#include "fx/Accumulator.h"
#include "fx/ADSR.h"
#include "fx/Delay.h"
#include "fx/Envelope.h"
#include "fx/Fader.h"
#include "fx/Highpass.h"
#include "fx/IIRFilter.h"
#include "fx/Limiter.h"
#include "fx/Loop.h"
#include "fx/Lowpass.h"
#include "fx/Pitch.h"
#include "fx/Reverse.h"
#include "fx/Sum.h"
#include "fx/Threshold.h"
#include "fx/Volume.h"
#include "sequence/Double.h"
#include "sequence/Superpose.h"
#include "sequence/PingPong.h"
#include "respec/LinearResample.h"
#include "respec/ChannelMapper.h"
#include "util/Buffer.h"
#include "Exception.h"

#include <cassert>

using namespace aud;

#define AUD_CAPI_IMPLEMENTATION
#include "AUD_Sound.h"

AUD_Sound* AUD_Sound_bufferFile(unsigned char* buffer, int size)
{
	assert(buffer);
	return new AUD_Sound(new File(buffer, size));
}

AUD_Sound* AUD_Sound_cache(AUD_Sound* sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new StreamBuffer(*sound));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_file(const char* filename)
{
	assert(filename);
	return new AUD_Sound(new File(filename));
}

AUD_Sound* AUD_Sound_sawtooth(float frequency, SampleRate rate)
{
	return new AUD_Sound(new Sawtooth(frequency, rate));
}

AUD_Sound*AUD_Sound_silence()
{
	return new AUD_Sound(new Silence());
}

AUD_Sound* AUD_Sound_sine(float frequency, SampleRate rate)
{
	return new AUD_Sound(new Sine(frequency, rate));
}

AUD_Sound* AUD_Sound_square(float frequency, SampleRate rate)
{
	return new AUD_Sound(new Square(frequency, rate));
}

AUD_Sound* AUD_Sound_triangle(float frequency, SampleRate rate)
{
	return new AUD_Sound(new Triangle(frequency, rate));
}

AUD_Sound* AUD_Sound_accumulate(AUD_Sound* sound, int additive)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Accumulator(*sound, additive));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_ADSR(AUD_Sound* sound, float attack, float decay, float sustain, float release)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new ADSR(*sound, attack, decay, sustain, release));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_delay(AUD_Sound* sound, float delay)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Delay(*sound, delay));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_envelope(AUD_Sound* sound, float attack, float release, float threshold, float arthreshold)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Envelope(*sound, attack, release, threshold, arthreshold));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_fadein(AUD_Sound* sound, float start, float length)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Fader(*sound, FADE_IN, start, length));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_fadeout(AUD_Sound* sound, float start, float length)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Fader(*sound, FADE_OUT, start, length));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_filter(AUD_Sound* sound, float* b, int b_length, float* a, int a_length)
{
	assert(sound);

	try
	{
		std::vector<float> a_coeff, b_coeff;

		if(b)
			for(int i = 0; i < b_length; i++)
				b_coeff.push_back(b[i]);

		if(a)
		{
			for(int i = 0; i < a_length; i++)
				a_coeff.push_back(a[i]);

			if(*a == 0.0f)
				a_coeff[0] = 1.0f;
		}

		return new AUD_Sound(new IIRFilter(*sound, b_coeff, a_coeff));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_highpass(AUD_Sound* sound, float frequency, float Q)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Highpass(*sound, frequency, Q));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_limit(AUD_Sound* sound, float start, float end)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Limiter(*sound, start, end));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_loop(AUD_Sound* sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Loop(*sound));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_lowpass(AUD_Sound* sound, float frequency, float Q)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Lowpass(*sound, frequency, Q));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_pitch(AUD_Sound* sound, float factor)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Pitch(*sound, factor));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_rechannel(AUD_Sound* sound, AUD_Channels channels)
{
	assert(sound);

	try
	{
		DeviceSpecs specs;
		specs.channels = static_cast<Channels>(channels);
		specs.rate = RATE_INVALID;
		specs.format = FORMAT_INVALID;
		return new AUD_Sound(new ChannelMapper(*sound, specs));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_reverse(AUD_Sound* sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Reverse(*sound));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_sum(AUD_Sound* sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Sum(*sound));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_threshold(AUD_Sound* sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Threshold(*sound));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_volume(AUD_Sound* sound, float volume)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new Volume(*sound, volume));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_join(AUD_Sound* first, AUD_Sound* second)
{
	assert(first);
	assert(second);

	try
	{
		return new AUD_Sound(new Double(*first, *second));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_mix(AUD_Sound* first, AUD_Sound* second)
{
	assert(first);
	assert(second);

	try
	{
		return new AUD_Sound(new Superpose(*first, *second));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

AUD_Sound* AUD_Sound_pingpong(AUD_Sound* sound)
{
	assert(sound);

	try
	{
		return new AUD_Sound(new PingPong(*sound));
	}
	catch(Exception&)
	{
		return nullptr;
	}
}

void AUD_Sound_free(AUD_Sound* sound)
{
	assert(sound);
	delete sound;
}

AUD_Sound* AUD_Sound_copy(AUD_Sound* sound)
{
	return new std::shared_ptr<ISound>(*sound);
}
