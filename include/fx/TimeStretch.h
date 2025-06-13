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
  * @file TimeStretch.h
  * @ingroup fx
  * The TimeStretch class.
  */
 
 #include "fx/Effect.h"
 
 AUD_NAMESPACE_BEGIN
 
 /**
  * This sound loops another sound.
  * \note The reader has to be seekable.
  */

 class AUD_API TimeStretch : public Effect
 {
 private:
  /**
   * The time ratio to stretch by.
   */
  const double m_ratio;

  /**
   * The size of buffer while passing data to the stretcher. 
   */
  int m_buffersize;
 
   // delete copy constructor and operator=
   TimeStretch(const TimeStretch&) = delete;
   TimeStretch& operator=(const TimeStretch&) = delete;
 
 public:
   /**
    * Creates a new pitch-corrected sound after stretching by the given ratio.
    * \param sound The input sound.
    * \param ratio The ratio for the stretcher
    */
   TimeStretch(std::shared_ptr<ISound> sound, double ratio, int buffersize);
 
   /**
    * Returns the time ratio.
    */
   double getTimeRatio() const;
 
   virtual std::shared_ptr<IReader> createReader();
 };
 
 AUD_NAMESPACE_END
 