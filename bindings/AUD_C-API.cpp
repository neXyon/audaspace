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

#ifdef WITH_PYTHON
#  include "PyInit.h"
#  include "PyAPI.h"
#endif

#include <set>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <sstream>

#include "NULLDevice.h"
#include "I3DDevice.h"
#include "I3DHandle.h"
#include "File.h"
#include "FileWriter.h"
#include "StreamBuffer.h"
#include "Delay.h"
#include "Limiter.h"
#include "PingPong.h"
#include "Loop.h"
#include "Rectify.h"
#include "Envelope.h"
#include "LinearResample.h"
#include "Lowpass.h"
#include "Highpass.h"
#include "Accumulator.h"
#include "Sum.h"
#include "Square.h"
#include "ChannelMapper.h"
#include "Buffer.h"
#include "ReadDevice.h"
#include "IReader.h"
#include "Sequence.h"
#include "SequenceEntry.h"
#include "Silence.h"
#include "MutexLock.h"

#ifdef WITH_SDL
#include "SDLDevice.h"
#endif

#ifdef WITH_OPENAL
#include "OpenALDevice.h"
#endif

#ifdef WITH_JACK
#include "JackDevice.h"
#include "JackLibrary.h"
#endif


#ifdef WITH_FFMPEG
extern "C" {
#include <libavformat/avformat.h>
}
#endif

#include <cassert>

using namespace aud;

typedef std::shared_ptr<ISound> AUD_Sound;
typedef std::shared_ptr<ReadDevice> AUD_Device;
typedef std::shared_ptr<IHandle> AUD_Handle;
typedef std::shared_ptr<SequenceEntry> AUD_SEntry;

#define AUD_CAPI_IMPLEMENTATION
#include "AUD_C-API.h"

#ifndef NULL
#  define NULL (void *)0
#endif

static std::shared_ptr<IDevice> AUD_device;
static I3DDevice *AUD_3ddevice;

void AUD_initOnce()
{
#ifdef WITH_FFMPEG
	av_register_all();
#endif
#ifdef WITH_JACK
	aud_jack_init();
#endif
}

void AUD_exitOnce()
{
#ifdef WITH_JACK
	aud_jack_exit();
#endif
}

int AUD_init(AUD_DeviceType device, DeviceSpecs specs, int buffersize)
{
	std::shared_ptr<IDevice> dev;

	if (AUD_device.get()) {
		AUD_exit();
	}

	try {
		switch(device) {
		case AUD_NULL_DEVICE:
			dev = std::shared_ptr<IDevice>(new NULLDevice());
			break;
#ifdef WITH_SDL
		case AUD_SDL_DEVICE:
			dev = std::shared_ptr<IDevice>(new SDLDevice(specs, buffersize));
			break;
#endif
#ifdef WITH_OPENAL
		case AUD_OPENAL_DEVICE:
			dev = std::shared_ptr<IDevice>(new OpenALDevice(specs, buffersize));
			break;
#endif
#ifdef WITH_JACK
		case AUD_JACK_DEVICE:
#ifdef __APPLE__
			struct stat st;
			if (stat("/Library/Frameworks/Jackmp.framework", &st) != 0) {
				printf("Warning: Jack Framework not installed\n");
				// No break, fall through to default, to return false
			}
			else
#endif
			if (!aud_jack_supported()) {
				printf("Warning: Jack cllient not installed\n");
				// No break, fall through to default, to return false
			}
			else {
				dev = std::shared_ptr<IDevice>(new JackDevice("Blender", specs, buffersize));
				break;
			}
#endif
		default:
			return false;
		}

		AUD_device = dev;
		AUD_3ddevice = dynamic_cast<I3DDevice *>(AUD_device.get());

		return true;
	}
	catch(Exception&)
	{
		return false;
	}
}

void AUD_exit()
{
	AUD_device = std::shared_ptr<IDevice>();
	AUD_3ddevice = NULL;
}

#ifdef WITH_PYTHON
static PyObject *AUD_getCDevice(PyObject *self)
{
	if (AUD_device.get()) {
		Device *device = (Device *)Device_empty();
		if (device != NULL) {
			device->device = new std::shared_ptr<IDevice>(AUD_device);
			return (PyObject *)device;
		}
	}

	Py_RETURN_NONE;
}

