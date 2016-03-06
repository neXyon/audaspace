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

#pragma once

/**
 * @file PluginManager.h
 * @ingroup plugin
 * The PluginManager class.
 */

#include "Audaspace.h"

#include <unordered_map>
#include <string>

AUD_NAMESPACE_BEGIN

/**
 * This manager provides utilities for plugin loading.
 */
class AUD_API PluginManager
{
private:
	static std::unordered_map<std::string, void*> m_plugins;

	// delete copy constructor and operator=
	PluginManager(const PluginManager&) = delete;
	PluginManager& operator=(const PluginManager&) = delete;
	PluginManager() = delete;

public:
	/**
	 * Loads a plugin from a file.
	 * @param path The path to the file.
	 * @return Whether the file could successfully be loaded.
	 */
	static bool loadPlugin(const std::string& path);

	/**
	 * Loads all plugins found in a folder.
	 * @param path The path to the folder containing the plugins.
	 */
	static void loadPlugins(const std::string& path = "");
};

AUD_NAMESPACE_END
