
/*******************************************************************************
 * Copyright 2009-2024 Jörg Müller
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

#include "devices/OpenCloseDevice.h"

AUD_NAMESPACE_BEGIN

void OpenCloseDevice::closeAfterDelay()
{
	while(std::chrono::steady_clock::now() < m_playback_stopped_time + m_device_close_delay)
	{
		auto time_left{m_playback_stopped_time + m_device_close_delay - std::chrono::steady_clock::now()};
		auto maximum_sleep_time{m_device_close_delay / 10};
		std::this_thread::sleep_for(std::min<std::common_type<decltype(time_left), decltype(maximum_sleep_time)>::type>(time_left, maximum_sleep_time));

		std::lock_guard<std::mutex> lock(m_delayed_close_mutex);

		if(m_playing)
		{
			m_delayed_close_running = false;
			return;
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_delayed_close_mutex);

		m_delayed_close_running = false;

		if(m_playing)
			return;

		close();
		m_device_opened = false;
	}
}

OpenCloseDevice::~OpenCloseDevice()
{
	std::unique_lock<std::mutex> lock(m_delayed_close_mutex);

	if(m_delayed_close_running)
	{
		m_playback_stopped_time = std::chrono::steady_clock::now() - m_device_close_delay;

		lock.unlock();
		m_delayed_close_thread.join();
		lock.lock();
	}
}

void OpenCloseDevice::playing(bool playing)
{
	std::lock_guard<std::mutex> lock(m_delayed_close_mutex);

	if(m_playing != playing)
	{
		m_playing = playing;
		if(playing)
		{
			if(!m_device_opened)
			{
				open();
				m_device_opened = true;
			}

			start();
		}
		else
		{
			stop();

			m_playback_stopped_time = std::chrono::steady_clock::now();

			if(m_device_opened && m_delayed_close_running)
			{
				m_delayed_close_running = true;
				m_delayed_close_thread = std::thread(&OpenCloseDevice::closeAfterDelay, this);
			}
		}
	}
}
AUD_NAMESPACE_END
