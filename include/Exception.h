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

#pragma once

/**
 * @file Exception.h
 * Defines the class as well as the AUD_THROW macro for easy throwing.
 */

#include "Audaspace.h"

#include <exception>
#include <string>

/// Throws a Exception with the provided error code.
#define AUD_THROW(exception, message) { throw exception(message, __FILE__, __LINE__); }

AUD_NAMESPACE_BEGIN

/// Exception structure.
class Exception : public std::exception
{
protected:
	const std::string m_message;
	const std::string m_file;
	const int m_line;
	Exception(std::string message, std::string file, int line);
public:
	virtual const char* what() const noexcept;
	virtual std::string getDebugMessage() const;

	const std::string& getMessage() const;
	const std::string& getFile() const;
	int getLine() const;
};

class FileException : public Exception
{
public:
	FileException(std::string message, std::string file, int line) :
		Exception(message, file, line) {}
};

class DeviceException : public Exception
{
public:
	DeviceException(std::string message, std::string file, int line) :
		Exception(message, file, line) {}
};

class StateException : public Exception
{
public:
	StateException(std::string message, std::string file, int line) :
		Exception(message, file, line) {}
};

AUD_NAMESPACE_END
