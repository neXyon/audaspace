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

#include "Audaspace.h"

#include <memory>

AUD_NAMESPACE_BEGIN

class IHandle;

/**
 * This class enables global synchronization of several audio applications if supported.
 * Jack for example supports synchronization through Jack Transport.
 */
class ISynchronizer
{
public:
	/**
	 * Destroys the synchronizer.
	 */
	virtual ~ISynchronizer() {}

	typedef void (*syncFunction)(void*, int, float);

	virtual void seek(std::shared_ptr<IHandle> handle, float time) = 0;
	virtual float getPosition(std::shared_ptr<IHandle> handle) = 0;
	virtual void play() = 0;
	virtual void stop() = 0;
	virtual void setSyncCallback(syncFunction function, void* data) = 0;
	virtual bool isPlaying() = 0;
};

AUD_NAMESPACE_END
