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
#include "fx/SoundList.h"
#include "fx/MutableSound.h"
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
#include "AUD_PlaybackManager.h"

AUD_API AUD_PlaybackManager* AUD_PlaybackManager_create(AUD_Device* device)
{
	assert(device);

	try
	{
		return new AUD_PlaybackManager(new PlaybackManager(*device));
	}
	catch (Exception&)
	{
		return nullptr;
	}
}

AUD_API void AUD_PlaybackManager_free(AUD_PlaybackManager* manager)
{
	assert(manager);
	delete manager;
}

AUD_API void AUD_PlaybackManager_play(AUD_PlaybackManager* manager, AUD_Sound* sound, unsigned int catKey)
{
	assert(manager);
	assert(sound);

	(*manager)->play(*sound, catKey);
}

AUD_API int AUD_PlaybackManager_resume(AUD_PlaybackManager* manager, unsigned int catKey)
{
	assert(manager);
	return (*manager)->resume(catKey);
}

AUD_API int AUD_PlaybackManager_pause(AUD_PlaybackManager* manager, unsigned int catKey)
{
	assert(manager);
	return (*manager)->pause(catKey);
}

AUD_API float AUD_PlaybackManager_getVolume(AUD_PlaybackManager* manager, unsigned int catKey)
{
	assert(manager);
	return (*manager)->getVolume(catKey);
}

AUD_API int AUD_PlaybackManager_setVolume(AUD_PlaybackManager* manager, float volume, unsigned int catKey)
{
	assert(manager);
	return (*manager)->setVolume(volume, catKey);
}

AUD_API int AUD_PlaybackManager_stop(AUD_PlaybackManager* manager, unsigned int catKey)
{
	assert(manager);
	return (*manager)->stop(catKey);
}

AUD_API void AUD_PlaybackManager_clean(AUD_PlaybackManager* manager)
{
	assert(manager);
	(*manager)->clean();
}