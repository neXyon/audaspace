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

#include "PySound.h"

#include "generator/Sine.h"
#include "file/File.h"
#include "Exception.h"
#include "fx/Lowpass.h"
#include "fx/Delay.h"
#include "sequence/Double.h"
#include "fx/Fader.h"
#include "fx/Highpass.h"
#include "fx/Limiter.h"
#include "fx/Loop.h"
#include "sequence/PingPong.h"
#include "fx/Pitch.h"
#include "fx/Reverse.h"
#include "fx/Square.h"
#include "fx/Volume.h"
#include "fx/IIRFilter.h"
#include "util/StreamBuffer.h"
#include "sequence/Superpose.h"


#include <structmember.h>

using namespace aud;

extern PyObject *AUDError;

static void
Sound_dealloc(Sound* self)
{
	if(self->sound)
		delete reinterpret_cast<std::shared_ptr<ISound>*>(self->sound);
	Py_XDECREF(self->child_list);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
Sound_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Sound *self;

	self = (Sound*)type->tp_alloc(type, 0);
	if(self != nullptr)
	{
		static const char *kwlist[] = {"filename", nullptr};
		const char* filename = nullptr;

		if(!PyArg_ParseTupleAndKeywords(args, kwds, "s:Sound", const_cast<char**>(kwlist), &filename))
		{
			Py_DECREF(self);
			return nullptr;
		}

		try
		{
			self->sound = new std::shared_ptr<ISound>(new File(filename));
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

PyDoc_STRVAR(M_aud_Sound_sine_doc,
			 "sine(frequency, rate=44100)\n\n"
			 "Creates a sine sound which plays a sine wave.\n\n"
			 ":arg frequency: The frequency of the sine wave in Hz.\n"
			 ":type frequency: float\n"
			 ":arg rate: The sampling rate in Hz. It's recommended to set this "
			 "value to the playback device's samling rate to avoid resamping.\n"
			 ":type rate: int\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`");

static PyObject *
Sound_sine(PyTypeObject* type, PyObject *args)
{
	float frequency;
	double rate = 44100;

	if(!PyArg_ParseTuple(args, "f|d:sine", &frequency, &rate))
		return nullptr;

	Sound *self;

	self = (Sound*)type->tp_alloc(type, 0);
	if(self != nullptr)
	{
		try
		{
			self->sound = new std::shared_ptr<ISound>(new Sine(frequency, (SampleRate)rate));
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

PyDoc_STRVAR(M_aud_Sound_file_doc,
			 "file(filename)\n\n"
			 "Creates a sound object of a sound file.\n\n"
			 ":arg filename: Path of the file.\n"
			 ":type filename: string\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. warning:: If the file doesn't exist or can't be read you will "
			 "not get an exception immediately, but when you try to start "
			 "playback of that sound.");

static PyObject *
Sound_file(PyTypeObject* type, PyObject *args)
{
	const char* filename = nullptr;

	if(!PyArg_ParseTuple(args, "s:file", &filename))
		return nullptr;

	Sound *self;

	self = (Sound*)type->tp_alloc(type, 0);
	if(self != nullptr)
	{
		try
		{
			self->sound = new std::shared_ptr<ISound>(new File(filename));
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

PyDoc_STRVAR(M_aud_Sound_lowpass_doc,
			 "lowpass(frequency, Q=0.5)\n\n"
			 "Creates a second order lowpass filter based on the transfer "
			 "function H(s) = 1 / (s^2 + s/Q + 1)\n\n"
			 ":arg frequency: The cut off trequency of the lowpass.\n"
			 ":type frequency: float\n"
			 ":arg Q: Q factor of the lowpass.\n"
			 ":type Q: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`");

static PyObject *
Sound_lowpass(Sound* self, PyObject *args)
{
	float frequency;
	float Q = 0.5;

	if(!PyArg_ParseTuple(args, "f|f:lowpass", &frequency, &Q))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Lowpass(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), frequency, Q));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_delay_doc,
			 "delay(time)\n\n"
			 "Delays by playing adding silence in front of the other sound's "
			 "data.\n\n"
			 ":arg time: How many seconds of silence should be added before "
			 "the sound.\n"
			 ":type time: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`");

static PyObject *
Sound_delay(Sound* self, PyObject *args)
{
	float delay;

	if(!PyArg_ParseTuple(args, "f:delay", &delay))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Delay(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), delay));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_join_doc,
			 "join(sound)\n\n"
			 "Plays two factories in sequence.\n\n"
			 ":arg sound: The sound to play second.\n"
			 ":type sound: :class:`Sound`\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: The two factories have to have the same specifications "
			 "(channels and samplerate).");

static PyObject *
Sound_join(Sound* self, PyObject *object)
{
	PyTypeObject* type = Py_TYPE(self);

	if(!PyObject_TypeCheck(object, type))
	{
		PyErr_SetString(PyExc_TypeError, "Object has to be of type Sound!");
		return nullptr;
	}

	Sound *parent;
	Sound *child = (Sound*)object;

	parent = (Sound*)type->tp_alloc(type, 0);
	if(parent != nullptr)
	{
		parent->child_list = Py_BuildValue("(OO)", self, object);

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Double(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), *reinterpret_cast<std::shared_ptr<ISound>*>(child->sound)));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_highpass_doc,
			 "highpass(frequency, Q=0.5)\n\n"
			 "Creates a second order highpass filter based on the transfer "
			 "function H(s) = s^2 / (s^2 + s/Q + 1)\n\n"
			 ":arg frequency: The cut off trequency of the highpass.\n"
			 ":type frequency: float\n"
			 ":arg Q: Q factor of the lowpass.\n"
			 ":type Q: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`");

static PyObject *
Sound_highpass(Sound* self, PyObject *args)
{
	float frequency;
	float Q = 0.5;

	if(!PyArg_ParseTuple(args, "f|f:highpass", &frequency, &Q))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Highpass(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), frequency, Q));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_limit_doc,
			 "limit(start, end)\n\n"
			 "Limits a sound within a specific start and end time.\n\n"
			 ":arg start: Start time in seconds.\n"
			 ":type start: float\n"
			 ":arg end: End time in seconds.\n"
			 ":type end: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`");

static PyObject *
Sound_limit(Sound* self, PyObject *args)
{
	float start, end;

	if(!PyArg_ParseTuple(args, "ff:limit", &start, &end))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Limiter(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), start, end));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_pitch_doc,
			 "pitch(factor)\n\n"
			 "Changes the pitch of a sound with a specific factor.\n\n"
			 ":arg factor: The factor to change the pitch with.\n"
			 ":type factor: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: This is done by changing the sample rate of the "
			 "underlying sound, which has to be an integer, so the factor "
			 "value rounded and the factor may not be 100 % accurate.\n\n"
			 ".. note:: This is a filter function, you might consider using "
			 ":attr:`Handle.pitch` instead.");

