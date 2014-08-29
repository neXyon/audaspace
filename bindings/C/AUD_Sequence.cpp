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

AUD_Sound* AUD_Sequence_create(float fps, int muted)
{
	// specs are changed at a later point!
	Specs specs;
	specs.channels = CHANNELS_STEREO;
	specs.rate = RATE_44100;
	AUD_Sound* sequence = new AUD_Sound(std::shared_ptr<Sequence>(new Sequence(specs, fps, muted)));
	return sequence;
}

void AUD_Sequence_free(AUD_Sound* sequence)
{
	delete sequence;
}

AUD_SequenceEntry* AUD_Sequence_addEntry(AUD_Sound* sequence, AUD_Sound* sound, float begin, float end, float skip)
{
	if(!sound)
		return new AUD_SequenceEntry(((Sequence *)sequence->get())->add(AUD_Sound(), begin, end, skip));
	return new AUD_SequenceEntry(((Sequence *)sequence->get())->add(*sound, begin, end, skip));
}

void AUD_Sequence_removeEntry(AUD_Sound* sequence, AUD_SequenceEntry* entry)
{
	dynamic_cast<Sequence *>(sequence->get())->remove(*entry);
	delete entry;
}

void AUD_Sequence_setAnimationData(AUD_Sound* sequence, AUD_AnimateablePropertyType type, int frame, float* data, char animated)
{
	AnimateableProperty* prop = dynamic_cast<Sequence *>(sequence->get())->getAnimProperty(static_cast<AnimateablePropertyType>(type));
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

AUD_DistanceModel AUD_Sequence_getDistanceModel(AUD_Sound* sequence)
{
	assert(sequence);
	return static_cast<AUD_DistanceModel>(dynamic_cast<Sequence *>(sequence->get())->getDistanceModel());
}

void AUD_Sequence_setDistanceModel(AUD_Sound* sequence, AUD_DistanceModel value)
{
	assert(sequence);
	dynamic_cast<Sequence *>(sequence->get())->setDistanceModel(static_cast<DistanceModel>(value));
}

float AUD_Sequence_getDopplerFactor(AUD_Sound* sequence)
{
	assert(sequence);
	return dynamic_cast<Sequence *>(sequence->get())->getDopplerFactor();
}

void AUD_Sequence_setDopplerFactor(AUD_Sound* sequence, float value)
{
	assert(sequence);
	dynamic_cast<Sequence *>(sequence->get())->setDopplerFactor(value);
}

float AUD_Sequence_getFPS(AUD_Sound* sequence)
{
	assert(sequence);
	return dynamic_cast<Sequence *>(sequence->get())->getFPS();
}

void AUD_Sequence_setFPS(AUD_Sound* sequence, float value)
{
	assert(sequence);
	dynamic_cast<Sequence *>(sequence->get())->setFPS(value);
}

int AUD_Sequence_isMuted(AUD_Sound* sequence)
{
	assert(sequence);
	return dynamic_cast<Sequence *>(sequence->get())->isMuted();
}

void AUD_Sequence_setMuted(AUD_Sound* sequence, int value)
{
	assert(sequence);
	dynamic_cast<Sequence *>(sequence->get())->mute(value);
}

static inline AUD_Specs convSpecToC(aud::Specs specs)
{
	AUD_Specs s;
	s.channels = static_cast<AUD_Channels>(specs.channels);
	s.rate = static_cast<AUD_SampleRate>(specs.rate);
	return s;
}

static inline aud::Specs convCToSpec(AUD_Specs specs)
{
	aud::Specs s;
	s.channels = static_cast<Channels>(specs.channels);
	s.rate = static_cast<SampleRate>(specs.rate);
	return s;
}

AUD_Specs AUD_Sequence_getSpecs(AUD_Sound* sequence)
{
	assert(sequence);
	return convSpecToC(dynamic_cast<Sequence *>(sequence->get())->getSpecs());
}

void AUD_Sequence_setSpecs(AUD_Sound* sequence, AUD_Specs value)
{
	assert(sequence);
	dynamic_cast<Sequence *>(sequence->get())->setSpecs(convCToSpec(value));
}

float AUD_Sequence_getSpeedOfSound(AUD_Sound* sequence)
{
	assert(sequence);
	return dynamic_cast<Sequence *>(sequence->get())->getSpeedOfSound();
}

void AUD_Sequence_setSpeedOfSound(AUD_Sound* sequence, float value)
{
	assert(sequence);
	dynamic_cast<Sequence *>(sequence->get())->setSpeedOfSound(value);
}



void AUD_SequenceEntry_move(AUD_SequenceEntry* entry, float begin, float end, float skip)
{
	(*entry)->move(begin, end, skip);
}

void AUD_SequenceEntry_setAnimationData(AUD_SequenceEntry* entry, AUD_AnimateablePropertyType type, int frame, float* data, char animated)
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

float AUD_SequenceEntry_getAttenuation(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->getAttenuation();
}

void AUD_SequenceEntry_setAttenuation(AUD_SequenceEntry* sequence_entry, float value)
{
	assert(sequence_entry);
	(*sequence_entry)->setAttenuation(value);
}

float AUD_SequenceEntry_getConeAngleInner(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->getConeAngleInner();
}

void AUD_SequenceEntry_setConeAngleInner(AUD_SequenceEntry* sequence_entry, float value)
{
	assert(sequence_entry);
	(*sequence_entry)->setConeAngleInner(value);
}

float AUD_SequenceEntry_getConeAngleOuter(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->getConeAngleOuter();
}

void AUD_SequenceEntry_setConeAngleOuter(AUD_SequenceEntry* sequence_entry, float value)
{
	assert(sequence_entry);
	(*sequence_entry)->setConeAngleOuter(value);
}

float AUD_SequenceEntry_getConeVolumeOuter(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->getConeVolumeOuter();
}

void AUD_SequenceEntry_setConeVolumeOuter(AUD_SequenceEntry* sequence_entry, float value)
{
	assert(sequence_entry);
	(*sequence_entry)->setConeVolumeOuter(value);
}

float AUD_SequenceEntry_getDistanceMaximum(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->getDistanceMaximum();
}

void AUD_SequenceEntry_setDistanceMaximum(AUD_SequenceEntry* sequence_entry, float value)
{
	assert(sequence_entry);
	(*sequence_entry)->setDistanceMaximum(value);
}

float AUD_SequenceEntry_getDistanceReference(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->getDistanceReference();
}

void AUD_SequenceEntry_setDistanceReference(AUD_SequenceEntry* sequence_entry, float value)
{
	assert(sequence_entry);
	(*sequence_entry)->setDistanceReference(value);
}

int AUD_SequenceEntry_isMuted(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->isMuted();
}

void AUD_SequenceEntry_setMuted(AUD_SequenceEntry* sequence_entry, int value)
{
	assert(sequence_entry);
	(*sequence_entry)->mute(value);
}

int AUD_SequenceEntry_isRelative(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->isRelative();
}

void AUD_SequenceEntry_setRelative(AUD_SequenceEntry* sequence_entry, int value)
{
	assert(sequence_entry);
	(*sequence_entry)->setRelative(value);
}

AUD_Sound* AUD_SequenceEntry_getSound(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return new std::shared_ptr<ISound>((*sequence_entry)->getSound());
}

void AUD_SequenceEntry_setSound(AUD_SequenceEntry* sequence_entry, AUD_Sound* value)
{
	assert(sequence_entry);
	if(value)
		(*sequence_entry)->setSound(*value);
	else
		(*sequence_entry)->setSound(AUD_Sound());
}

float AUD_SequenceEntry_getVolumeMaximum(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->getVolumeMaximum();
}

void AUD_SequenceEntry_setVolumeMaximum(AUD_SequenceEntry* sequence_entry, float value)
{
	assert(sequence_entry);
	(*sequence_entry)->setVolumeMaximum(value);
}

float AUD_SequenceEntry_getVolumeMinimum(AUD_SequenceEntry* sequence_entry)
{
	assert(sequence_entry);
	return (*sequence_entry)->getVolumeMinimum();
}

void AUD_SequenceEntry_setVolumeMinimum(AUD_SequenceEntry* sequence_entry, float value)
{
	assert(sequence_entry);
	(*sequence_entry)->setVolumeMinimum(value);
}
