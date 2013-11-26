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
#include "Buffer.h"

#include <AL/al.h>
#include <AL/alc.h>

AUD_NAMESPACE_BEGIN

/**
 * This class is used for sine tone playback.
 * The output format is in the 16 bit format and stereo, the sample rate can be
 * specified.
 * As the two channels both play the same the output could also be mono, but
 * in most cases this will result in having to resample for output, so stereo
 * sound is created directly.
 */
class OpenALReader : public IReader
{
private:
	/**
	 * The specs of the reader.
	 */
	Specs m_specs;

	/**
	 * The current position in samples.
	 */
	int m_position;

	/**
	 * The playback buffer.
	 */
	Buffer m_buffer;

	/**
	 * The capture device.
	 */
	ALCdevice* m_device;

	// hide copy constructor and operator=
	OpenALReader(const OpenALReader&);
	OpenALReader& operator=(const OpenALReader&);

public:
	/**
	 * Creates a new reader.
	 * \param frequency The frequency of the sine wave.
	 * \param sampleRate The output sample rate.
	 */
	OpenALReader(Specs specs, int buffersize = AUD_DEFAULT_BUFFER_SIZE);

	virtual ~OpenALReader();

	virtual bool isSeekable() const;
	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;
	virtual Specs getSpecs() const;
	virtual void read(int & length, bool& eos, sample_t* buffer);
};

AUD_NAMESPACE_END
