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

#include "devices/I3DHandle.h"
#include "Exception.h"

#include <cassert>

using namespace aud;

#define AUD_CAPI_IMPLEMENTATION
#include "AUD_Handle.h"

int AUD_setLoop(AUD_Handle *handle, int loops)
{
	assert(handle);

	try
	{
		return (*handle)->setLoopCount(loops);
	}
	catch(Exception&)
	{
	}

	return false;
}

int AUD_pause(AUD_Handle *handle)
{
	assert(handle);
	return (*handle)->pause();
}

int AUD_resume(AUD_Handle *handle)
{
	assert(handle);
	return (*handle)->resume();
}

int AUD_stop(AUD_Handle *handle)
{
	assert(handle);
	int result = (*handle)->stop();
	delete handle;
	return result;
}

int AUD_setKeep(AUD_Handle *handle, int keep)
{
	assert(handle);
	return (*handle)->setKeep(keep);
}

int AUD_seek(AUD_Handle *handle, float seekTo)
{
	assert(handle);
	return (*handle)->seek(seekTo);
}

float AUD_getPosition(AUD_Handle *handle)
{
	assert(handle);
	return (*handle)->getPosition();
}

AUD_Status AUD_getStatus(AUD_Handle *handle)
{
	assert(handle);
	return static_cast<AUD_Status>((*handle)->getStatus());
}

int AUD_setSourceLocation(AUD_Handle *handle, const float location[3])
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		Vector3 v(location[0], location[1], location[2]);
		return h->setSourceLocation(v);
	}

	return false;
}

int AUD_setSourceVelocity(AUD_Handle *handle, const float velocity[3])
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		Vector3 v(velocity[0], velocity[1], velocity[2]);
		return h->setSourceVelocity(v);
	}

	return false;
}

int AUD_setSourceOrientation(AUD_Handle *handle, const float orientation[4])
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		Quaternion q(orientation[3], orientation[0], orientation[1], orientation[2]);
		return h->setSourceOrientation(q);
	}

	return false;
}

int AUD_setRelative(AUD_Handle *handle, int relative)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setRelative(relative);
	}

	return false;
}

int AUD_setVolumeMaximum(AUD_Handle *handle, float volume)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setVolumeMaximum(volume);
	}

	return false;
}

int AUD_setVolumeMinimum(AUD_Handle *handle, float volume)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setVolumeMinimum(volume);
	}

	return false;
}

int AUD_setDistanceMaximum(AUD_Handle *handle, float distance)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setDistanceMaximum(distance);
	}

	return false;
}

int AUD_setDistanceReference(AUD_Handle *handle, float distance)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setDistanceReference(distance);
	}

	return false;
}

int AUD_setAttenuation(AUD_Handle *handle, float factor)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setAttenuation(factor);
	}

	return false;
}

int AUD_setConeAngleOuter(AUD_Handle *handle, float angle)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setConeAngleOuter(angle);
	}

	return false;
}

int AUD_setConeAngleInner(AUD_Handle *handle, float angle)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setConeAngleInner(angle);
	}

	return false;
}

int AUD_setConeVolumeOuter(AUD_Handle *handle, float volume)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if(h.get())
	{
		return h->setConeVolumeOuter(volume);
	}

	return false;
}

int AUD_setSoundVolume(AUD_Handle *handle, float volume)
{
	assert(handle);
	try
	{
		return (*handle)->setVolume(volume);
	}
	catch(Exception&)
	{
	}
	return false;
}

int AUD_setSoundPitch(AUD_Handle *handle, float pitch)
{
	assert(handle);
	try
	{
		return (*handle)->setPitch(pitch);
	}
	catch(Exception&)
	{
	}
	return false;
}

void AUD_freeHandle(AUD_Handle *handle)
{
	delete handle;
}
