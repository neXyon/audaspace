audaspace
=========

Audaspace (pronounced "outer space") is a high level audio library written in C++ with language bindings for Python for example. It started out as the audio engine of the 3D modelling application Blender. It's currently under heavy development to be released as a standalone library.

Features
--------

The following (probably incomplete) features are supported by audaspace:

* input/output devices
 * input from microphones, line in, etc.
 * output devices including 3D audio support
* file reading/writing
* filters like low-/highpass and effects like delay, reverse or fading
* generators for simple waveforms like silence, sine and triangle
* respecification - this term is used for changing stream parameters which are
 * channel count - channel remapping
 * sample format - the library internally uses 32 bit floats
 * sample rate - resampling
* simple (superposition, joining and ping-pong aka forward-reverse) and more complex (non-linear audio editing) sequencing of sounds

Library Concepts
----------------

The most important classes to understand the overall architecture of the library are the interfaces Device, Handle and Sound. A device is an output device, that usually outputs to speakers through a soundcard. A sound is merely a description of a playable sound, but not yet a realisation of a single playback event. Thus a sound can be played multiple times and in parallel. Actual playback instances are so called Handles which are returned by the play method of the device.

Additionally to these, two more interfaces might be important depending on the application case. The Reader represents an actual playback instance. The difference to the Handle here is, that the Handle is associated to a Device, while a Reader is not. Sounds actually produce Readers and a Handle internally always reads from a Reader. Writers exist too and are usually used for writing sound data to files. The main difference between a Device and a Writer is that a Device pulls the sound data by itself from multiple readers and has a quite complex structure, while a Writer is lightweight and needs to be fed with sample data.

The architecture of sample data reading is top-down and not bottom-up. That means that the Devices are supplying the data buffers for the Readers to read into. This saves memory and increases the speed, except for the case where a simple buffer is played back and thus has to be copied during playback.

The memory management of audaspace is as far as possible done with shared_ptr objects and the library follows the RAII principle wherever possible. Errors are dealt with using exceptions instead of error values.

Build Dependencies
------------------

Audaspace is written in C++ 11 so a fairly recent compiler (g++ 4.8.2, clang 3.3, MSVC 2013) is needed to build it. The following build dependencies are all optional, but without any it's neither possible to open sound files nor play back through the speakers.

* OpenAL (input/output device)
* SDL (output device)
* Jack (output device)
* libsndfile (file access)
* ffmpeg (file access)
* Python (language binding)

License
-------

> Copyright © 2013 Jörg Müller. All rights reserved.
>
> Licensed under the Apache License, Version 2.0 (the "License");
> you may not use this file except in compliance with the License.
> You may obtain a copy of the License at
>
>   http://www.apache.org/licenses/LICENSE-2.0
>
> Unless required by applicable law or agreed to in writing, software
> distributed under the License is distributed on an "AS IS" BASIS,
> WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
> See the License for the specific language governing permissions and
> limitations under the License.
