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
#include <structmember.h>

#include "devices/I3DDevice.h"
#include "devices/I3DHandle.h"
#include "devices/NULLDevice.h"
#include "fx/Delay.h"
#include "fx/Double.h"
#include "fx/Fader.h"
#include "fx/Highpass.h"
#include "fx/Limiter.h"
#include "fx/Loop.h"
#include "fx/Lowpass.h"
#include "fx/PingPong.h"
#include "fx/Pitch.h"
#include "fx/Reverse.h"
#include "generator/Sinus.h"
#include "file/File.h"
#include "fx/Square.h"
#include "util/StreamBuffer.h"
#include "fx/Superpose.h"
#include "fx/Volume.h"
#include "fx/IIRFilter.h"
#include "devices/SDLDevice.h"
#include "devices/OpenALDevice.h"
#include "devices/JackDevice.h"

using namespace aud;

// ====================================================================

typedef enum
{
	DEVICE_NULL = 0,
	DEVICE_OPENAL,
	DEVICE_SDL,
	DEVICE_JACK,
	DEVICE_READ,
} DeviceTypes;

// ====================================================================

#define PY_MODULE_ADD_CONSTANT(module, name) PyModule_AddIntConstant(module, #name, name)

// ====================================================================

static PyObject *AUDError;

static const char* device_not_3d_error = "Device is not a 3D device!";

