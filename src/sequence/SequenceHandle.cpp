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

#include "SequenceHandle.h"
#include "ReadDevice.h"
#include "MutexLock.h"

AUD_NAMESPACE_BEGIN

SequenceHandle::SequenceHandle(std::shared_ptr<SequenceEntry> entry, ReadDevice& device) :
	m_entry(entry),
	m_status(0),
	m_pos_status(0),
	m_sound_status(0),
	m_device(device)
{
	if(entry->m_sound.get())
	{
		m_handle = device.play(entry->m_sound, true);
		m_3dhandle = std::dynamic_pointer_cast<I3DHandle>(m_handle);
	}
}

SequenceHandle::~SequenceHandle()
{
	stop();
}

int SequenceHandle::compare(std::shared_ptr<SequenceEntry> entry) const
{
	if(m_entry->getID() < entry->getID())
		return -1;
	else if(m_entry->getID() == entry->getID())
		return 0;
	return 1;
}

void SequenceHandle::stop()
{
	if(m_handle.get())
		m_handle->stop();
}

void SequenceHandle::update(float position, float frame, float fps)
{
	if(m_handle.get())
	{
		MutexLock lock(*m_entry);
		if(position >= m_entry->m_end && m_entry->m_end >= 0)
			m_handle->pause();
		else if(position >= m_entry->m_begin)
			m_handle->resume();

		if(m_sound_status != m_entry->m_sound_status)
		{
			if(m_handle.get())
				m_handle->stop();

			if(m_entry->m_sound.get())
			{
				m_handle = m_device.play(m_entry->m_sound, true);
				m_3dhandle = std::dynamic_pointer_cast<I3DHandle>(m_handle);
			}

			m_sound_status = m_entry->m_sound_status;
			m_pos_status--;
			m_status--;
		}

		if(m_pos_status != m_entry->m_pos_status)
		{
			seek(position);

			m_pos_status = m_entry->m_pos_status;
		}

		if(m_status != m_entry->m_status)
		{
			m_3dhandle->setRelative(m_entry->m_relative);
			m_3dhandle->setVolumeMaximum(m_entry->m_volume_max);
			m_3dhandle->setVolumeMinimum(m_entry->m_volume_min);
			m_3dhandle->setDistanceMaximum(m_entry->m_distance_max);
			m_3dhandle->setDistanceReference(m_entry->m_distance_reference);
			m_3dhandle->setAttenuation(m_entry->m_attenuation);
			m_3dhandle->setConeAngleOuter(m_entry->m_cone_angle_outer);
			m_3dhandle->setConeAngleInner(m_entry->m_cone_angle_inner);
			m_3dhandle->setConeVolumeOuter(m_entry->m_cone_volume_outer);

			m_status = m_entry->m_status;
		}

		float value;

		m_entry->m_volume.read(frame, &value);
		m_handle->setVolume(value);
		m_entry->m_pitch.read(frame, &value);
		m_handle->setPitch(value);
		m_entry->m_panning.read(frame, &value);
		SoftwareDevice::setPanning(m_handle.get(), value);

		Vector3 v, v2;
		Quaternion q;

		m_entry->m_orientation.read(frame, q.get());
		m_3dhandle->setSourceOrientation(q);
		m_entry->m_location.read(frame, v.get());
		m_3dhandle->setSourceLocation(v);
		m_entry->m_location.read(frame + 1, v2.get());
		v2 -= v;
		m_3dhandle->setSourceVelocity(v2 * fps);

		if(m_entry->m_muted)
			m_handle->setVolume(0);
	}
}

void SequenceHandle::seek(float position)
{
	if(m_handle.get())
	{
		MutexLock lock(*m_entry);
		if(position >= m_entry->m_end && m_entry->m_end >= 0)
		{
			m_handle->pause();
			return;
		}

		float seekpos = position - m_entry->m_begin;
		if(seekpos < 0)
			seekpos = 0;
		seekpos += m_entry->m_skip;
		m_handle->setPitch(1.0f);
		m_handle->seek(seekpos);
		if(position < m_entry->m_begin)
			m_handle->pause();
		else
			m_handle->resume();
	}
}

AUD_NAMESPACE_END
