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

#include "fx/EffectReader.h"
#include "util/Buffer.h"

AUD_NAMESPACE_BEGIN

/**
 * This class maps a sound source's channels to a specific output channel count.
 * \note The input sample format must be float.
 */
class ChannelMapperReader : public EffectReader
{
private:
	/**
	 * The sound reading buffer.
	 */
	Buffer m_buffer;

	/**
	 * The output specification.
	 */
	Channels m_target_channels;

	/**
	 * The channel count of the reader.
	 */
	Channels m_source_channels;

	/**
	 * The mapping specification.
	 */
	float* m_mapping;

	/**
	 * The size of the mapping.
	 */
	int m_map_size;

	/**
	 * The mono source angle.
	 */
	float m_mono_angle;

	static const Channel MONO_MAP[];
	static const Channel STEREO_MAP[];
	static const Channel STEREO_LFE_MAP[];
	static const Channel SURROUND4_MAP[];
	static const Channel SURROUND5_MAP[];
	static const Channel SURROUND51_MAP[];
	static const Channel SURROUND61_MAP[];
	static const Channel SURROUND71_MAP[];
	static const Channel* CHANNEL_MAPS[];

	static const float MONO_ANGLES[];
	static const float STEREO_ANGLES[];
	static const float STEREO_LFE_ANGLES[];
	static const float SURROUND4_ANGLES[];
	static const float SURROUND5_ANGLES[];
	static const float SURROUND51_ANGLES[];
	static const float SURROUND61_ANGLES[];
	static const float SURROUND71_ANGLES[];
	static const float* CHANNEL_ANGLES[];

	// hide copy constructor and operator=
	ChannelMapperReader(const ChannelMapperReader&);
	ChannelMapperReader& operator=(const ChannelMapperReader&);

	/**
	 * Calculates the mapping matrix.
	 */
	void calculateMapping();

	/**
	 * Calculates the distance between two angles.
	 */
	float angleDistance(float alpha, float beta);

public:
	/**
	 * Creates a channel mapper reader.
	 * \param reader The reader to map.
	 * \param mapping The mapping specification as two dimensional float array.
	 */
	ChannelMapperReader(std::shared_ptr<IReader> reader, Channels channels);

	/**
	 * Destroys the reader.
	 */
	~ChannelMapperReader();

	/**
	 * Sets the requested channel output count.
	 * \param channels The channel output count.
	 */
	void setChannels(Channels channels);

	/**
	 * Sets the angle for mono sources.
	 * \param angle The angle for mono sources.
	 */
	void setMonoAngle(float angle);

	virtual Specs getSpecs() const;
	virtual void read(int& length, bool& eos, sample_t* buffer);
};

AUD_NAMESPACE_END
