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

#include "PySequence.h"

#include "PySound.h"

#include "sequence/Sequence.h"
#include "Exception.h"

#include <structmember.h>

using aud::Exception;

extern PyObject* AUDError;

// ====================================================================

static void
Sequence_dealloc(Sequence* self)
{
	if(self->sequence)
		delete reinterpret_cast<std::shared_ptr<aud::Sequence>*>(self->sequence);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
Sequence_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	Sequence* self;

	int channels = aud::CHANNELS_STEREO;
	double rate = aud::RATE_44100;
	float fps = 30.0f;
	bool muted = false;
	PyObject* mutedo = nullptr;

	self = (Sequence*)type->tp_alloc(type, 0);
	if(self != nullptr)
	{
		static const char* kwlist[] = {"channels", "rate", "fps", "muted", nullptr};

		if(!PyArg_ParseTupleAndKeywords(args, kwds, "|idfO:Sequence", const_cast<char**>(kwlist), &channels, &rate, &fps, &mutedo))
		{
			Py_DECREF(self);
			return nullptr;
		}

		if(mutedo)
		{
			if(!PyBool_Check(mutedo))
			{
				PyErr_SetString(PyExc_TypeError, "muted is not a boolean!");
				return nullptr;
			}

			muted = mutedo == Py_True;
		}

		aud::Specs specs;
		specs.channels = static_cast<aud::Channels>(channels);
		specs.rate = rate;

		try
		{
			self->sequence = new std::shared_ptr<aud::Sequence>(new aud::Sequence(specs, fps, muted));
		}
		catch(Exception& e)
		{
			Py_DECREF(self);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)self;
}

static PyMethodDef Sequence_methods[] = {
	{nullptr}  /* Sentinel */
};

static PyGetSetDef Sequence_properties[] = {
	{nullptr}  /* Sentinel */
};

PyDoc_STRVAR(M_aud_Sequence_doc,
			 "This sound represents sequenced entries to play a sound scene.");

static PyTypeObject SequenceType = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	"aud.Sequence",              /* tp_name */
	sizeof(Sequence),            /* tp_basicsize */
	0,                           /* tp_itemsize */
	(destructor)Sequence_dealloc,/* tp_dealloc */
	0,                           /* tp_print */
	0,                           /* tp_getattr */
	0,                           /* tp_setattr */
	0,                           /* tp_reserved */
	0,                           /* tp_repr */
	0,                           /* tp_as_number */
	0,                           /* tp_as_sequence */
	0,                           /* tp_as_mapping */
	0,                           /* tp_hash  */
	0,                           /* tp_call */
	0,                           /* tp_str */
	0,                           /* tp_getattro */
	0,                           /* tp_setattro */
	0,                           /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,          /* tp_flags */
	M_aud_Sequence_doc,          /* tp_doc */
	0,		                     /* tp_traverse */
	0,		                     /* tp_clear */
	0,		                     /* tp_richcompare */
	0,		                     /* tp_weaklistoffset */
	0,		                     /* tp_iter */
	0,		                     /* tp_iternext */
	Sequence_methods,            /* tp_methods */
	0,                           /* tp_members */
	Sequence_properties,         /* tp_getset */
	0,                           /* tp_base */
	0,                           /* tp_dict */
	0,                           /* tp_descr_get */
	0,                           /* tp_descr_set */
	0,                           /* tp_dictoffset */
	0,                           /* tp_init */
	0,                           /* tp_alloc */
	Sequence_new,                /* tp_new */
};

PyObject* Sequence_empty()
{
	return SequenceType.tp_alloc(&SequenceType, 0);
}


Sequence* checkSequence(PyObject* sequence)
{
	if(!PyObject_TypeCheck(sequence, &SequenceType))
	{
		PyErr_SetString(PyExc_TypeError, "Object is not of type Sequence!");
		return nullptr;
	}

	return (Sequence*)sequence;
}


bool initializeSequence()
{
	return PyType_Ready(&SequenceType) >= 0;
}


void addSequenceToModule(PyObject* module)
{
	Py_INCREF(&SequenceType);
	PyModule_AddObject(module, "Sequence", (PyObject *)&SequenceType);
}