static PyMethodDef meth_getcdevice[] = {
    {"device", (PyCFunction)AUD_getCDevice, METH_NOARGS,
     "device()\n\n"
     "Returns the application's :class:`Device`.\n\n"
     ":return: The application's :class:`Device`.\n"
     ":rtype: :class:`Device`"}
};

extern "C" {
extern void *sound_get_factory(void *sound);
}

static PyObject *AUD_getSoundFromPointer(PyObject *self, PyObject *args)
{
	long int lptr;

	if (PyArg_Parse(args, "l:_sound_from_pointer", &lptr)) {
		if (lptr) {
			std::shared_ptr<ISound>* factory = (std::shared_ptr<ISound>*) sound_get_factory((void *) lptr);

			if (factory) {
				Factory *obj = (Factory *)Factory_empty();
				if (obj) {
					obj->factory = new std::shared_ptr<ISound>(*factory);
					return (PyObject *) obj;
				}
			}
		}
	}

	Py_RETURN_NONE;
}

static PyMethodDef meth_sound_from_pointer[] = {
    {"_sound_from_pointer", (PyCFunction)AUD_getSoundFromPointer, METH_O,
     "_sound_from_pointer(pointer)\n\n"
     "Returns the corresponding :class:`Factory` object.\n\n"
     ":arg pointer: The pointer to the bSound object as long.\n"
     ":type pointer: long\n"
     ":return: The corresponding :class:`Factory` object.\n"
     ":rtype: :class:`Factory`"}
};

PyObject *AUD_initPython()
{
	PyObject *module = PyInit_aud();
	PyModule_AddObject(module, "device", (PyObject *)PyCFunction_New(meth_getcdevice, NULL));
	PyModule_AddObject(module, "_sound_from_pointer", (PyObject *)PyCFunction_New(meth_sound_from_pointer, NULL));
	PyDict_SetItemString(PyImport_GetModuleDict(), "aud", module);

	return module;
}

void *AUD_getPythonFactory(AUD_Sound *sound)
{
	if (sound) {
		Factory *obj = (Factory *) Factory_empty();
		if (obj) {
			obj->factory = new std::shared_ptr<ISound>(*sound);
			return (PyObject *) obj;
		}
	}

	return NULL;
}

AUD_Sound *AUD_getPythonSound(void *sound)
{
	Factory *factory = checkFactory((PyObject *)sound);

	if (!factory)
		return NULL;

	return new std::shared_ptr<ISound>(*reinterpret_cast<std::shared_ptr<ISound>*>(factory->factory));
}

#endif

void AUD_lock()
{
	AUD_device->lock();
}

void AUD_unlock()
{
	AUD_device->unlock();
}

AUD_SoundInfo AUD_getInfo(AUD_Sound *sound)
{
	assert(sound);

	AUD_SoundInfo info;
	info.specs.channels = CHANNELS_INVALID;
	info.specs.rate = RATE_INVALID;
	info.length = 0.0f;

	try {
		std::shared_ptr<IReader> reader = (*sound)->createReader();

		if (reader.get()) {
			info.specs = reader->getSpecs();
			info.length = reader->getLength() / (float) info.specs.rate;
		}
	}
	catch(Exception&)
	{
	}

	return info;
}

AUD_Sound *AUD_load(const char *filename)
{
	assert(filename);
	return new AUD_Sound(new File(filename));
}

AUD_Sound *AUD_loadBuffer(unsigned char *buffer, int size)
{
	assert(buffer);
	return new AUD_Sound(new File(buffer, size));
}

