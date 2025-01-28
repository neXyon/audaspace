/*******************************************************************************
 * Copyright 2009-2016 Jörg Müller
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

#include "devices/DefaultSynchronizer.h"

AUD_NAMESPACE_BEGIN

void DefaultSynchronizer::seek(double time)
{
	m_position = time;

	if(m_syncFunction)
		m_syncFunction(m_data, m_state, m_position);
}

double DefaultSynchronizer::getPosition()
{
	return m_position;
}

void DefaultSynchronizer::play()
{
	m_state = 1;

	if(m_syncFunction)
		m_syncFunction(m_data, m_state, m_position);
}

void DefaultSynchronizer::stop()
{
	m_state = 0;

	if(m_syncFunction)
		m_syncFunction(m_data, m_state, m_position);
}

void DefaultSynchronizer::setSyncCallback(ISynchronizer::syncFunction function, void* data)
{
	m_syncFunction = function;
	m_data = data;
}

int DefaultSynchronizer::isPlaying()
{
	return m_state;
}

/*void DefaultSynchronizer::setPosition(double position)
{
	m_position = position;
}*/

AUD_NAMESPACE_END
