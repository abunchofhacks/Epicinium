/**
 * Part of Epicinium
 * developed by A Bunch of Hacks.
 *
 * Copyright (c) 2017-2020 A Bunch of Hacks
 *
 * Epicinium is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epicinium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * [authors:]
 * Sander in 't Veld (sander@abunchofhacks.coop)
 * Daan Mulder (daan@abunchofhacks.coop)
 */
#pragma once
#include "header.hpp"

/* This file can be included into every .cpp source file. */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <csignal>
#include <algorithm>

#if INCLUDE_IMGUI_ENABLED
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_stdlib.h"
#endif

#include "libs/jsoncpp/json.h"

#define PLOG_FORCE_UTF8_MESSAGE 1
#include "libs/plog/Log.h"

#if INTL_ENABLED
#include "language.hpp"
#define _(STRING) Language::gettext(STRING)
#else
#define _(STRING) (STRING)
#endif

// This is a separate macro to prevent xgettext from detecting it,
// and to make it more obvious that the translation only works if the actual
// translation source is defined elsewhere in this codebase.
#define GETTEXT_FROM_SERVER(STRING) _(STRING)

/* If we are running dev, we want to be able to call profiling macros. */
#if VALGRIND_INTEGRATION_ENABLED
#include <valgrind/memcheck.h>
#include <valgrind/callgrind.h>
#include <valgrind/helgrind.h>
#include <valgrind/drd.h>
#endif

/* If we are running dev, we want as many crashes as possible. */
// Deprecated by DEBUG_ASSERT() and RETHROW_IF_DEV() in header.hpp.
#ifdef DEVELOPMENT
#define CRASHME
#endif

/* If we are running dev, we want to be able to use PERFLOG. */
#ifdef DEVELOPMENT
#define PERFLOG
#endif

/* If we are running PERFLOG, we want to be able to use it. */
#ifdef PERFLOG
#define PERFLOG_INSTANCE 123
#define PERFLOGI LOGI_(PERFLOG_INSTANCE)
#define PERFLOGV LOGV_(PERFLOG_INSTANCE)
#else
#define PERFLOGI LOGI_IF(false)
#define PERFLOGV LOGV_IF(false)
#endif


/* Portable replacement for PI */
constexpr long double pi()
{
	return 3.141592653589793238462643383279502884L; /* GNU PI */
}

/* Portable replacement for SQRT(2) */
constexpr long double sqrt2()
{
	return 1.414213562373095048801688724209698079L; /* GNU SQRT2 */
}

/* Mathematical signum */
constexpr int signum(int x)
{
	return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}

/* Mathematical modulo */
constexpr int mod(int x, int n)
{
	return (x % n + n) % n;
}

constexpr long mod(long x, long n)
{
	return (x % n + n) % n;
}

/* Calculates least power of two that is greater or equal, up to 2^31. */
constexpr int nearestPowerOfTwo(int n)
{
	return (n <= 0) ? 1 : ((
		(n - 1)
			| (n >> 1)
			| (n >> 2)
			| (n >> 4)
			| (n >> 8)
			| (n >> 16)
		) + 1);
}

/* String conversion */
inline std::string tolower(std::string result)
{
	std::transform(result.begin(), result.end(), result.begin(),
		(int (*)(int))(std::tolower));
	return result;
}

inline std::string toupper(std::string result)
{
	std::transform(result.begin(), result.end(), result.begin(),
		(int (*)(int))(std::toupper));
	return result;
}

inline std::string toupper1(std::string result)
{
	if (!result.empty()) result[0] = std::toupper(result[0]);
	return result;
}

/* String sanitation */
inline std::string sanitize(const std::string& input)
{
	std::string output = input;
	for (char& x : output)
	{
		if (x == '_') continue;
		else if (x >= 'a' && x <= 'z') continue;
		else if (x >= 'A' && x <= 'Z') continue;
		else if (x >= '0' && x <= '9') continue;
		else
		{
			x = '_';
		}
	}
	return output;
}

/* UTF8 */
constexpr bool isContinuationByte(char c)
{
	// If the two most-significant bits are 10, i.e. the byte is 10xxxxxx,
	// this is a UTF8 continuation byte.
	return ((((uint8_t) c) & 0xC0) == 0x80);
}

constexpr bool isNonContinuationByte(char c)
{
	return !isContinuationByte(c);
}

/* Get the size of a locally declared array. */
template<size_t SIZE, class T>
constexpr size_t array_size(T (&/*arr*/)[SIZE])
{
	return SIZE;
}

/* Non-localized JSON-compatible boolean strings. */
constexpr const char* jsonify(bool value)
{
	return value ? "true" : "false";
}

#ifdef PLATFORMUNIX
#define x_snprintf snprintf
#else
__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}
__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}
#define x_snprintf c99_snprintf
#endif

/* Format like printf but returning a string. */
template<typename... Args>
inline std::string format(const char* format, Args... args)
{
	std::vector<char> buffer(1024);
	int n = x_snprintf(buffer.data(), buffer.size(), format, args...);
	DEBUG_ASSERT(n >= 0);
	if (n < 0)
	{
		return "";
	}
	if (((size_t) n) >= buffer.size())
	{
		buffer.resize(n + 1);
		n = x_snprintf(buffer.data(), buffer.size(), format, args...);
		DEBUG_ASSERT(n >= 0);
		if (n < 0)
		{
			return "";
		}
	}
	return std::string(buffer.begin(), buffer.begin() + n);
}

template<typename... Args>
inline std::string format(const std::string& format, Args... args)
{
	return ::format(format.c_str(), args...);
}