AUD_Sound *AUD_bufferSound(AUD_Sound *sound)
{
	assert(sound);

	try {
		return new AUD_Sound(new StreamBuffer(*sound));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_monoSound(AUD_Sound *sound)
{
	assert(sound);

	try {
		DeviceSpecs specs;
		specs.channels = CHANNELS_MONO;
		specs.rate = RATE_INVALID;
		specs.format = FORMAT_INVALID;
		return new AUD_Sound(new ChannelMapper(*sound, specs));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_delaySound(AUD_Sound *sound, float delay)
{
	assert(sound);

	try {
		return new AUD_Sound(new Delay(*sound, delay));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_limitSound(AUD_Sound *sound, float start, float end)
{
	assert(sound);

	try {
		return new AUD_Sound(new Limiter(*sound, start, end));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_pingpongSound(AUD_Sound *sound)
{
	assert(sound);

	try {
		return new AUD_Sound(new PingPong(*sound));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Sound *AUD_loopSound(AUD_Sound *sound)
{
	assert(sound);

	try {
		return new AUD_Sound(new Loop(*sound));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

int AUD_setLoop(AUD_Handle *handle, int loops)
{
	assert(handle);

	try {
		return (*handle)->setLoopCount(loops);
	}
	catch(Exception&)
	{
	}

	return false;
}

AUD_Sound *AUD_rectifySound(AUD_Sound *sound)
{
	assert(sound);

	try {
		return new AUD_Sound(new Rectify(*sound));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

void AUD_unload(AUD_Sound *sound)
{
	assert(sound);
	delete sound;
}

AUD_Handle *AUD_play(AUD_Sound *sound, int keep)
{
	assert(sound);
	try {
		AUD_Handle handle = AUD_device->play(*sound, keep);
		if (handle.get()) {
			return new AUD_Handle(handle);
		}
	}
	catch(Exception&)
	{
	}
	return NULL;
}

int AUD_pause(AUD_Handle *handle)
{
	assert(handle);
	return (*handle)->pause();
}

int AUD_resume(AUD_Handle *handle)
{
	assert(handle);
	return (*handle)->resume();
}

int AUD_stop(AUD_Handle *handle)
{
	assert(handle);
	int result = (*handle)->stop();
	delete handle;
	return result;
}

int AUD_setKeep(AUD_Handle *handle, int keep)
{
	assert(handle);
	return (*handle)->setKeep(keep);
}

int AUD_seek(AUD_Handle *handle, float seekTo)
{
	assert(handle);
	return (*handle)->seek(seekTo);
}

float AUD_getPosition(AUD_Handle *handle)
{
	assert(handle);
	return (*handle)->getPosition();
}

Status AUD_getStatus(AUD_Handle *handle)
{
	assert(handle);
	return (*handle)->getStatus();
}

int AUD_setListenerLocation(const float location[3])
{
	if (AUD_3ddevice) {
		Vector3 v(location[0], location[1], location[2]);
		AUD_3ddevice->setListenerLocation(v);
		return true;
	}

	return false;
}

int AUD_setListenerVelocity(const float velocity[3])
{
	if (AUD_3ddevice) {
		Vector3 v(velocity[0], velocity[1], velocity[2]);
		AUD_3ddevice->setListenerVelocity(v);
		return true;
	}

	return false;
}

int AUD_setListenerOrientation(const float orientation[4])
{
	if (AUD_3ddevice) {
		Quaternion q(orientation[3], orientation[0], orientation[1], orientation[2]);
		AUD_3ddevice->setListenerOrientation(q);
		return true;
	}

	return false;
}

int AUD_setSpeedOfSound(float speed)
{
	if (AUD_3ddevice) {
		AUD_3ddevice->setSpeedOfSound(speed);
		return true;
	}

	return false;
}

int AUD_setDopplerFactor(float factor)
{
	if (AUD_3ddevice) {
		AUD_3ddevice->setDopplerFactor(factor);
		return true;
	}

	return false;
}

int AUD_setDistanceModel(DistanceModel model)
{
	if (AUD_3ddevice) {
		AUD_3ddevice->setDistanceModel(model);
		return true;
	}

	return false;
}

int AUD_setSourceLocation(AUD_Handle *handle, const float location[3])
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		Vector3 v(location[0], location[1], location[2]);
		return h->setSourceLocation(v);
	}

	return false;
}

int AUD_setSourceVelocity(AUD_Handle *handle, const float velocity[3])
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		Vector3 v(velocity[0], velocity[1], velocity[2]);
		return h->setSourceVelocity(v);
	}

	return false;
}

int AUD_setSourceOrientation(AUD_Handle *handle, const float orientation[4])
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		Quaternion q(orientation[3], orientation[0], orientation[1], orientation[2]);
		return h->setSourceOrientation(q);
	}

	return false;
}

int AUD_setRelative(AUD_Handle *handle, int relative)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setRelative(relative);
	}

	return false;
}

int AUD_setVolumeMaximum(AUD_Handle *handle, float volume)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setVolumeMaximum(volume);
	}

	return false;
}

int AUD_setVolumeMinimum(AUD_Handle *handle, float volume)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setVolumeMinimum(volume);
	}

	return false;
}

int AUD_setDistanceMaximum(AUD_Handle *handle, float distance)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setDistanceMaximum(distance);
	}

	return false;
}

