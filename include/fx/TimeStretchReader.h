/*******************************************************************************
 * Copyright 2009-2025 Jörg Müller
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
 * @file TimeStretchReader.h
 * @ingroup fx
 * The TimeStretch class.
 */

#include "TimeStretch.h"

#include "fx/EffectReader.h"
#include "rubberband/RubberBandStretcher.h"
#include "util/Buffer.h"

using namespace RubberBand;

AUD_NAMESPACE_BEGIN

/**
 * This class reads another reader and changes the playback speed while preserving the pitch.
 */
class AUD_API TimeStretchReader : public EffectReader
{
private:
	/**
	 * The amount of samples to pad the input audio before processing
	 */
	int m_padAmount;

	/**
	 * The amount of samples drop the output after retreiving
	 */
	int m_dropAmount;

	/**
	 * The current position.
	 */
	int m_position;

	/**
	 * Whether the reader has reached the end of stream
	 */
	bool m_finishedReader;

	/**
	 * The input buffer for the reader
	 */
	Buffer m_buffer;

	/**
	 * The input deinterleaved buffers for each channel
	 */
	std::vector<Buffer> m_input;

	/**
	 * The pointers to the input deinterleaved buffer data for processing
	 */
	std::vector<sample_t*> m_processData;

	/**
	 * The output deinterleaved buffers for each channel
	 */
	std::vector<Buffer> m_output;

	/**
	 * The pointers to the output deinterleaved buffer data
	 */
	std::vector<sample_t*> m_retrieveData;

	/**
	 * The length of the output.
	 */
	int m_length;

	/**
	 * The time ratio for the stretcher.
	 */
	double m_timeRatio;

	/**
	 * The pitch scale for the stretcher.
	 */
	double m_pitchScale;

	/**
	 * Stretcher.
	 */
	RubberBandStretcher* m_stretcher;

	/**
	 * Stretcher options.
	 */
	TimeStretchQualityOptions m_quality;

	// delete copy constructor and operator=
	TimeStretchReader(const TimeStretchReader&) = delete;
	TimeStretchReader& operator=(const TimeStretchReader&) = delete;

public:
	/**
	 * Creates a new stretcher reader.
	 * \param reader The reader to read from.
	 * \param time_ratio The time ratio for the stretcher.
	 */
	TimeStretchReader(std::shared_ptr<IReader> reader, double time_ratio, double pitch_scale, TimeStretchQualityOptions quality);

	~TimeStretchReader();

	virtual void read(int& length, bool& eos, sample_t* buffer);

	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;

	/**
	 * Retrieves the current time ratio for the stretcher.
	 * \return The current time ratio value.
	 */
	double getTimeRatio() const;

	/**
	 * Sets the time ratio for the stretcher.
	 */
	void setTimeRatio(double timeRatio);

	/**
	 * Retrieves the pitch scale for the stretcher
	 * \return The current pitch scale value.
	 */
	double getPitchScale() const;

	/**
	 * Sets the pitch scale for the stretcher.
	 */
	void setPitchScale(double pitchScale);

	/**
	 * Sets the configuration for the stretcher
	 */
	void configure(TimeStretchQualityOptions quality);
};

AUD_NAMESPACE_END