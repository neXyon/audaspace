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

#include "devices/I3DDevice.h"
#include "devices/DeviceManager.h"
#include "sequence/Sequence.h"
#include "Exception.h"

#include <cassert>

using namespace aud;

#define AUD_CAPI_IMPLEMENTATION
#include "AUD_Sequence.h"

AUD_Sound* AUD_createSequencer(float fps, int muted)
{
	// specs are changed at a later point!
	Specs specs;
	specs.channels = CHANNELS_STEREO;
	specs.rate = RATE_44100;
	AUD_Sound* sequencer = new AUD_Sound(std::shared_ptr<Sequence>(new Sequence(specs, fps, muted)));
	return sequencer;
}

void AUD_destroySequencer(AUD_Sound* sequencer)
{
	delete sequencer;
}

void AUD_setSequencerMuted(AUD_Sound* sequencer, int muted)
{
	dynamic_cast<Sequence *>(sequencer->get())->mute(muted);
}

void AUD_setSequencerFPS(AUD_Sound* sequencer, float fps)
{
	dynamic_cast<Sequence *>(sequencer->get())->setFPS(fps);
}

AUD_SEntry* AUD_addSequence(AUD_Sound* sequencer, AUD_Sound* sound,
							float begin, float end, float skip)
{
	if(!sound)
		return new AUD_SEntry(((Sequence *)sequencer->get())->add(AUD_Sound(), begin, end, skip));
	return new AUD_SEntry(((Sequence *)sequencer->get())->add(*sound, begin, end, skip));
}

void AUD_removeSequence(AUD_Sound* sequencer, AUD_SEntry* entry)
{
	dynamic_cast<Sequence *>(sequencer->get())->remove(*entry);
	delete entry;
}

void AUD_moveSequence(AUD_SEntry* entry, float begin, float end, float skip)
{
	(*entry)->move(begin, end, skip);
}

void AUD_muteSequence(AUD_SEntry* entry, char mute)
{
	(*entry)->mute(mute);
}

void AUD_setRelativeSequence(AUD_SEntry* entry, char relative)
{
	(*entry)->setRelative(relative);
}

void AUD_updateSequenceSound(AUD_SEntry* entry, AUD_Sound* sound)
{
	if(sound)
		(*entry)->setSound(*sound);
	else
		(*entry)->setSound(AUD_Sound());
}

void AUD_setSequenceAnimData(AUD_SEntry* entry, AUD_AnimateablePropertyType type, int frame, float* data, char animated)
{
	AnimateableProperty* prop = (*entry)->getAnimProperty(static_cast<AnimateablePropertyType>(type));
	if(animated)
	{
		if(frame >= 0)
			prop->write(data, frame, 1);
	}
	else
	{
		prop->write(data);
	}
}

void AUD_setSequencerAnimData(AUD_Sound* sequencer, AUD_AnimateablePropertyType type, int frame, float* data, char animated)
{
	AnimateableProperty* prop = dynamic_cast<Sequence *>(sequencer->get())->getAnimProperty(static_cast<AnimateablePropertyType>(type));
	if(animated)
	{
		if(frame >= 0)
		{
			prop->write(data, frame, 1);
		}
	}
	else
	{
		prop->write(data);
	}
}

void AUD_updateSequenceData(AUD_SEntry* entry, float volume_max, float volume_min,
							float distance_max, float distance_reference, float attenuation,
							float cone_angle_outer, float cone_angle_inner, float cone_volume_outer)
{
	(*entry)->updateAll(volume_max, volume_min, distance_max, distance_reference, attenuation,
						cone_angle_outer, cone_angle_inner, cone_volume_outer);
}

void AUD_updateSequencerData(AUD_Sound* sequencer, float speed_of_sound,
							 float factor, AUD_DistanceModel model)
{
	Sequence* f = dynamic_cast<Sequence *>(sequencer->get());
	f->setSpeedOfSound(speed_of_sound);
	f->setDopplerFactor(factor);
	f->setDistanceModel(static_cast<DistanceModel>(model));
}

void AUD_setSequencerDeviceSpecs(AUD_Sound* sequencer)
{
	dynamic_cast<Sequence *>(sequencer->get())->setSpecs(DeviceManager::getDevice()->getSpecs().specs);
}

static inline aud::Specs convCToSpec(AUD_Specs specs)
{
	aud::Specs s;
	s.channels = static_cast<Channels>(specs.channels);
	s.rate = static_cast<SampleRate>(specs.rate);
	return s;
}

void AUD_setSequencerSpecs(AUD_Sound* sequencer, AUD_Specs specs)
{
	dynamic_cast<Sequence *>(sequencer->get())->setSpecs(convCToSpec(specs));
}
