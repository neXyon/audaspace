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

#include "PyAPI.h"
#include "PySound.h"
#include "PyHandle.h"
#include "PyDevice.h"
#include "PySequenceEntry.h"
#include "PySequence.h"

#include "respec/Specification.h"
#include "devices/IHandle.h"
#include "devices/I3DDevice.h"
#include "plugin/PluginManager.h"
#include "ISound.h"

#include <memory>

#include <structmember.h>

using namespace aud;

// ====================================================================

#define PY_MODULE_ADD_CONSTANT(module, name) PyModule_AddIntConstant(module, #name, name)

// ====================================================================

extern PyObject* AUDError;
PyObject* AUDError = nullptr;

// ====================================================================

PyDoc_STRVAR(M_aud_doc,
			 "This module provides access to the audaspace audio library.");

static struct PyModuleDef audmodule = {
	PyModuleDef_HEAD_INIT,
	"aud",     /* name of module */
	M_aud_doc, /* module documentation */
	-1,        /* size of per-interpreter state of the module,
				  or -1 if the module keeps state in global variables. */
   nullptr, nullptr, nullptr, nullptr, nullptr
};

PyMODINIT_FUNC
PyInit_aud()
{
	PyObject* module;

	PluginManager::loadPlugins();

	if(!initializeSound())
		return nullptr;

	if(!initializeDevice())
		return nullptr;

	if(!initializeHandle())
		return nullptr;

	if(!initializeSequenceEntry())
		return nullptr;

	if(!initializeSequence())
		return nullptr;

	module = PyModule_Create(&audmodule);
	if(module == nullptr)
		return nullptr;

	addSoundToModule(module);
	addHandleToModule(module);
	addDeviceToModule(module);
	addSequenceEntryToModule(module);
	addSequenceToModule(module);

	AUDError = PyErr_NewException("aud.error", nullptr, nullptr);
	Py_INCREF(AUDError);
	PyModule_AddObject(module, "error", AUDError);

	// format constants
	PY_MODULE_ADD_CONSTANT(module, FORMAT_FLOAT32);
	PY_MODULE_ADD_CONSTANT(module, FORMAT_FLOAT64);
	PY_MODULE_ADD_CONSTANT(module, FORMAT_INVALID);
	PY_MODULE_ADD_CONSTANT(module, FORMAT_S16);
	PY_MODULE_ADD_CONSTANT(module, FORMAT_S24);
	PY_MODULE_ADD_CONSTANT(module, FORMAT_S32);
	PY_MODULE_ADD_CONSTANT(module, FORMAT_U8);
	// status constants
	PY_MODULE_ADD_CONSTANT(module, STATUS_INVALID);
	PY_MODULE_ADD_CONSTANT(module, STATUS_PAUSED);
	PY_MODULE_ADD_CONSTANT(module, STATUS_PLAYING);
	PY_MODULE_ADD_CONSTANT(module, STATUS_STOPPED);
	// distance model constants
	PY_MODULE_ADD_CONSTANT(module, DISTANCE_MODEL_EXPONENT);
	PY_MODULE_ADD_CONSTANT(module, DISTANCE_MODEL_EXPONENT_CLAMPED);
	PY_MODULE_ADD_CONSTANT(module, DISTANCE_MODEL_INVERSE);
	PY_MODULE_ADD_CONSTANT(module, DISTANCE_MODEL_INVERSE_CLAMPED);
	PY_MODULE_ADD_CONSTANT(module, DISTANCE_MODEL_LINEAR);
	PY_MODULE_ADD_CONSTANT(module, DISTANCE_MODEL_LINEAR_CLAMPED);
	PY_MODULE_ADD_CONSTANT(module, DISTANCE_MODEL_INVALID);

	return module;
}

PyObject* AUD_getPythonSound(void* sound)
{
	if(sound)
	{
		Sound* object = (Sound*) Sound_empty();
		if(object)
		{
			object->sound = new std::shared_ptr<ISound>(*reinterpret_cast<std::shared_ptr<ISound>*>(sound));
			return (PyObject *) object;
		}
	}

	return nullptr;
}

void* AUD_getSoundFromPython(PyObject* object)
{
	Sound* sound = checkSound(object);

	if(!sound)
		return nullptr;

	return new std::shared_ptr<ISound>(*reinterpret_cast<std::shared_ptr<ISound>*>(sound->sound));
}
