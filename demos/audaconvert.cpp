/*******************************************************************************
 * Copyright 2009-2016 Jörg Müller
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

#include "IReader.h"
#include "file/File.h"
#include "file/IWriter.h"
#include "file/FileWriter.h"
#include "plugin/PluginManager.h"
#include "respec/JOSResampleReader.h"
#include "Exception.h"

#include <string>
#include <iostream>

using namespace aud;

int main(int argc, char* argv[])
{
	if(argc != 6)
	{
		std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <container> <codec> <bitrate>" << std::endl;
		return 1;
	}

	std::string infile = argv[1];
	std::string outfile = argv[2];
	std::string sContainer = argv[3];
	std::string sCodec = argv[4];
	int bitrate;
	Codec codec = CODEC_INVALID;
	Container container = CONTAINER_INVALID;

	try
	{
		bitrate = std::stoi(argv[5]);
	}
	catch(std::exception&)
	{
		std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <container> <codec> <bitrate>" << std::endl;
		return 1;
	}

	if(sContainer == "ogg")
		container = CONTAINER_OGG;
	else if(sContainer == "mp3")
		container = CONTAINER_MP3;
	else if(sContainer == "wav")
		container = CONTAINER_WAV;

	if(sCodec == "opus")
		codec = CODEC_OPUS;
	else if(sCodec == "vorbis")
		codec = CODEC_VORBIS;
	else if(sCodec == "mp3")
		codec = CODEC_MP3;
	else if(sCodec == "pcm")
		codec = CODEC_PCM;

	if(container == CONTAINER_INVALID)
	{
		std::cerr << "Unknown container format: " << sContainer << std::endl;
		return 3;
	}

	if(codec == CODEC_INVALID)
	{
		std::cerr << "Unknown codec format: " << sCodec << std::endl;
		return 4;
	}

	PluginManager::loadPlugins("");

	File file(infile);
	std::shared_ptr<IReader> reader;

	try
	{
		reader = file.createReader();
	}
	catch(Exception& e)
	{
		std::cerr << "Error opening file " << argv[1] << " - " << e.getMessage() << std::endl;
		return 2;
	}

	DeviceSpecs specs;
	specs.specs = reader->getSpecs();
	specs.format = FORMAT_FLOAT32;

	auto writer = FileWriter::createWriter(outfile, specs, container, codec, bitrate);

	if(writer->getSpecs().rate != specs.rate)
	{
		specs.rate = writer->getSpecs().rate;
		reader = std::shared_ptr<IReader>(new JOSResampleReader(reader, specs.rate));
	}

	FileWriter::writeReader(reader, writer, 0, AUD_DEFAULT_BUFFER_SIZE);

	writer.reset();

	return 0;
}
