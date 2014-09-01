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

#include "file/FileWriter.h"
#include "plugin/PluginManager.h"
#include "generator/Sawtooth.h"
#include "generator/Silence.h"
#include "generator/Sine.h"
#include "generator/Square.h"
#include "generator/Triangle.h"
#include "fx/LimiterReader.h"
#include "Exception.h"
#include "IReader.h"

#include <string>
#include <iostream>

using namespace aud;

int main(int argc, char* argv[])
{
	PluginManager::loadPlugins(".");

	SampleRate sampleRate = RATE_48000;
	float frequency = 1.0f;
	float duration = 1.0f;

	Sawtooth sawtooth(frequency, sampleRate);
	Silence silence;
	Sine sine(frequency, sampleRate);
	Square square(frequency, sampleRate);
	Triangle triangle(frequency, sampleRate);

	struct { ISound* sound; std::string name; } generators[] = {
		{&sawtooth, "sawtooth"},
		{&silence, "silence"},
		{&sine, "sine"},
		{&square, "square"},
		{&triangle, "triangle"},
		{nullptr, ""}
	};

	std::shared_ptr<IReader> reader;

	DeviceSpecs specs;
	specs.channels = CHANNELS_MONO;
	specs.rate = sampleRate;
	specs.format = FORMAT_S16;

	for(int i = 0; generators[i].sound; i++)
	{
		reader = generators[i].sound->createReader();

		reader = std::shared_ptr<IReader>(new LimiterReader(reader, 0, duration));

		auto writer = FileWriter::createWriter(generators[i].name + ".wav", specs, CONTAINER_WAV, CODEC_PCM, 0);

		FileWriter::writeReader(reader, writer, 0, AUD_DEFAULT_BUFFER_SIZE);

		writer.reset();
	}

	return 0;
}