static PyObject *
Sound_pitch(Sound* self, PyObject *args)
{
	float factor;

	if(!PyArg_ParseTuple(args, "f:pitch", &factor))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Pitch(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), factor));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_volume_doc,
			 "volume(volume)\n\n"
			 "Changes the volume of a sound.\n\n"
			 ":arg volume: The new volume..\n"
			 ":type volume: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: Should be in the range [0, 1] to avoid clipping.\n\n"
			 ".. note:: This is a filter function, you might consider using "
			 ":attr:`Handle.volume` instead.");

static PyObject *
Sound_volume(Sound* self, PyObject *args)
{
	float volume;

	if(!PyArg_ParseTuple(args, "f:volume", &volume))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Volume(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), volume));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_fadein_doc,
			 "fadein(start, length)\n\n"
			 "Fades a sound in by raising the volume linearly in the given "
			 "time interval.\n\n"
			 ":arg start: Time in seconds when the fading should start.\n"
			 ":type start: float\n"
			 ":arg length: Time in seconds how long the fading should last.\n"
			 ":type length: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: Before the fade starts it plays silence.");

static PyObject *
Sound_fadein(Sound* self, PyObject *args)
{
	float start, length;

	if(!PyArg_ParseTuple(args, "ff:fadein", &start, &length))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Fader(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), FADE_IN, start, length));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_fadeout_doc,
			 "fadeout(start, length)\n\n"
			 "Fades a sound in by lowering the volume linearly in the given "
			 "time interval.\n\n"
			 ":arg start: Time in seconds when the fading should start.\n"
			 ":type start: float\n"
			 ":arg length: Time in seconds how long the fading should last.\n"
			 ":type length: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: After the fade this sound plays silence, so that "
			 "the length of the sound is not altered.");

