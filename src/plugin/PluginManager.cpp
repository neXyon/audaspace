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

#include <dlfcn.h>
#include <dirent.h>

AUD_NAMESPACE_BEGIN

std::unordered_map<std::string, void*> PluginManager::m_plugins;

bool PluginManager::loadPlugin(const std::string& path)
{
	void* handle = dlopen(path.c_str(), RTLD_LAZY);

	if (!handle)
		return false;

	void (*registerPlugin)() = (void (*)())dlsym(handle, "registerPlugin");
	const char* (*getName)() = (const char* (*)())dlsym(handle, "getName");

	if(!registerPlugin || !getName)
	{
		dlclose(handle);
		return false;
	}

	registerPlugin();

	m_plugins[getName()] = handle;

	return true;
}

void PluginManager::loadPlugins(const std::string &path)
{
	// AUD_XXX TODO: register linked plugins!

	DIR* dir = opendir(path.c_str());

	if(!dir)
		return;

	while(dirent* entry = readdir(dir))
	{
		const std::string filename = entry->d_name;
		const std::string end = ".so";
		const std::string start = "lib";

		if(filename.length() >= end.length() + start.length() && filename.substr(0, start.length()) == start && filename.substr(filename.length() - end.length()) == end)
		{
			loadPlugin(path + "/" + filename);
		}
	}

	closedir(dir);
}

AUD_NAMESPACE_END
