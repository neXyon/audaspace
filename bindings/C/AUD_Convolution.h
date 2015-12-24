/*******************************************************************************
* Copyright 2009-2015 Juan Francisco Crespo Galán
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

#include "AUD_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern AUD_API AUD_ThreadPool* AUD_ThreadPool_create(int nThreads);
extern AUD_API void AUD_ThreadPool_free(AUD_ThreadPool* threadPool);
extern AUD_API AUD_ImpulseResponse* AUD_ImpulseResponse_create(AUD_Sound* sound);
extern AUD_API void AUD_ImpulseResponse_free(AUD_ImpulseResponse* filter);
extern AUD_API AUD_Sound* AUD_Sound_Convolver_create(AUD_Sound* sound, AUD_ImpulseResponse* filter, AUD_ThreadPool* threadPool);

#ifdef __cplusplus
}
#endif