static PyObject *
Sound_fadeout(Sound* self, PyObject *args)
{
	float start, length;

	if(!PyArg_ParseTuple(args, "ff:fadeout", &start, &length))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Fader(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), FADE_OUT, start, length));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_loop_doc,
			 "loop(count)\n\n"
			 "Loops a sound.\n\n"
			 ":arg count: How often the sound should be looped. "
			 "Negative values mean endlessly.\n"
			 ":type count: integer\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: This is a filter function, you might consider using "
			 ":attr:`Handle.loop_count` instead.");

static PyObject *
Sound_loop(Sound* self, PyObject *args)
{
	int loop;

	if(!PyArg_ParseTuple(args, "i:loop", &loop))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Loop(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), loop));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_mix_doc,
			 "mix(sound)\n\n"
			 "Mixes two factories.\n\n"
			 ":arg sound: The sound to mix over the other.\n"
			 ":type sound: :class:`Sound`\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: The two factories have to have the same specifications "
			 "(channels and samplerate).");

static PyObject *
Sound_mix(Sound* self, PyObject *object)
{
	PyTypeObject* type = Py_TYPE(self);

	if(!PyObject_TypeCheck(object, type))
	{
		PyErr_SetString(PyExc_TypeError, "Object is not of type Sound!");
		return nullptr;
	}

	Sound *parent = (Sound*)type->tp_alloc(type, 0);
	Sound *child = (Sound*)object;

	if(parent != nullptr)
	{
		parent->child_list = Py_BuildValue("(OO)", self, object);

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Superpose(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), *reinterpret_cast<std::shared_ptr<ISound>*>(child->sound)));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_pingpong_doc,
			 "pingpong()\n\n"
			 "Plays a sound forward and then backward.\n"
			 "This is like joining a sound with its reverse.\n\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`");

static PyObject *
Sound_pingpong(Sound* self)
{
	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new PingPong(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound)));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_reverse_doc,
			 "reverse()\n\n"
			 "Plays a sound reversed.\n\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: The sound has to have a finite length and has to be "
			 "seekable. It's recommended to use this only with factories	 with "
			 "fast and accurate seeking, which is not true for encoded audio "
			 "files, such ones should be buffered using :meth:`buffer` before "
			 "being played reversed.\n\n"
			 ".. warning:: If seeking is not accurate in the underlying sound "
			 "you'll likely hear skips/jumps/cracks.");

static PyObject *
Sound_reverse(Sound* self)
{
	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Reverse(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound)));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_buffer_doc,
			 "buffer()\n\n"
			 "Buffers a sound into RAM.\n"
			 "This saves CPU usage needed for decoding and file access if the "
			 "underlying sound reads from a file on the harddisk, but it "
			 "consumes a lot of memory.\n\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`\n\n"
			 ".. note:: Only known-length factories can be buffered.\n\n"
			 ".. warning:: Raw PCM data needs a lot of space, only buffer "
			 "short factories.");

