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

#include <cstring>

AUD_NAMESPACE_BEGIN

typedef void (*convert_f)(data_t* target, data_t* source, int length);

template <class T>
void convert_copy(data_t* target, data_t* source, int length)
{
	std::memcpy(target, source, length*sizeof(T));
}

void convert_u8_s16(data_t* target, data_t* source, int length);

void convert_u8_s24_be(data_t* target, data_t* source, int length);

void convert_u8_s24_le(data_t* target, data_t* source, int length);

void convert_u8_s32(data_t* target, data_t* source, int length);

void convert_u8_float(data_t* target, data_t* source, int length);

void convert_u8_double(data_t* target, data_t* source, int length);

void convert_s16_u8(data_t* target, data_t* source, int length);

void convert_s16_s24_be(data_t* target, data_t* source, int length);

void convert_s16_s24_le(data_t* target, data_t* source, int length);

void convert_s16_s32(data_t* target, data_t* source, int length);

void convert_s16_float(data_t* target, data_t* source, int length);

void convert_s16_double(data_t* target, data_t* source, int length);

void convert_s24_u8_be(data_t* target, data_t* source, int length);

void convert_s24_u8_le(data_t* target, data_t* source, int length);

void convert_s24_s16_be(data_t* target, data_t* source, int length);

void convert_s24_s16_le(data_t* target, data_t* source, int length);

void convert_s24_s24(data_t* target, data_t* source, int length);

void convert_s24_s32_be(data_t* target, data_t* source, int length);

void convert_s24_s32_le(data_t* target, data_t* source, int length);

void convert_s24_float_be(data_t* target, data_t* source, int length);

void convert_s24_float_le(data_t* target, data_t* source, int length);

void convert_s24_double_be(data_t* target, data_t* source, int length);

void convert_s24_double_le(data_t* target, data_t* source, int length);

void convert_s32_u8(data_t* target, data_t* source, int length);

void convert_s32_s16(data_t* target, data_t* source, int length);

void convert_s32_s24_be(data_t* target, data_t* source, int length);

void convert_s32_s24_le(data_t* target, data_t* source, int length);

void convert_s32_float(data_t* target, data_t* source, int length);

void convert_s32_double(data_t* target, data_t* source, int length);

void convert_float_u8(data_t* target, data_t* source, int length);

void convert_float_s16(data_t* target, data_t* source, int length);

void convert_float_s24_be(data_t* target, data_t* source, int length);

void convert_float_s24_le(data_t* target, data_t* source, int length);

void convert_float_s32(data_t* target, data_t* source, int length);

void convert_float_double(data_t* target, data_t* source, int length);

void convert_double_u8(data_t* target, data_t* source, int length);

void convert_double_s16(data_t* target, data_t* source, int length);

void convert_double_s24_be(data_t* target, data_t* source, int length);

void convert_double_s24_le(data_t* target, data_t* source, int length);

void convert_double_s32(data_t* target, data_t* source, int length);

void convert_double_float(data_t* target, data_t* source, int length);

AUD_NAMESPACE_END