int AUD_setDistanceReference(AUD_Handle *handle, float distance)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setDistanceReference(distance);
	}

	return false;
}

int AUD_setAttenuation(AUD_Handle *handle, float factor)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setAttenuation(factor);
	}

	return false;
}

int AUD_setConeAngleOuter(AUD_Handle *handle, float angle)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setConeAngleOuter(angle);
	}

	return false;
}

int AUD_setConeAngleInner(AUD_Handle *handle, float angle)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setConeAngleInner(angle);
	}

	return false;
}

int AUD_setConeVolumeOuter(AUD_Handle *handle, float volume)
{
	assert(handle);
	std::shared_ptr<I3DHandle> h = std::dynamic_pointer_cast<I3DHandle>(*handle);

	if (h.get()) {
		return h->setConeVolumeOuter(volume);
	}

	return false;
}

int AUD_setSoundVolume(AUD_Handle *handle, float volume)
{
	assert(handle);
	try {
		return (*handle)->setVolume(volume);
	}
	catch(Exception&) {}
	return false;
}

int AUD_setSoundPitch(AUD_Handle *handle, float pitch)
{
	assert(handle);
	try {
		return (*handle)->setPitch(pitch);
	}
	catch(Exception&) {}
	return false;
}

AUD_Device *AUD_openReadDevice(DeviceSpecs specs)
{
	try {
		return new AUD_Device(new ReadDevice(specs));
	}
	catch(Exception&)
	{
		return NULL;
	}
}

AUD_Handle *AUD_playDevice(AUD_Device *device, AUD_Sound *sound, float seek)
{
	assert(device);
	assert(sound);

	try {
		AUD_Handle handle = (*device)->play(*sound);
		if (handle.get()) {
			handle->seek(seek);
			return new AUD_Handle(handle);
		}
	}
	catch(Exception&)
	{
	}
	return NULL;
}

int AUD_setDeviceVolume(AUD_Device *device, float volume)
{
	assert(device);

	try {
		(*device)->setVolume(volume);
		return true;
	}
	catch(Exception&) {}

	return false;
}

int AUD_readDevice(AUD_Device *device, data_t *buffer, int length)
{
	assert(device);
	assert(buffer);

	try {
		return (*device)->read(buffer, length);
	}
	catch(Exception&)
	{
		return false;
	}
}

void AUD_closeReadDevice(AUD_Device *device)
{
	assert(device);

	try {
		delete device;
	}
	catch(Exception&)
	{
	}
}

float *AUD_readSoundBuffer(const char *filename, float low, float high,
                           float attack, float release, float threshold,
                           int accumulate, int additive, int square,
                           float sthreshold, double samplerate, int *length)
{
	Buffer buffer;
	DeviceSpecs specs;
	specs.channels = CHANNELS_MONO;
	specs.rate = (SampleRate)samplerate;
	std::shared_ptr<ISound> sound;

	std::shared_ptr<ISound> file = std::shared_ptr<ISound>(new File(filename));

	int position = 0;

	try {
		std::shared_ptr<IReader> reader = file->createReader();

		SampleRate rate = reader->getSpecs().rate;

		sound = std::shared_ptr<ISound>(new ChannelMapper(file, specs));

		if (high < rate)
			sound = std::shared_ptr<ISound>(new Lowpass(sound, high));
		if (low > 0)
			sound = std::shared_ptr<ISound>(new Highpass(sound, low));

		sound = std::shared_ptr<ISound>(new Envelope(sound, attack, release, threshold, 0.1f));
		sound = std::shared_ptr<ISound>(new LinearResample(sound, specs));

		if (square)
			sound = std::shared_ptr<ISound>(new Square(sound, sthreshold));

		if (accumulate)
			sound = std::shared_ptr<ISound>(new Accumulator(sound, additive));
		else if (additive)
			sound = std::shared_ptr<ISound>(new Sum(sound));

		reader = sound->createReader();

		if (!reader.get())
			return NULL;

		int len;
		bool eos;
		do
		{
			len = samplerate;
			buffer.resize((position + len) * sizeof(float), true);
			reader->read(len, eos, buffer.getBuffer() + position);
			position += len;
		} while(!eos);
	}
	catch(Exception&)
	{
		return NULL;
	}

	float * result = (float *)malloc(position * sizeof(float));
	memcpy(result, buffer.getBuffer(), position * sizeof(float));
	*length = position;
	return result;
}

