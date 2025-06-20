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

#include "fx/TimeStretch.h"

#include "fx/TimeStretchReader.h"

AUD_NAMESPACE_BEGIN

TimeStretch::TimeStretch(std::shared_ptr<ISound> sound, double ratio, TimeStretchQuality quality) : Effect(sound), m_timeRatio(ratio), m_quality(quality)

{
}

std::shared_ptr<IReader> TimeStretch::createReader()
{
	return std::shared_ptr<IReader>(new TimeStretchReader(getReader(), m_timeRatio, m_quality));
}

double TimeStretch::getTimeRatio() const
{
	return m_timeRatio;
}

AUD_NAMESPACE_END
