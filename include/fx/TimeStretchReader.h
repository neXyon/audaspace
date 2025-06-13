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

 #pragma once

 /**
  * @file ShiftReader.h
  * @ingroup fx
  * The ShiftReader class.
  */
 
#include "fx/EffectReader.h"
#include "rubberband/RubberBandStretcher.h"

using namespace RubberBand;
AUD_NAMESPACE_BEGIN
 
 /**
  * This class reads another reader and changes the playback speed while preserving the pitch.
  */
class AUD_API TimeStretchReader : public EffectReader
{
private:

	/**
	 * Specs.
	 */
	Specs m_specs;
  
  /**
	 * The current position.
	 */
  int m_position;

  /**
   * The size of buffer while passing data to the stretcher. 
   */
  int m_buffersize;

  /**
	 * The length of the output.
	 */
  int m_length;
  
  
  /**
   * The time ratio for the stretcher.
   */
  double m_ratio;

  /**
   * Data retrieved from the stretcher.
   */
  sample_t* m_processedData;


  /**
   * Stretcher
   */
  RubberBandStretcher m_stretcher;

  // delete copy constructor and operator=
  TimeStretchReader(const TimeStretchReader&) = delete;
  TimeStretchReader& operator=(const TimeStretchReader&) = delete;

public:
  /**
   * Creates a new stretcher reader.
   * \param reader The reader to read from.
   * \param time_ratio The time ratio for the stretcher
   */
  TimeStretchReader(std::shared_ptr<IReader> reader, double time_ratio, int buffersize);
  virtual ~TimeStretchReader();


	virtual void read(int& length, bool& eos, sample_t* buffer);

	virtual void seek(int position);
	virtual int getLength() const;
	virtual int getPosition() const;

  /**
   * Retrieves the time \ratio for the stretcher.
   * \return The current time ratio value.
   */
  float getRatio() const;
};

AUD_NAMESPACE_END