static void pauseSound(AUD_Handle *handle)
{
	assert(handle);
	(*handle)->pause();
}

AUD_Handle *AUD_pauseAfter(AUD_Handle *handle, float seconds)
{
	std::shared_ptr<ISound> silence = std::shared_ptr<ISound>(new Silence);
	std::shared_ptr<ISound> limiter = std::shared_ptr<ISound>(new Limiter(silence, 0, seconds));

	MutexLock lock(*AUD_device);

	try {
		AUD_Handle handle2 = AUD_device->play(limiter);
		if (handle2.get()) {
			handle2->setStopCallback((stopCallback)pauseSound, handle);
			return new AUD_Handle(handle2);
		}
	}
	catch(Exception&)
	{
	}

	return NULL;
}

AUD_Sound *AUD_createSequencer(float fps, int muted)
{
	// specs are changed at a later point!
	Specs specs;
	specs.channels = CHANNELS_STEREO;
	specs.rate = RATE_44100;
	AUD_Sound *sequencer = new AUD_Sound(std::shared_ptr<Sequence>(new Sequence(specs, fps, muted)));
	return sequencer;
}

void AUD_destroySequencer(AUD_Sound *sequencer)
{
	delete sequencer;
}

void AUD_setSequencerMuted(AUD_Sound *sequencer, int muted)
{
	dynamic_cast<Sequence *>(sequencer->get())->mute(muted);
}

void AUD_setSequencerFPS(AUD_Sound *sequencer, float fps)
{
	dynamic_cast<Sequence *>(sequencer->get())->setFPS(fps);
}

AUD_SEntry *AUD_addSequence(AUD_Sound *sequencer, AUD_Sound *sound,
                            float begin, float end, float skip)
{
	if (!sound)
		return new AUD_SEntry(((Sequence *)sequencer->get())->add(AUD_Sound(), begin, end, skip));
	return new AUD_SEntry(((Sequence *)sequencer->get())->add(*sound, begin, end, skip));
}

void AUD_removeSequence(AUD_Sound *sequencer, AUD_SEntry *entry)
{
	dynamic_cast<Sequence *>(sequencer->get())->remove(*entry);
	delete entry;
}

void AUD_moveSequence(AUD_SEntry *entry, float begin, float end, float skip)
{
	(*entry)->move(begin, end, skip);
}

void AUD_muteSequence(AUD_SEntry *entry, char mute)
{
	(*entry)->mute(mute);
}

void AUD_setRelativeSequence(AUD_SEntry *entry, char relative)
{
	(*entry)->setRelative(relative);
}

void AUD_updateSequenceSound(AUD_SEntry *entry, AUD_Sound *sound)
{
	if (sound)
		(*entry)->setSound(*sound);
	else
		(*entry)->setSound(AUD_Sound());
}

void AUD_setSequenceAnimData(AUD_SEntry *entry, AnimateablePropertyType type, int frame, float *data, char animated)
{
	AnimateableProperty *prop = (*entry)->getAnimProperty(type);
	if (animated) {
		if (frame >= 0)
			prop->write(data, frame, 1);
	}
	else {
		prop->write(data);
	}
}

void AUD_setSequencerAnimData(AUD_Sound *sequencer, AnimateablePropertyType type, int frame, float *data, char animated)
{
	AnimateableProperty *prop = dynamic_cast<Sequence *>(sequencer->get())->getAnimProperty(type);
	if (animated) {
		if (frame >= 0) {
			prop->write(data, frame, 1);
		}
	}
	else {
		prop->write(data);
	}
}

