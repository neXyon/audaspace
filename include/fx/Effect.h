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

#include "ISound.h"

AUD_NAMESPACE_BEGIN

/**
 * This factory is a base class for all effect factories that take one other
 * factory as input.
 */
class Effect : public ISound
{
private:
	// hide copy constructor and operator=
	Effect(const Effect&);
	Effect& operator=(const Effect&);

protected:
	/**
	 * If there is no reader it is created out of this factory.
	 */
	std::shared_ptr<ISound> m_factory;

	/**
	 * Returns the reader created out of the factory.
	 * This method can be used for the createReader function of the implementing
	 * classes.
	 * \return The reader created out of the factory.
	 */
	inline std::shared_ptr<IReader> getReader() const
	{
		return m_factory->createReader();
	}

public:
	/**
	 * Creates a new factory.
	 * \param factory The input factory.
	 */
	Effect(std::shared_ptr<ISound> factory);

	/**
	 * Destroys the factory.
	 */
	virtual ~Effect();

	/**
	 * Returns the saved factory.
	 * \return The factory or NULL if there has no factory been saved.
	 */
	std::shared_ptr<ISound> getFactory() const;
};

AUD_NAMESPACE_END
