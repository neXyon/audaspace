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
 * @file Audaspace.h
 * The main header file of the library defining the namespace and basic data types.
 */

/// The default playback buffer size of a device.
#define AUD_DEFAULT_BUFFER_SIZE 1024

#define AUD_NAMESPACE_BEGIN namespace aud {
#define AUD_NAMESPACE_END }

AUD_NAMESPACE_BEGIN

/// Sample type.(float samples)
typedef float sample_t;

/// Sample data type (format samples)
typedef unsigned char data_t;

AUD_NAMESPACE_END