void AUD_updateSequenceData(AUD_SEntry *entry, float volume_max, float volume_min,
                            float distance_max, float distance_reference, float attenuation,
                            float cone_angle_outer, float cone_angle_inner, float cone_volume_outer)
{
	(*entry)->updateAll(volume_max, volume_min, distance_max, distance_reference, attenuation,
	                    cone_angle_outer, cone_angle_inner, cone_volume_outer);
}

void AUD_updateSequencerData(AUD_Sound *sequencer, float speed_of_sound,
                             float factor, DistanceModel model)
{
	Sequence *f = dynamic_cast<Sequence *>(sequencer->get());
	f->setSpeedOfSound(speed_of_sound);
	f->setDopplerFactor(factor);
	f->setDistanceModel(model);
}

void AUD_setSequencerDeviceSpecs(AUD_Sound *sequencer)
{
	dynamic_cast<Sequence *>(sequencer->get())->setSpecs(AUD_device->getSpecs().specs);
}

void AUD_setSequencerSpecs(AUD_Sound *sequencer, Specs specs)
{
	dynamic_cast<Sequence *>(sequencer->get())->setSpecs(specs);
}

void AUD_seekSequencer(AUD_Handle *handle, float time)
{
#ifdef WITH_JACK
	JackDevice *device = dynamic_cast<JackDevice *>(AUD_device.get());
	if (device) {
		device->seekPlayback(time);
	}
	else
#endif
	{
		assert(handle);
		(*handle)->seek(time);
	}
}

float AUD_getSequencerPosition(AUD_Handle *handle)
{
#ifdef WITH_JACK
	JackDevice *device = dynamic_cast<JackDevice *>(AUD_device.get());
	if (device) {
		return device->getPlaybackPosition();
	}
	else
#endif
	{
		assert(handle);
		return (*handle)->getPosition();
	}
}

void AUD_startPlayback()
{
#ifdef WITH_JACK
	JackDevice *device = dynamic_cast<JackDevice *>(AUD_device.get());
	if (device) {
		device->startPlayback();
	}
#endif
}

void AUD_stopPlayback()
{
#ifdef WITH_JACK
	JackDevice *device = dynamic_cast<JackDevice *>(AUD_device.get());
	if (device) {
		device->stopPlayback();
	}
#endif
}

#ifdef WITH_JACK
void AUD_setSyncCallback(syncFunction function, void *data)
{
	JackDevice *device = dynamic_cast<JackDevice *>(AUD_device.get());
	if (device) {
		device->setSyncCallback(function, data);
	}
}
#endif

int AUD_doesPlayback()
{
#ifdef WITH_JACK
	JackDevice *device = dynamic_cast<JackDevice *>(AUD_device.get());
	if (device) {
		return device->doesPlayback();
	}
#endif
	return -1;
}

int AUD_readSound(AUD_Sound *sound, sample_t *buffer, int length, int samples_per_second)
{
	DeviceSpecs specs;
	sample_t *buf;
	Buffer aBuffer;

	specs.rate = RATE_INVALID;
	specs.channels = CHANNELS_MONO;
	specs.format = FORMAT_INVALID;

	std::shared_ptr<IReader> reader = ChannelMapper(*sound, specs).createReader();

	specs.specs = reader->getSpecs();
	int len;
	float samplejump = specs.rate / samples_per_second;
	float min, max, power, overallmax;
	bool eos;

	overallmax = 0;

	for (int i = 0; i < length; i++) {
		len = floor(samplejump * (i+1)) - floor(samplejump * i);

		aBuffer.assureSize(len * AUD_SAMPLE_SIZE(specs));
		buf = aBuffer.getBuffer();

		reader->read(len, eos, buf);

		max = min = *buf;
		power = *buf * *buf;
		for (int j = 1; j < len; j++) {
			if (buf[j] < min)
				min = buf[j];
			if (buf[j] > max)
				max = buf[j];
			power += buf[j] * buf[j];
		}

		buffer[i * 3] = min;
		buffer[i * 3 + 1] = max;
		buffer[i * 3 + 2] = sqrt(power) / len;

		if (overallmax < max)
			overallmax = max;
		if (overallmax < -min)
			overallmax = -min;

		if (eos) {
			length = i;
			break;
		}
	}

	if (overallmax > 1.0f) {
		for (int i = 0; i < length * 3; i++) {
			buffer[i] /= overallmax;
		}
	}

	return length;
}