static PyObject *
Sound_buffer(Sound* self)
{
	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		try
		{
			parent->sound = new std::shared_ptr<ISound>(new StreamBuffer(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound)));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_square_doc,
			 "square(threshold = 0)\n\n"
			 "Makes a square wave out of an audio wave by setting all samples "
			 "with a amplitude >= threshold to 1, all <= -threshold to -1 and "
			 "all between to 0.\n\n"
			 ":arg threshold: Threshold value over which an amplitude counts "
			 "non-zero.\n"
			 ":type threshold: float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`");

static PyObject *
Sound_square(Sound* self, PyObject *args)
{
	float threshold = 0;

	if(!PyArg_ParseTuple(args, "|f:square", &threshold))
		return nullptr;

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new Square(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), threshold));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

PyDoc_STRVAR(M_aud_Sound_filter_doc,
			 "filter(b, a = (1))\n\n"
			 "Filters a sound with the supplied IIR filter coefficients.\n"
			 "Without the second parameter you'll get a FIR filter.\n"
			 "If the first value of the a sequence is 0 it will be set to 1 "
			 "automatically.\n"
			 "If the first value of the a sequence is neither 0 nor 1, all "
			 "filter coefficients will be scaled by this value so that it is 1 "
			 "in the end, you don't have to scale yourself.\n\n"
			 ":arg b: The nominator filter coefficients.\n"
			 ":type b: sequence of float\n"
			 ":arg a: The denominator filter coefficients.\n"
			 ":type a: sequence of float\n"
			 ":return: The created :class:`Sound` object.\n"
			 ":rtype: :class:`Sound`");

static PyObject *
Sound_filter(Sound* self, PyObject *args)
{
	PyObject *py_b;
	PyObject *py_a = nullptr;
	Py_ssize_t py_a_len;
	Py_ssize_t py_b_len;

	if(!PyArg_ParseTuple(args, "O|O:filter", &py_b, &py_a))
		return nullptr;

	if(!PySequence_Check(py_b) || (py_a != nullptr && !PySequence_Check(py_a)))
	{
		PyErr_SetString(PyExc_TypeError, "Parameter is not a sequence!");
		return nullptr;
	}

	py_a_len= py_a ? PySequence_Size(py_a) : 0;
	py_b_len= PySequence_Size(py_b);

	if(!py_b_len || ((py_a != nullptr) && !py_b_len))
	{
		PyErr_SetString(PyExc_ValueError, "The sequence has to contain at least one value!");
		return nullptr;
	}

	std::vector<float> a, b;
	PyObject *py_value;
	float value;

	for(Py_ssize_t i = 0; i < py_b_len; i++)
	{
		py_value = PySequence_GetItem(py_b, i);
		value= (float)PyFloat_AsDouble(py_value);
		Py_DECREF(py_value);

		if (value==-1.0f && PyErr_Occurred()) {
			return nullptr;
		}

		b.push_back(value);
	}

	if(py_a)
	{
		for(Py_ssize_t i = 0; i < py_a_len; i++)
		{
			py_value = PySequence_GetItem(py_a, i);
			value= (float)PyFloat_AsDouble(py_value);
			Py_DECREF(py_value);

			if (value==-1.0f && PyErr_Occurred()) {
				return nullptr;
			}

			a.push_back(value);
		}

		if(a[0] == 0)
			a[0] = 1;
	}
	else
		a.push_back(1);

	PyTypeObject* type = Py_TYPE(self);
	Sound *parent = (Sound*)type->tp_alloc(type, 0);

	if(parent != nullptr)
	{
		Py_INCREF(self);
		parent->child_list = (PyObject *)self;

		try
		{
			parent->sound = new std::shared_ptr<ISound>(new IIRFilter(*reinterpret_cast<std::shared_ptr<ISound>*>(self->sound), b, a));
		}
		catch(Exception& e)
		{
			Py_DECREF(parent);
			PyErr_SetString(AUDError, e.what());
			return nullptr;
		}
	}

	return (PyObject *)parent;
}

