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

#include "IReader.h"
#include "ReadDevice.h"
#include "SequenceData.h"

AUD_NAMESPACE_BEGIN

class SequenceHandle;

/**
 * This reader plays back sequenced entries.
 */
class SequenceReader : public IReader
{
private:
	/**
	 * The current position.
	 */
	int m_position;

	/**
	 * The read device used to mix the sounds correctly.
	 */
	ReadDevice m_device;

	/**
	 * Saves the sequence the reader belongs to.
	 */
	std::shared_ptr<SequenceData> m_sequence;

	/**
	 * The list of playback handles for the entries.
	 */
	std::list<std::shared_ptr<SequenceHandle> > m_handles;

	/**
	 * Last status read from the sequence.
	 */
	int m_status;

	/**
	 * Last entry status read from the sequence.
	 */
	int m_entry_status;

	// hide copy constructor and operator=
	SequenceReader(const SequenceReader&);
	SequenceReader& operator=(const SequenceReader&);

public:
	/**
	 * Creates a resampling reader.
	 * \param reader The reader to mix.
	 * \param specs The target specification.
	 */
	SequenceReader(std::shared_ptr<SequenceData> sequence, bool quality = false);

	/**
	 * Destroys the reader.
	 */
	~SequenceReader();

	virtual bool isSeekable() const;
	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;
	virtual Specs getSpecs() const;
	virtual void read(int& length, bool& eos, sample_t* buffer);
};

AUD_NAMESPACE_END
