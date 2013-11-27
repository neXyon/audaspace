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

#pragma once

#include "util/Buffer.h"
#include "util/ILockable.h"

#include <pthread.h>
#include <list>

AUD_NAMESPACE_BEGIN

/**
 * This class saves animation data for float properties.
 */
class AnimateableProperty : private Buffer, public ILockable
{
private:
	struct Unknown {
		int start;
		int end;

		Unknown(int start, int end) :
			start(start), end(end) {}
	};

	/// The count of floats for a single property.
	const int m_count;

	/// Whether the property is animated or not.
	bool m_isAnimated;

	/// The mutex for locking.
	pthread_mutex_t m_mutex;

	/// The list of unknown buffer areas.
	std::list<Unknown> m_unknown;

	// hide copy constructor and operator=
	AnimateableProperty(const AnimateableProperty&);
	AnimateableProperty& operator=(const AnimateableProperty&);

	void updateUnknownCache(int start, int end);

public:
	/**
	 * Creates a new animateable property.
	 * \param count The count of floats for a single property.
	 */
	AnimateableProperty(int count = 1);

	/**
	 * Destroys the animateable property.
	 */
	~AnimateableProperty();

	/**
	 * Locks the property.
	 */
	virtual void lock();

	/**
	 * Unlocks the previously locked property.
	 */
	virtual void unlock();

	/**
	 * Writes the properties value and marks it non-animated.
	 * \param data The new value.
	 */
	void write(const float* data);

	/**
	 * Writes the properties value and marks it animated.
	 * \param data The new value.
	 * \param position The position in the animation in frames.
	 * \param count The count of frames to write.
	 */
	void write(const float* data, int position, int count);

	/**
	 * Reads the properties value.
	 * \param position The position in the animation in frames.
	 * \param[out] out Where to write the value to.
	 */
	void read(float position, float* out);

	/**
	 * Returns whether the property is animated.
	 * \return Whether the property is animated.
	 */
	bool isAnimated() const;
};

AUD_NAMESPACE_END
