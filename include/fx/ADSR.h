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

/**
 * @file ADSR.h
 * @ingroup fx
 * The ADSR class.
 */

#include "fx/Effect.h"

AUD_NAMESPACE_BEGIN

class ADSR : public Effect
{
private:
	/**
	 * Attack time.
	 */
	float m_attack;

	/**
	 * Decay time.
	 */
	float m_decay;

	/**
	 * Sustain level.
	 */
	float m_sustain;

	/**
	 * Release time.
	 */
	float m_release;

	// delete copy constructor and operator=
	ADSR(const ADSR&) = delete;
	ADSR& operator=(const ADSR&) = delete;

public:
	ADSR(std::shared_ptr<ISound> sound, float attack, float decay, float sustain, float release);

	float getAttack() const;

	void setAttack(float attack);

	float getDecay() const;

	void setDecay(float decay);

	float getSustain() const;

	void setSustain(float sustain);

	float getRelease() const;

	void setRelease(float release);

	virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END
