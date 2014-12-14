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

#include "plugin/PluginManager.h"

#include <windows.h>

AUD_NAMESPACE_BEGIN

std::unordered_map<std::string, void*> PluginManager::m_plugins;

bool PluginManager::loadPlugin(const std::string& path)
{
	HMODULE handle = LoadLibrary(path.c_str());

	if (!handle)
		return false;

	void(*registerPlugin)() = (void(*)())GetProcAddress(handle, "registerPlugin");
	const char* (*getName)() = (const char* (*)())GetProcAddress(handle, "getName");

	if(!registerPlugin || !getName)
	{
		FreeLibrary(handle);
		return false;
	}

	registerPlugin();

	m_plugins[getName()] = handle;

	return true;
}

void PluginManager::loadPlugins(const std::string& path)
{
	std::string readpath = path;

	if(path == "")
		readpath = "@DEFAULT_PLUGIN_PATH@";

	WIN32_FIND_DATA entry;
	bool found_file = true;
	std::string search = readpath + "\\*";
	HANDLE dir = FindFirstFile(search.c_str(), &entry);

	if(dir == INVALID_HANDLE_VALUE)
		return;

	while(found_file)
	{
		const std::string filename = entry.cFileName;
		const std::string end = ".dll";

		if(filename.length() >= end.length() && filename.substr(filename.length() - end.length()) == end)
		{
			loadPlugin(readpath + "/" + filename);
		}

		found_file = FindNextFile(dir, &entry);
	}

	FindClose(dir);
}

AUD_NAMESPACE_END
