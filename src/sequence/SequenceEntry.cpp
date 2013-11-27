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

#include "sequence/SequenceEntry.h"
#include "sequence/SequenceReader.h"

#include <cmath>
#include <limits>
#include <mutex>

AUD_NAMESPACE_BEGIN

SequenceEntry::SequenceEntry(std::shared_ptr<ISound> sound, float begin, float end, float skip, int id) :
	m_status(0),
	m_pos_status(1),
	m_sound_status(0),
	m_id(id),
	m_sound(sound),
	m_begin(begin),
	m_end(end),
	m_skip(skip),
	m_muted(false),
	m_relative(true),
	m_volume_max(1.0f),
	m_volume_min(0),
	m_distance_max(std::numeric_limits<float>::max()),
	m_distance_reference(1.0f),
	m_attenuation(1.0f),
	m_cone_angle_outer(360),
	m_cone_angle_inner(360),
	m_cone_volume_outer(0),
	m_location(3),
	m_orientation(4)
{
	Quaternion q;
	m_orientation.write(q.get());
	float f = 1;
	m_volume.write(&f);
	m_pitch.write(&f);

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&m_mutex, &attr);

	pthread_mutexattr_destroy(&attr);
}

SequenceEntry::~SequenceEntry()
{
	pthread_mutex_destroy(&m_mutex);
}

void SequenceEntry::lock()
{
	pthread_mutex_lock(&m_mutex);
}

void SequenceEntry::unlock()
{
	pthread_mutex_unlock(&m_mutex);
}

void SequenceEntry::setSound(std::shared_ptr<ISound> sound)
{
	std::lock_guard<ILockable> lock(*this);

	if(m_sound.get() != sound.get())
	{
		m_sound = sound;
		m_sound_status++;
	}
}

void SequenceEntry::move(float begin, float end, float skip)
{
	std::lock_guard<ILockable> lock(*this);

	if(m_begin != begin || m_skip != skip || m_end != end)
	{
		m_begin = begin;
		m_skip = skip;
		m_end = end;
		m_pos_status++;
	}
}

void SequenceEntry::mute(bool mute)
{
	std::lock_guard<ILockable> lock(*this);

	m_muted = mute;
}

int SequenceEntry::getID() const
{
	return m_id;
}

AnimateableProperty* SequenceEntry::getAnimProperty(AnimateablePropertyType type)
{
	switch(type)
	{
	case AP_VOLUME:
		return &m_volume;
	case AP_PITCH:
		return &m_pitch;
	case AP_PANNING:
		return &m_panning;
	case AP_LOCATION:
		return &m_location;
	case AP_ORIENTATION:
		return &m_orientation;
	default:
		return NULL;
	}
}

void SequenceEntry::updateAll(float volume_max, float volume_min, float distance_max,
								   float distance_reference, float attenuation, float cone_angle_outer,
								   float cone_angle_inner, float cone_volume_outer)
{
	std::lock_guard<ILockable> lock(*this);

	if(volume_max != m_volume_max)
	{
		m_volume_max = volume_max;
		m_status++;
	}

	if(volume_min != m_volume_min)
	{
		m_volume_min = volume_min;
		m_status++;
	}

	if(distance_max != m_distance_max)
	{
		m_distance_max = distance_max;
		m_status++;
	}

	if(distance_reference != m_distance_reference)
	{
		m_distance_reference = distance_reference;
		m_status++;
	}

	if(attenuation != m_attenuation)
	{
		m_attenuation = attenuation;
		m_status++;
	}

	if(cone_angle_outer != m_cone_angle_outer)
	{
		m_cone_angle_outer = cone_angle_outer;
		m_status++;
	}

	if(cone_angle_inner != m_cone_angle_inner)
	{
		m_cone_angle_inner = cone_angle_inner;
		m_status++;
	}

	if(cone_volume_outer != m_cone_volume_outer)
	{
		m_cone_volume_outer = cone_volume_outer;
		m_status++;
	}
}

bool SequenceEntry::isRelative()
{
	return m_relative;
}

void SequenceEntry::setRelative(bool relative)
{
	std::lock_guard<ILockable> lock(*this);

	if(m_relative != relative)
	{
		m_relative = relative;
		m_status++;
	}
}

float SequenceEntry::getVolumeMaximum()
{
	return m_volume_max;
}

void SequenceEntry::setVolumeMaximum(float volume)
{
	std::lock_guard<ILockable> lock(*this);

	m_volume_max = volume;
	m_status++;
}

float SequenceEntry::getVolumeMinimum()
{
	return m_volume_min;
}

void SequenceEntry::setVolumeMinimum(float volume)
{
	std::lock_guard<ILockable> lock(*this);

	m_volume_min = volume;
	m_status++;
}

float SequenceEntry::getDistanceMaximum()
{
	return m_distance_max;
}

void SequenceEntry::setDistanceMaximum(float distance)
{
	std::lock_guard<ILockable> lock(*this);

	m_distance_max = distance;
	m_status++;
}

float SequenceEntry::getDistanceReference()
{
	return m_distance_reference;
}

void SequenceEntry::setDistanceReference(float distance)
{
	std::lock_guard<ILockable> lock(*this);

	m_distance_reference = distance;
	m_status++;
}

float SequenceEntry::getAttenuation()
{
	return m_attenuation;
}

void SequenceEntry::setAttenuation(float factor)
{
	std::lock_guard<ILockable> lock(*this);

	m_attenuation = factor;
	m_status++;
}

float SequenceEntry::getConeAngleOuter()
{
	return m_cone_angle_outer;
}

void SequenceEntry::setConeAngleOuter(float angle)
{
	std::lock_guard<ILockable> lock(*this);

	m_cone_angle_outer = angle;
	m_status++;
}

float SequenceEntry::getConeAngleInner()
{
	return m_cone_angle_inner;
}

void SequenceEntry::setConeAngleInner(float angle)
{
	std::lock_guard<ILockable> lock(*this);

	m_cone_angle_inner = angle;
	m_status++;
}

float SequenceEntry::getConeVolumeOuter()
{
	return m_cone_volume_outer;
}

void SequenceEntry::setConeVolumeOuter(float volume)
{
	std::lock_guard<ILockable> lock(*this);

	m_cone_volume_outer = volume;
	m_status++;
}

AUD_NAMESPACE_END