static PyMethodDef Sound_methods[] = {
	{"sine", (PyCFunction)Sound_sine, METH_VARARGS | METH_CLASS,
	 M_aud_Sound_sine_doc
	},
	{"file", (PyCFunction)Sound_file, METH_VARARGS | METH_CLASS,
	 M_aud_Sound_file_doc
	},
	{"lowpass", (PyCFunction)Sound_lowpass, METH_VARARGS,
	 M_aud_Sound_lowpass_doc
	},
	{"delay", (PyCFunction)Sound_delay, METH_VARARGS,
	 M_aud_Sound_delay_doc
	},
	{"join", (PyCFunction)Sound_join, METH_O,
	 M_aud_Sound_join_doc
	},
	{"highpass", (PyCFunction)Sound_highpass, METH_VARARGS,
	 M_aud_Sound_highpass_doc
	},
	{"limit", (PyCFunction)Sound_limit, METH_VARARGS,
	 M_aud_Sound_limit_doc
	},
	{"pitch", (PyCFunction)Sound_pitch, METH_VARARGS,
	 M_aud_Sound_pitch_doc
	},
	{"volume", (PyCFunction)Sound_volume, METH_VARARGS,
	 M_aud_Sound_volume_doc
	},
	{"fadein", (PyCFunction)Sound_fadein, METH_VARARGS,
	 M_aud_Sound_fadein_doc
	},
	{"fadeout", (PyCFunction)Sound_fadeout, METH_VARARGS,
	 M_aud_Sound_fadeout_doc
	},
	{"loop", (PyCFunction)Sound_loop, METH_VARARGS,
	 M_aud_Sound_loop_doc
	},
	{"mix", (PyCFunction)Sound_mix, METH_O,
	 M_aud_Sound_mix_doc
	},
	{"pingpong", (PyCFunction)Sound_pingpong, METH_NOARGS,
	 M_aud_Sound_pingpong_doc
	},
	{"reverse", (PyCFunction)Sound_reverse, METH_NOARGS,
	 M_aud_Sound_reverse_doc
	},
	{"buffer", (PyCFunction)Sound_buffer, METH_NOARGS,
	 M_aud_Sound_buffer_doc
	},
	{"square", (PyCFunction)Sound_square, METH_VARARGS,
	 M_aud_Sound_square_doc
	},
	{"filter", (PyCFunction)Sound_filter, METH_VARARGS,
	 M_aud_Sound_filter_doc
	},
	{nullptr}  /* Sentinel */
};

PyDoc_STRVAR(M_aud_Sound_doc,
			 "Sound objects are immutable and represent a sound that can be "
			 "played simultaneously multiple times. They are called factories "
			 "because they create reader objects internally that are used for "
			 "playback.");

static PyTypeObject SoundType = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	"aud.Sound",               /* tp_name */
	sizeof(Sound),             /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)Sound_dealloc, /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_reserved */
	0,                         /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash  */
	0,                         /* tp_call */
	0,                         /* tp_str */
	0,                         /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,        /* tp_flags */
	M_aud_Sound_doc,           /* tp_doc */
	0,		                   /* tp_traverse */
	0,		                   /* tp_clear */
	0,		                   /* tp_richcompare */
	0,		                   /* tp_weaklistoffset */
	0,		                   /* tp_iter */
	0,		                   /* tp_iternext */
	Sound_methods,             /* tp_methods */
	0,                         /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	0,                         /* tp_init */
	0,                         /* tp_alloc */
	Sound_new,                 /* tp_new */
};

PyObject *
Sound_empty()
{
	return SoundType.tp_alloc(&SoundType, 0);
}

Sound*
checkSound(PyObject *sound)
{
	if(!PyObject_TypeCheck(sound, &SoundType))
	{
		PyErr_SetString(PyExc_TypeError, "Object is not of type Sound!");
		return nullptr;
	}

	return (Sound*)sound;
}


bool initializeSound()
{
	return PyType_Ready(&SoundType) >= 0;
}


void addSoundToModule(PyObject *module)
{
	Py_INCREF(&SoundType);
	PyModule_AddObject(module, "Sound", (PyObject *)&SoundType);
}
