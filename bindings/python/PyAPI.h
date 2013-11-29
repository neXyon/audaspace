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

#include "Python.h"

#ifdef __cplusplus
extern "C" {
#else
typedef void ISound;
typedef void IDevice;
typedef void IHandle;
#endif

typedef void Reference_ISound;
typedef void Reference_IDevice;
typedef void Reference_IHandle;

typedef struct {
	PyObject_HEAD
	PyObject *child_list;
	Reference_ISound* sound;
} Sound;

typedef struct {
	PyObject_HEAD
	Reference_IHandle* handle;
} Handle;

typedef struct {
	PyObject_HEAD
	Reference_IDevice* device;
} Device;

PyMODINIT_FUNC
PyInit_aud(void);

extern PyObject *Device_empty();
extern PyObject *Sound_empty();
extern Sound  *checkSound(PyObject *sound);

#ifdef __cplusplus
}
#endif