// ====================================================================

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
			PyErr_SetString(AUDError, e.str);
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
			self->sound = new std::shared_ptr<ISound>(new Sinus(frequency, (SampleRate)rate));
		}
		catch(Exception& e)
		{
			Py_DECREF(self);
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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
			PyErr_SetString(AUDError, e.str);
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

// ========== Handle ==================================================

static void
Handle_dealloc(Handle* self)
{
	if(self->handle)
		delete reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

PyDoc_STRVAR(M_aud_Handle_pause_doc,
			 "pause()\n\n"
			 "Pauses playback.\n\n"
			 ":return: Whether the action succeeded.\n"
			 ":rtype: bool");

static PyObject *
Handle_pause(Handle *self)
{
	try
	{
		return PyBool_FromLong((long)(*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->pause());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

PyDoc_STRVAR(M_aud_Handle_resume_doc,
			 "resume()\n\n"
			 "Resumes playback.\n\n"
			 ":return: Whether the action succeeded.\n"
			 ":rtype: bool");

static PyObject *
Handle_resume(Handle *self)
{
	try
	{
		return PyBool_FromLong((long)(*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->resume());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

PyDoc_STRVAR(M_aud_Handle_stop_doc,
			 "stop()\n\n"
			 "Stops playback.\n\n"
			 ":return: Whether the action succeeded.\n"
			 ":rtype: bool\n\n"
			 ".. note:: This makes the handle invalid.");

static PyObject *
Handle_stop(Handle *self)
{
	try
	{
		return PyBool_FromLong((long)(*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->stop());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static PyMethodDef Handle_methods[] = {
	{"pause", (PyCFunction)Handle_pause, METH_NOARGS,
	 M_aud_Handle_pause_doc
	},
	{"resume", (PyCFunction)Handle_resume, METH_NOARGS,
	 M_aud_Handle_resume_doc
	},
	{"stop", (PyCFunction)Handle_stop, METH_NOARGS,
	 M_aud_Handle_stop_doc
	},
	{nullptr}  /* Sentinel */
};

PyDoc_STRVAR(M_aud_Handle_position_doc,
			 "The playback position of the sound in seconds.");

static PyObject *
Handle_get_position(Handle *self, void* nothing)
{
	try
	{
		return Py_BuildValue("f", (*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->getPosition());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_position(Handle *self, PyObject *args, void* nothing)
{
	float position;

	if(!PyArg_Parse(args, "f:position", &position))
		return -1;

	try
	{
		if((*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->seek(position))
			return 0;
		PyErr_SetString(AUDError, "Couldn't seek the sound!");
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_keep_doc,
			 "Whether the sound should be kept paused in the device when its "
			 "end is reached.\n"
			 "This can be used to seek the sound to some position and start "
			 "playback again.\n\n"
			 ".. warning:: If this is set to true and you forget stopping this "
			 "equals a memory leak as the handle exists until the device is "
			 "destroyed.");

static PyObject *
Handle_get_keep(Handle *self, void* nothing)
{
	try
	{
		return PyBool_FromLong((long)(*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->getKeep());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_keep(Handle *self, PyObject *args, void* nothing)
{
	if(!PyBool_Check(args))
	{
		PyErr_SetString(PyExc_TypeError, "keep is not a boolean!");
		return -1;
	}

	bool keep = args == Py_True;

	try
	{
		if((*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->setKeep(keep))
			return 0;
		PyErr_SetString(AUDError, "Couldn't set keep of the sound!");
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_status_doc,
			 "Whether the sound is playing, paused or stopped (=invalid).");

static PyObject *
Handle_get_status(Handle *self, void* nothing)
{
	try
	{
		return PyBool_FromLong((long)(*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->getStatus());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

PyDoc_STRVAR(M_aud_Handle_volume_doc,
			 "The volume of the sound.");

static PyObject *
Handle_get_volume(Handle *self, void* nothing)
{
	try
	{
		return Py_BuildValue("f", (*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->getVolume());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_volume(Handle *self, PyObject *args, void* nothing)
{
	float volume;

	if(!PyArg_Parse(args, "f:volume", &volume))
		return -1;

	try
	{
		if((*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->setVolume(volume))
			return 0;
		PyErr_SetString(AUDError, "Couldn't set the sound volume!");
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_pitch_doc,
			 "The pitch of the sound.");

static PyObject *
Handle_get_pitch(Handle *self, void* nothing)
{
	try
	{
		return Py_BuildValue("f", (*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->getPitch());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_pitch(Handle *self, PyObject *args, void* nothing)
{
	float pitch;

	if(!PyArg_Parse(args, "f:pitch", &pitch))
		return -1;

	try
	{
		if((*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->setPitch(pitch))
			return 0;
		PyErr_SetString(AUDError, "Couldn't set the sound pitch!");
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_loop_count_doc,
			 "The (remaining) loop count of the sound. A negative value indicates infinity.");

static PyObject *
Handle_get_loop_count(Handle *self, void* nothing)
{
	try
	{
		return Py_BuildValue("i", (*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->getLoopCount());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_loop_count(Handle *self, PyObject *args, void* nothing)
{
	int loops;

	if(!PyArg_Parse(args, "i:loop_count", &loops))
		return -1;

	try
	{
		if((*reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle))->setLoopCount(loops))
			return 0;
		PyErr_SetString(AUDError, "Couldn't set the loop count!");
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_location_doc,
			 "The source's location in 3D space, a 3D tuple of floats.");

static PyObject *
Handle_get_location(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			Vector3 v = handle->getSourceLocation();
			return Py_BuildValue("(fff)", v.x(), v.y(), v.z());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return nullptr;
}

static int
Handle_set_location(Handle *self, PyObject *args, void* nothing)
{
	float x, y, z;

	if(!PyArg_Parse(args, "(fff):location", &x, &y, &z))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			Vector3 location(x, y, z);
			if(handle->setSourceLocation(location))
				return 0;
			PyErr_SetString(AUDError, "Location couldn't be set!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_velocity_doc,
			 "The source's velocity in 3D space, a 3D tuple of floats.");

static PyObject *
Handle_get_velocity(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			Vector3 v = handle->getSourceVelocity();
			return Py_BuildValue("(fff)", v.x(), v.y(), v.z());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return nullptr;
}

static int
Handle_set_velocity(Handle *self, PyObject *args, void* nothing)
{
	float x, y, z;

	if(!PyArg_Parse(args, "(fff):velocity", &x, &y, &z))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			Vector3 velocity(x, y, z);
			if(handle->setSourceVelocity(velocity))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the velocity!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_orientation_doc,
			 "The source's orientation in 3D space as quaternion, a 4 float tuple.");

static PyObject *
Handle_get_orientation(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			Quaternion o = handle->getSourceOrientation();
			return Py_BuildValue("(ffff)", o.w(), o.x(), o.y(), o.z());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return nullptr;
}

static int
Handle_set_orientation(Handle *self, PyObject *args, void* nothing)
{
	float w, x, y, z;

	if(!PyArg_Parse(args, "(ffff):orientation", &w, &x, &y, &z))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			Quaternion orientation(w, x, y, z);
			if(handle->setSourceOrientation(orientation))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the orientation!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_relative_doc,
			 "Whether the source's location, velocity and orientation is relative or absolute to the listener.");

static PyObject *
Handle_get_relative(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return PyBool_FromLong((long)handle->isRelative());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return nullptr;
}

static int
Handle_set_relative(Handle *self, PyObject *args, void* nothing)
{
	if(!PyBool_Check(args))
	{
		PyErr_SetString(PyExc_TypeError, "Value is not a boolean!");
		return -1;
	}

	bool relative = (args == Py_True);

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setRelative(relative))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the relativeness!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_volume_minimum_doc,
			 "The minimum volume of the source.\n\n"
			 ".. seealso:: :attr:`Device.distance_model`");

static PyObject *
Handle_get_volume_minimum(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return Py_BuildValue("f", handle->getVolumeMinimum());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_volume_minimum(Handle *self, PyObject *args, void* nothing)
{
	float volume;

	if(!PyArg_Parse(args, "f:volume_minimum", &volume))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setVolumeMinimum(volume))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the minimum volume!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_volume_maximum_doc,
			 "The maximum volume of the source.\n\n"
			 ".. seealso:: :attr:`Device.distance_model`");

static PyObject *
Handle_get_volume_maximum(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return Py_BuildValue("f", handle->getVolumeMaximum());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_volume_maximum(Handle *self, PyObject *args, void* nothing)
{
	float volume;

	if(!PyArg_Parse(args, "f:volume_maximum", &volume))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setVolumeMaximum(volume))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the maximum volume!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_distance_reference_doc,
			 "The reference distance of the source.\n"
			 "At this distance the volume will be exactly :attr:`volume`.\n\n"
			 ".. seealso:: :attr:`Device.distance_model`");

static PyObject *
Handle_get_distance_reference(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return Py_BuildValue("f", handle->getDistanceReference());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_distance_reference(Handle *self, PyObject *args, void* nothing)
{
	float distance;

	if(!PyArg_Parse(args, "f:distance_reference", &distance))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setDistanceReference(distance))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the reference distance!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_distance_maximum_doc,
			 "The maximum distance of the source.\n"
			 "If the listener is further away the source volume will be 0.\n\n"
			 ".. seealso:: :attr:`Device.distance_model`");

static PyObject *
Handle_get_distance_maximum(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return Py_BuildValue("f", handle->getDistanceMaximum());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_distance_maximum(Handle *self, PyObject *args, void* nothing)
{
	float distance;

	if(!PyArg_Parse(args, "f:distance_maximum", &distance))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setDistanceMaximum(distance))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the maximum distance!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_attenuation_doc,
			 "This factor is used for distance based attenuation of the "
			 "source.\n\n"
			 ".. seealso:: :attr:`Device.distance_model`");

static PyObject *
Handle_get_attenuation(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return Py_BuildValue("f", handle->getAttenuation());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_attenuation(Handle *self, PyObject *args, void* nothing)
{
	float factor;

	if(!PyArg_Parse(args, "f:attenuation", &factor))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setAttenuation(factor))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the attenuation!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_cone_angle_inner_doc,
			 "The opening angle of the inner cone of the source. If the cone "
			 "values of a source are set there are two (audible) cones with "
			 "the apex at the :attr:`location` of the source and with infinite "
			 "height, heading in the direction of the source's "
			 ":attr:`orientation`.\n"
			 "In the inner cone the volume is normal. Outside the outer cone "
			 "the volume will be :attr:`cone_volume_outer` and in the area "
			 "between the volume will be interpolated linearly.");

static PyObject *
Handle_get_cone_angle_inner(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return Py_BuildValue("f", handle->getConeAngleInner());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_cone_angle_inner(Handle *self, PyObject *args, void* nothing)
{
	float angle;

	if(!PyArg_Parse(args, "f:cone_angle_inner", &angle))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setConeAngleInner(angle))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the cone inner angle!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_cone_angle_outer_doc,
			 "The opening angle of the outer cone of the source.\n\n"
			 ".. seealso:: :attr:`cone_angle_inner`");

static PyObject *
Handle_get_cone_angle_outer(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return Py_BuildValue("f", handle->getConeAngleOuter());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_cone_angle_outer(Handle *self, PyObject *args, void* nothing)
{
	float angle;

	if(!PyArg_Parse(args, "f:cone_angle_outer", &angle))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setConeAngleOuter(angle))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the cone outer angle!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Handle_cone_volume_outer_doc,
			 "The volume outside the outer cone of the source.\n\n"
			 ".. seealso:: :attr:`cone_angle_inner`");

static PyObject *
Handle_get_cone_volume_outer(Handle *self, void* nothing)
{
	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			return Py_BuildValue("f", handle->getConeVolumeOuter());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Handle_set_cone_volume_outer(Handle *self, PyObject *args, void* nothing)
{
	float volume;

	if(!PyArg_Parse(args, "f:cone_volume_outer", &volume))
		return -1;

	try
	{
		I3DHandle* handle = dynamic_cast<I3DHandle*>(reinterpret_cast<std::shared_ptr<IHandle>*>(self->handle)->get());
		if(handle)
		{
			if(handle->setConeVolumeOuter(volume))
				return 0;
			PyErr_SetString(AUDError, "Couldn't set the cone outer volume!");
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

static PyGetSetDef Handle_properties[] = {
	{(char*)"position", (getter)Handle_get_position, (setter)Handle_set_position,
	 M_aud_Handle_position_doc, nullptr },
	{(char*)"keep", (getter)Handle_get_keep, (setter)Handle_set_keep,
	 M_aud_Handle_keep_doc, nullptr },
	{(char*)"status", (getter)Handle_get_status, nullptr,
	 M_aud_Handle_status_doc, nullptr },
	{(char*)"volume", (getter)Handle_get_volume, (setter)Handle_set_volume,
	 M_aud_Handle_volume_doc, nullptr },
	{(char*)"pitch", (getter)Handle_get_pitch, (setter)Handle_set_pitch,
	 M_aud_Handle_pitch_doc, nullptr },
	{(char*)"loop_count", (getter)Handle_get_loop_count, (setter)Handle_set_loop_count,
	 M_aud_Handle_loop_count_doc, nullptr },
	{(char*)"location", (getter)Handle_get_location, (setter)Handle_set_location,
	 M_aud_Handle_location_doc, nullptr },
	{(char*)"velocity", (getter)Handle_get_velocity, (setter)Handle_set_velocity,
	 M_aud_Handle_velocity_doc, nullptr },
	{(char*)"orientation", (getter)Handle_get_orientation, (setter)Handle_set_orientation,
	 M_aud_Handle_orientation_doc, nullptr },
	{(char*)"relative", (getter)Handle_get_relative, (setter)Handle_set_relative,
	 M_aud_Handle_relative_doc, nullptr },
	{(char*)"volume_minimum", (getter)Handle_get_volume_minimum, (setter)Handle_set_volume_minimum,
	 M_aud_Handle_volume_minimum_doc, nullptr },
	{(char*)"volume_maximum", (getter)Handle_get_volume_maximum, (setter)Handle_set_volume_maximum,
	 M_aud_Handle_volume_maximum_doc, nullptr },
	{(char*)"distance_reference", (getter)Handle_get_distance_reference, (setter)Handle_set_distance_reference,
	 M_aud_Handle_distance_reference_doc, nullptr },
	{(char*)"distance_maximum", (getter)Handle_get_distance_maximum, (setter)Handle_set_distance_maximum,
	 M_aud_Handle_distance_maximum_doc, nullptr },
	{(char*)"attenuation", (getter)Handle_get_attenuation, (setter)Handle_set_attenuation,
	 M_aud_Handle_attenuation_doc, nullptr },
	{(char*)"cone_angle_inner", (getter)Handle_get_cone_angle_inner, (setter)Handle_set_cone_angle_inner,
	 M_aud_Handle_cone_angle_inner_doc, nullptr },
	{(char*)"cone_angle_outer", (getter)Handle_get_cone_angle_outer, (setter)Handle_set_cone_angle_outer,
	 M_aud_Handle_cone_angle_outer_doc, nullptr },
	{(char*)"cone_volume_outer", (getter)Handle_get_cone_volume_outer, (setter)Handle_set_cone_volume_outer,
	 M_aud_Handle_cone_volume_outer_doc, nullptr },
	{nullptr}  /* Sentinel */
};

PyDoc_STRVAR(M_aud_Handle_doc,
			 "Handle objects are playback handles that can be used to control "
			 "playback of a sound. If a sound is played back multiple times "
			 "then there are as many handles.");

static PyTypeObject HandleType = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	"aud.Handle",              /* tp_name */
	sizeof(Handle),            /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)Handle_dealloc,/* tp_dealloc */
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
	M_aud_Handle_doc,          /* tp_doc */
	0,		                   /* tp_traverse */
	0,		                   /* tp_clear */
	0,		                   /* tp_richcompare */
	0,		                   /* tp_weaklistoffset */
	0,		                   /* tp_iter */
	0,		                   /* tp_iternext */
	Handle_methods,            /* tp_methods */
	0,                         /* tp_members */
	Handle_properties,         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	0,                         /* tp_init */
	0,                         /* tp_alloc */
	0,                         /* tp_new */
};

// ========== Device ==================================================

static void
Device_dealloc(Device* self)
{
	if(self->device)
		delete reinterpret_cast<std::shared_ptr<IDevice>*>(self->device);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
Device_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Device *self;

	static const char *kwlist[] = {"type", "rate", "channels", "format", "buffer_size", "name", nullptr};
	int device;
	double rate = RATE_44100;
	int channels = CHANNELS_STEREO;
	int format = FORMAT_FLOAT32;
	int buffersize = AUD_DEFAULT_BUFFER_SIZE;
	const char* name = "Audaspace";

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "i|diiis:Device", const_cast<char**>(kwlist),
									&device, &rate, &channels, &format, &buffersize, &name))
		return nullptr;

	if(buffersize < 128)
	{
		PyErr_SetString(PyExc_ValueError, "buffer_size must be greater than 127!");
		return nullptr;
	}

	self = (Device*)type->tp_alloc(type, 0);
	if(self != nullptr)
	{
		DeviceSpecs specs;
		specs.channels = (Channels)channels;
		specs.format = (SampleFormat)format;
		specs.rate = (SampleRate)rate;

		self->device = nullptr;

		try
		{
			switch(device)
			{
			case DEVICE_NULL:
				(void)specs; /* quiet warning when others disabled */
				self->device = new std::shared_ptr<IDevice>(new NULLDevice());
				break;
			case DEVICE_OPENAL:
				self->device = new std::shared_ptr<IDevice>(new OpenALDevice(specs, buffersize));
				break;
			case DEVICE_SDL:
				self->device = new std::shared_ptr<IDevice>(new SDLDevice(specs, buffersize));
				break;
			case DEVICE_JACK:
				self->device = new std::shared_ptr<IDevice>(new JackDevice(name, specs, buffersize));
				break;
			case DEVICE_READ:
				break;
			}

		}
		catch(Exception& e)
		{
			Py_DECREF(self);
			PyErr_SetString(AUDError, e.str);
			return nullptr;
		}

		if(!self->device)
		{
			Py_DECREF(self);
			PyErr_SetString(AUDError, "Unsupported device type!");
			return nullptr;
		}
	}

	return (PyObject *)self;
}

PyDoc_STRVAR(M_aud_Device_play_doc,
			 "play(sound, keep=False)\n\n"
			 "Plays a sound.\n\n"
			 ":arg sound: The sound to play.\n"
			 ":type sound: :class:`Sound`\n"
			 ":arg keep: See :attr:`Handle.keep`.\n"
			 ":type keep: bool\n"
			 ":return: The playback handle with which playback can be "
			 "controlled with.\n"
			 ":rtype: :class:`Handle`");

static PyObject *
Device_play(Device *self, PyObject *args, PyObject *kwds)
{
	PyObject *object;
	PyObject *keepo = nullptr;

	bool keep = false;

	static const char *kwlist[] = {"sound", "keep", nullptr};

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|O:play", const_cast<char**>(kwlist), &object, &keepo))
		return nullptr;

	if(!PyObject_TypeCheck(object, &SoundType))
	{
		PyErr_SetString(PyExc_TypeError, "Object is not of type Sound!");
		return nullptr;
	}

	if(keepo != nullptr)
	{
		if(!PyBool_Check(keepo))
		{
			PyErr_SetString(PyExc_TypeError, "keep is not a boolean!");
			return nullptr;
		}

		keep = keepo == Py_True;
	}

	Sound* sound = (Sound*)object;
	Handle *handle;

	handle = (Handle*)HandleType.tp_alloc(&HandleType, 0);
	if(handle != nullptr)
	{
		try
		{
			handle->handle = new std::shared_ptr<IHandle>((*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->play(*reinterpret_cast<std::shared_ptr<ISound>*>(sound->sound), keep));
		}
		catch(Exception& e)
		{
			Py_DECREF(handle);
			PyErr_SetString(AUDError, e.str);
			return nullptr;
		}
	}

	return (PyObject *)handle;
}

PyDoc_STRVAR(M_aud_Device_stopAll_doc,
			 "stopAll()\n\n"
			 "Stops all playing and paused sounds.");

static PyObject *
Device_stopAll(Device *self)
{
	try
	{
		(*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->stopAll();
		Py_RETURN_NONE;
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

PyDoc_STRVAR(M_aud_Device_lock_doc,
			 "lock()\n\n"
			 "Locks the device so that it's guaranteed, that no samples are "
			 "read from the streams until :meth:`unlock` is called.\n"
			 "This is useful if you want to do start/stop/pause/resume some "
			 "sounds at the same time.\n\n"
			 ".. note:: The device has to be unlocked as often as locked to be "
			 "able to continue playback.\n\n"
			 ".. warning:: Make sure the time between locking and unlocking is "
			 "as short as possible to avoid clicks.");

static PyObject *
Device_lock(Device *self)
{
	try
	{
		(*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->lock();
		Py_RETURN_NONE;
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

PyDoc_STRVAR(M_aud_Device_unlock_doc,
			 "unlock()\n\n"
			 "Unlocks the device after a lock call, see :meth:`lock` for "
			 "details.");

static PyObject *
Device_unlock(Device *self)
{
	try
	{
		(*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->unlock();
		Py_RETURN_NONE;
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static PyMethodDef Device_methods[] = {
	{"play", (PyCFunction)Device_play, METH_VARARGS | METH_KEYWORDS,
	 M_aud_Device_play_doc
	},
	{"stopAll", (PyCFunction)Device_stopAll, METH_NOARGS,
	 M_aud_Device_stopAll_doc
	},
	{"lock", (PyCFunction)Device_lock, METH_NOARGS,
	 M_aud_Device_lock_doc
	},
	{"unlock", (PyCFunction)Device_unlock, METH_NOARGS,
	 M_aud_Device_unlock_doc
	},
	{nullptr}  /* Sentinel */
};

PyDoc_STRVAR(M_aud_Device_rate_doc,
			 "The sampling rate of the device in Hz.");

static PyObject *
Device_get_rate(Device *self, void* nothing)
{
	try
	{
		DeviceSpecs specs = (*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->getSpecs();
		return Py_BuildValue("d", specs.rate);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

PyDoc_STRVAR(M_aud_Device_format_doc,
			 "The native sample format of the device.");

static PyObject *
Device_get_format(Device *self, void* nothing)
{
	try
	{
		DeviceSpecs specs = (*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->getSpecs();
		return Py_BuildValue("i", specs.format);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

PyDoc_STRVAR(M_aud_Device_channels_doc,
			 "The channel count of the device.");

static PyObject *
Device_get_channels(Device *self, void* nothing)
{
	try
	{
		DeviceSpecs specs = (*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->getSpecs();
		return Py_BuildValue("i", specs.channels);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

PyDoc_STRVAR(M_aud_Device_volume_doc,
			 "The overall volume of the device.");

static PyObject *
Device_get_volume(Device *self, void* nothing)
{
	try
	{
		return Py_BuildValue("f", (*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->getVolume());
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Device_set_volume(Device *self, PyObject *args, void* nothing)
{
	float volume;

	if(!PyArg_Parse(args, "f:volume", &volume))
		return -1;

	try
	{
		(*reinterpret_cast<std::shared_ptr<IDevice>*>(self->device))->setVolume(volume);
		return 0;
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return -1;
	}
}

PyDoc_STRVAR(M_aud_Device_listener_location_doc,
			 "The listeners's location in 3D space, a 3D tuple of floats.");

static PyObject *
Device_get_listener_location(Device *self, void* nothing)
{
	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			Vector3 v = device->getListenerLocation();
			return Py_BuildValue("(fff)", v.x(), v.y(), v.z());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return nullptr;
}

static int
Device_set_listener_location(Device *self, PyObject *args, void* nothing)
{
	float x, y, z;

	if(!PyArg_Parse(args, "(fff):listener_location", &x, &y, &z))
		return -1;

	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			Vector3 location(x, y, z);
			device->setListenerLocation(location);
			return 0;
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Device_listener_velocity_doc,
			 "The listener's velocity in 3D space, a 3D tuple of floats.");

static PyObject *
Device_get_listener_velocity(Device *self, void* nothing)
{
	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			Vector3 v = device->getListenerVelocity();
			return Py_BuildValue("(fff)", v.x(), v.y(), v.z());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return nullptr;
}

static int
Device_set_listener_velocity(Device *self, PyObject *args, void* nothing)
{
	float x, y, z;

	if(!PyArg_Parse(args, "(fff):listener_velocity", &x, &y, &z))
		return -1;

	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			Vector3 velocity(x, y, z);
			device->setListenerVelocity(velocity);
			return 0;
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Device_listener_orientation_doc,
			 "The listener's orientation in 3D space as quaternion, a 4 float tuple.");

static PyObject *
Device_get_listener_orientation(Device *self, void* nothing)
{
	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			Quaternion o = device->getListenerOrientation();
			return Py_BuildValue("(ffff)", o.w(), o.x(), o.y(), o.z());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return nullptr;
}

static int
Device_set_listener_orientation(Device *self, PyObject *args, void* nothing)
{
	float w, x, y, z;

	if(!PyArg_Parse(args, "(ffff):listener_orientation", &w, &x, &y, &z))
		return -1;

	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			Quaternion orientation(w, x, y, z);
			device->setListenerOrientation(orientation);
			return 0;
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Device_speed_of_sound_doc,
			 "The speed of sound of the device.\n"
			 "The speed of sound in air is typically 343 m/s.");

static PyObject *
Device_get_speed_of_sound(Device *self, void* nothing)
{
	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			return Py_BuildValue("f", device->getSpeedOfSound());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Device_set_speed_of_sound(Device *self, PyObject *args, void* nothing)
{
	float speed;

	if(!PyArg_Parse(args, "f:speed_of_sound", &speed))
		return -1;

	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			device->setSpeedOfSound(speed);
			return 0;
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Device_doppler_factor_doc,
			 "The doppler factor of the device.\n"
			 "This factor is a scaling factor for the velocity vectors in "
			 "doppler calculation. So a value bigger than 1 will exaggerate "
			 "the effect as it raises the velocity.");

static PyObject *
Device_get_doppler_factor(Device *self, void* nothing)
{
	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			return Py_BuildValue("f", device->getDopplerFactor());
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Device_set_doppler_factor(Device *self, PyObject *args, void* nothing)
{
	float factor;

	if(!PyArg_Parse(args, "f:doppler_factor", &factor))
		return -1;

	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			device->setDopplerFactor(factor);
			return 0;
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

PyDoc_STRVAR(M_aud_Device_distance_model_doc,
			 "The distance model of the device.\n\n"
			 ".. seealso:: http://connect.creativelabs.com/openal/Documentation/OpenAL%201.1%20Specification.htm#_Toc199835864");

static PyObject *
Device_get_distance_model(Device *self, void* nothing)
{
	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			return Py_BuildValue("i", int(device->getDistanceModel()));
		}
		else
		{
			PyErr_SetString(AUDError, device_not_3d_error);
			return nullptr;
		}
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
		return nullptr;
	}
}

static int
Device_set_distance_model(Device *self, PyObject *args, void* nothing)
{
	int model;

	if(!PyArg_Parse(args, "i:distance_model", &model))
		return -1;

	try
	{
		I3DDevice* device = dynamic_cast<I3DDevice*>(reinterpret_cast<std::shared_ptr<IDevice>*>(self->device)->get());
		if(device)
		{
			device->setDistanceModel(DistanceModel(model));
			return 0;
		}
		else
			PyErr_SetString(AUDError, device_not_3d_error);
	}
	catch(Exception& e)
	{
		PyErr_SetString(AUDError, e.str);
	}

	return -1;
}

static PyGetSetDef Device_properties[] = {
	{(char*)"rate", (getter)Device_get_rate, nullptr,
	 M_aud_Device_rate_doc, nullptr },
	{(char*)"format", (getter)Device_get_format, nullptr,
	 M_aud_Device_format_doc, nullptr },
	{(char*)"channels", (getter)Device_get_channels, nullptr,
	 M_aud_Device_channels_doc, nullptr },
	{(char*)"volume", (getter)Device_get_volume, (setter)Device_set_volume,
	 M_aud_Device_volume_doc, nullptr },
	{(char*)"listener_location", (getter)Device_get_listener_location, (setter)Device_set_listener_location,
	 M_aud_Device_listener_location_doc, nullptr },
	{(char*)"listener_velocity", (getter)Device_get_listener_velocity, (setter)Device_set_listener_velocity,
	 M_aud_Device_listener_velocity_doc, nullptr },
	{(char*)"listener_orientation", (getter)Device_get_listener_orientation, (setter)Device_set_listener_orientation,
	 M_aud_Device_listener_orientation_doc, nullptr },
	{(char*)"speed_of_sound", (getter)Device_get_speed_of_sound, (setter)Device_set_speed_of_sound,
	 M_aud_Device_speed_of_sound_doc, nullptr },
	{(char*)"doppler_factor", (getter)Device_get_doppler_factor, (setter)Device_set_doppler_factor,
	 M_aud_Device_doppler_factor_doc, nullptr },
	{(char*)"distance_model", (getter)Device_get_distance_model, (setter)Device_set_distance_model,
	 M_aud_Device_distance_model_doc, nullptr },
	{nullptr}  /* Sentinel */
};

PyDoc_STRVAR(M_aud_Device_doc,
			 "Device objects represent an audio output backend like OpenAL or "
			 "SDL, but might also represent a file output or RAM buffer "
			 "output.");

static PyTypeObject DeviceType = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	"aud.Device",              /* tp_name */
	sizeof(Device),            /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)Device_dealloc,/* tp_dealloc */
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
	M_aud_Device_doc,          /* tp_doc */
	0,		                   /* tp_traverse */
	0,		                   /* tp_clear */
	0,		                   /* tp_richcompare */
	0,		                   /* tp_weaklistoffset */
	0,		                   /* tp_iter */
	0,		                   /* tp_iternext */
	Device_methods,            /* tp_methods */
	0,                         /* tp_members */
	Device_properties,         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	0,                         /* tp_init */
	0,                         /* tp_alloc */
	Device_new,                /* tp_new */
};

PyObject *
Device_empty()
{
	return DeviceType.tp_alloc(&DeviceType, 0);
}

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
PyInit_aud(void)
{
	PyObject *m;

	if(PyType_Ready(&SoundType) < 0)
		return nullptr;

	if(PyType_Ready(&DeviceType) < 0)
		return nullptr;

	if(PyType_Ready(&HandleType) < 0)
		return nullptr;

	m = PyModule_Create(&audmodule);
	if(m == nullptr)
		return nullptr;

	Py_INCREF(&SoundType);
	PyModule_AddObject(m, "Sound", (PyObject *)&SoundType);

	Py_INCREF(&DeviceType);
	PyModule_AddObject(m, "Device", (PyObject *)&DeviceType);

	Py_INCREF(&HandleType);
	PyModule_AddObject(m, "Handle", (PyObject *)&HandleType);

	AUDError = PyErr_NewException("aud.error", nullptr, nullptr);
	Py_INCREF(AUDError);
	PyModule_AddObject(m, "error", AUDError);

	// device constants
	PY_MODULE_ADD_CONSTANT(m, DEVICE_NULL);
	PY_MODULE_ADD_CONSTANT(m, DEVICE_OPENAL);
	PY_MODULE_ADD_CONSTANT(m, DEVICE_SDL);
	PY_MODULE_ADD_CONSTANT(m, DEVICE_JACK);
	//PY_MODULE_ADD_CONSTANT(m, DEVICE_READ);
	// format constants
	PY_MODULE_ADD_CONSTANT(m, FORMAT_FLOAT32);
	PY_MODULE_ADD_CONSTANT(m, FORMAT_FLOAT64);
	PY_MODULE_ADD_CONSTANT(m, FORMAT_INVALID);
	PY_MODULE_ADD_CONSTANT(m, FORMAT_S16);
	PY_MODULE_ADD_CONSTANT(m, FORMAT_S24);
	PY_MODULE_ADD_CONSTANT(m, FORMAT_S32);
	PY_MODULE_ADD_CONSTANT(m, FORMAT_U8);
	// status constants
	PY_MODULE_ADD_CONSTANT(m, STATUS_INVALID);
	PY_MODULE_ADD_CONSTANT(m, STATUS_PAUSED);
	PY_MODULE_ADD_CONSTANT(m, STATUS_PLAYING);
	PY_MODULE_ADD_CONSTANT(m, STATUS_STOPPED);
	// distance model constants
	PY_MODULE_ADD_CONSTANT(m, DISTANCE_MODEL_EXPONENT);
	PY_MODULE_ADD_CONSTANT(m, DISTANCE_MODEL_EXPONENT_CLAMPED);
	PY_MODULE_ADD_CONSTANT(m, DISTANCE_MODEL_INVERSE);
	PY_MODULE_ADD_CONSTANT(m, DISTANCE_MODEL_INVERSE_CLAMPED);
	PY_MODULE_ADD_CONSTANT(m, DISTANCE_MODEL_LINEAR);
	PY_MODULE_ADD_CONSTANT(m, DISTANCE_MODEL_LINEAR_CLAMPED);
	PY_MODULE_ADD_CONSTANT(m, DISTANCE_MODEL_INVALID);

	return m;
}

int main()
{
	return 0;
}