AUD_Sound *AUD_copy(AUD_Sound *sound)
{
	return new std::shared_ptr<ISound>(*sound);
}

void AUD_freeHandle(AUD_Handle *handle)
{
	delete handle;
}

void *AUD_createSet()
{
	return new std::set<void *>();
}

void AUD_destroySet(void *set)
{
	delete reinterpret_cast<std::set<void *>*>(set);
}

char AUD_removeSet(void *set, void *entry)
{
	if (set)
		return reinterpret_cast<std::set<void *>*>(set)->erase(entry);
	return 0;
}

void AUD_addSet(void *set, void *entry)
{
	if (entry)
		reinterpret_cast<std::set<void *>*>(set)->insert(entry);
}

void *AUD_getSet(void *set)
{
	if (set) {
		std::set<void *>* rset = reinterpret_cast<std::set<void *>*>(set);
		if (!rset->empty()) {
			std::set<void *>::iterator it = rset->begin();
			void *result = *it;
			rset->erase(it);
			return result;
		}
	}

	return NULL;
}

const char *AUD_mixdown(AUD_Sound *sound, unsigned int start, unsigned int length, unsigned int buffersize, const char *filename, DeviceSpecs specs, Container format, Codec codec, unsigned int bitrate)
{
	try {
		Sequence *f = dynamic_cast<Sequence *>(sound->get());

		f->setSpecs(specs.specs);
		std::shared_ptr<IReader> reader = f->createQualityReader();
		reader->seek(start);
		std::shared_ptr<IWriter> writer = FileWriter::createWriter(filename, specs, format, codec, bitrate);
		FileWriter::writeReader(reader, writer, length, buffersize);

		return NULL;
	}
	catch(Exception& e)
	{
		return e.str;
	}
}

const char *AUD_mixdown_per_channel(AUD_Sound *sound, unsigned int start, unsigned int length, unsigned int buffersize, const char *filename, DeviceSpecs specs, Container format, Codec codec, unsigned int bitrate)
{
	try {
		Sequence *f = dynamic_cast<Sequence *>(sound->get());

		f->setSpecs(specs.specs);

		std::vector<std::shared_ptr<IWriter> > writers;

		int channels = specs.channels;
		specs.channels = CHANNELS_MONO;

		for (int i = 0; i < channels; i++) {
			std::stringstream stream;
			std::string fn = filename;
			size_t index = fn.find_last_of('.');
			size_t index_slash = fn.find_last_of('/');
			size_t index_backslash = fn.find_last_of('\\');

			if ((index == std::string::npos) ||
			    ((index < index_slash) && (index_slash != std::string::npos)) ||
			    ((index < index_backslash) && (index_backslash != std::string::npos)))
			{
				stream << filename << "_" << (i + 1);
			}
			else {
				stream << fn.substr(0, index) << "_" << (i + 1) << fn.substr(index);
			}
			writers.push_back(FileWriter::createWriter(stream.str(), specs, format, codec, bitrate));
		}

		std::shared_ptr<IReader> reader = f->createQualityReader();
		reader->seek(start);
		FileWriter::writeReader(reader, writers, length, buffersize);

		return NULL;
	}
	catch(Exception& e)
	{
		return e.str;
	}
}

AUD_Device *AUD_openMixdownDevice(DeviceSpecs specs, AUD_Sound *sequencer, float volume, float start)
{
	try {
		ReadDevice *device = new ReadDevice(specs);
		device->setQuality(true);
		device->setVolume(volume);

		dynamic_cast<Sequence *>(sequencer->get())->setSpecs(specs.specs);

		AUD_Handle handle = device->play(*sequencer);
		if (handle.get()) {
			handle->seek(start);
		}

		return new AUD_Device(device);
	}
	catch(Exception&)
	{
		return NULL;
	}
}

std::shared_ptr<IDevice> AUD_getDevice()
{
	return AUD_device;
}

I3DDevice *AUD_get3DDevice()
{
	return AUD_3ddevice;
}

int AUD_isJackSupported(void)
{
#ifdef WITH_JACK
	return aud_jack_supported();
#else
	return 0;
#endif
}
