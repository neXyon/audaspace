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
#include "PyAnimateableProperty.h"

#include "Exception.h"

#include "sequence/AnimateableProperty.h"

// #include "PySound.h"
// #include "PyHandle.h"

// #include "Exception.h"
// #include "devices/IDevice.h"
// #include "devices/I3DDevice.h"
// #include "devices/DeviceManager.h"
// #include "devices/IDeviceFactory.h"

// #include <structmember.h>

using namespace aud;

extern PyObject* AUDError;

static PyObject* AnimateableProperty_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	AnimateablePropertyP* self = (AnimateablePropertyP*) type->tp_alloc(type, 0);

	int count;
	float value = 0.0f;

	if(self != nullptr)
	{
		PyObject* object;
		if(!PyArg_ParseTuple(args, "i|f:animateableProperty", &count, &value))
			return nullptr;

		try
		{
			if(PyTuple_Size(args) == 1)
			{
				self->animateableProperty = new std::shared_ptr<aud::AnimateableProperty>(new aud::AnimateableProperty(count));
			}
			else
			{
				self->animateableProperty = new std::shared_ptr<aud::AnimateableProperty>(new aud::AnimateableProperty(count, value));
			}
		}
		catch(aud::Exception& e)
		{
			Py_DECREF(self);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject*) self;
}

static void AnimateableProperty_dealloc(AnimateablePropertyP* self)
{
	if(self->animateableProperty)
		delete reinterpret_cast<std::shared_ptr<aud::AnimateableProperty>*>(self->animateableProperty);
	Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyMethodDef AnimateableProperty_methods[] = {
    {(char*) "writeConstantRange", (PyCFunction) AnimateableProperty_writeConstantRange, METH_VARARGS, M_aud_AnimateableProperty_writeConstantRange_doc},
    {(char*) "read", (PyCFunction) AnimateableProperty_read, METH_VARARGS, M_aud_AnimateableProperty_read_doc},
    {(char*) "readSingle", (PyCFunction) AnimateableProperty_readSingle, METH_VARARGS, M_aud_AnimateableProperty_readSingle_doc},
    {(char*) "write", (PyCFunction) AnimateableProperty_write, METH_VARARGS, M_aud_AnimateableProperty_write_doc},
    {nullptr} /* Sentinel */
};

static PyGetSetDef AnimateableProperty_properties[] = {
    {(char*) "count", (getter) AnimateableProperty_get_count, nullptr, M_aud_AnimateableProperty_count_doc, nullptr},
    {(char*) "animated", (getter) AnimateableProperty_get_animated, nullptr, M_aud_AnimateableProperty_animated_doc, nullptr} {nullptr} /* Sentinel */
};

PyDoc_STRVAR(M_aud_AnimateableProperty_doc, "An AnimateableProperty object stores an array of float values for animating sound properties (e.g. pan, volume, pitch-scale)");

// Note that AnimateablePropertyType name is already taken - a better name can be selected
PyTypeObject AnimateablePropertyPyType = {
    PyVarObject_HEAD_INIT(nullptr, 0) "aud.AnimateableProperty", /* tp_name */
    sizeof(AnimateablePropertyP),                                /* tp_basicsize */
    0,                                                           /* tp_itemsize */
    (destructor) AnimateableProperty_dealloc,                    /* tp_dealloc */
    0,                                                           /* tp_print */
    0,                                                           /* tp_getattr */
    0,                                                           /* tp_setattr */
    0,                                                           /* tp_reserved */
    0,                                                           /* tp_repr */
    0,                                                           /* tp_as_number */
    0,                                                           /* tp_as_sequence */
    0,                                                           /* tp_as_mapping */
    0,                                                           /* tp_hash  */
    0,                                                           /* tp_call */
    0,                                                           /* tp_str */
    0,                                                           /* tp_getattro */
    0,                                                           /* tp_setattro */
    0,                                                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                                          /* tp_flags */
    M_aud_AnimateableProperty_doc,                               /* tp_doc */
    0,                                                           /* tp_traverse */
    0,                                                           /* tp_clear */
    0,                                                           /* tp_richcompare */
    0,                                                           /* tp_weaklistoffset */
    0,                                                           /* tp_iter */
    0,                                                           /* tp_iternext */
    AnimateableProperty_methods,                                 /* tp_methods */
    0,                                                           /* tp_members */
    AnimateableProperty_properties,                              /* tp_getset */
    0,                                                           /* tp_base */
    0,                                                           /* tp_dict */
    0,                                                           /* tp_descr_get */
    0,                                                           /* tp_descr_set */
    0,                                                           /* tp_dictoffset */
    0,                                                           /* tp_init */
    0,                                                           /* tp_alloc */
    AnimateableProperty_new,                                     /* tp_new */
};

AUD_API PyObject* AnimateableProperty_empty()
{
	return AnimateablePropertyPyType.tp_alloc(&AnimateablePropertyPyType, 0);
}

AUD_API AnimateablePropertyP* checkAnimateableProperty(PyObject* animateableProperty)
{
	if(!PyObject_TypeCheck(animateableProperty, &AnimateablePropertyPyType))
	{
		PyErr_SetString(PyExc_TypeError, "Object is not of type AnimateableProperty!");
		return nullptr;
	}

	return (AnimateablePropertyP*) animateableProperty;
}

bool initializeAnimateableProperty()
{
	return PyType_Ready(&AnimateablePropertyPyType) >= 0;
}

void addAnimateablePropertyToModule(PyObject* module)
{
	Py_INCREF(&AnimateablePropertyPyType);
	PyModule_AddObject(module, "AnimateableProperty", (PyObject*) &AnimateablePropertyPyType);
}
