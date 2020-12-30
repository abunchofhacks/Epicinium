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

/* This file can be included into every .hpp header file. */

#include <cstdlib>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <iosfwd>
#include <cassert>
#include <array>
#include <cctype>
#include <tuple>

/* If we are running dev, we want as many crashes as possible. */
// Usage: "DEBUG_ASSERT(xyz);"
#ifdef DEVELOPMENT
#define DEBUG_ASSERT(x) assert(x)
#else
#define DEBUG_ASSERT(x) do { (void)sizeof(x); } while (0) // "Stupid C++ Tricks"
#endif
// Usage: "RETHROW_IF_DEV();"
#ifdef DEVELOPMENT
#define RETHROW_IF_DEV() throw
#else
#define RETHROW_IF_DEV() ((void)0)
#endif

#include "libs/jsoncpp/json-forwards.h"

#ifdef PLATFORMDEBIAN32
#define PLATFORMDEBIAN
#define PLATFORMUNIX
#endif
#ifdef PLATFORMDEBIAN64
#define PLATFORMDEBIAN
#define PLATFORMUNIX
#endif
#ifdef PLATFORMOSX32
#define PLATFORMOSX
#define PLATFORMUNIX
#endif
#ifdef PLATFORMOSX64
#define PLATFORMOSX
#define PLATFORMUNIX
#endif
#ifdef PLATFORMWINDOWS32
#define PLATFORMWINDOWS
#endif
#ifdef PLATFORMWINDOWS64
#define PLATFORMWINDOWS
#endif

struct stringref;

#if defined(__GNUC__) || defined(__clang__)
#define ATTRIBUTE_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define ATTRIBUTE_WARN_UNUSED_RESULT
#endif

/* We can turn these features on or off; they are not available everywhere. */

#ifndef COREDUMP_ENABLED
#ifdef PLATFORMUNIX
#define COREDUMP_ENABLED true
#else
#define COREDUMP_ENABLED false
#endif
#endif

#ifndef STATIC_WRITER_ENABLED
#define STATIC_WRITER_ENABLED true
#endif

#ifndef INCLUDE_IMGUI_ENABLED
#define INCLUDE_IMGUI_ENABLED true
#endif

#ifndef INTL_ENABLED
#define INTL_ENABLED true
#endif

#ifndef LOG_REPLACE_WITH_CALLBACK_ENABLED
#define LOG_REPLACE_WITH_CALLBACK_ENABLED false
#endif

#ifndef FEMTOZIP_ENABLED
#define FEMTOZIP_ENABLED false
#endif

#ifndef DICTATOR_ENABLED
#define DICTATOR_ENABLED false
#endif

#ifndef LIBLOADER_ENABLED
#define LIBLOADER_ENABLED false
#endif

#ifndef VALGRIND_INTEGRATION_ENABLED
#define VALGRIND_INTEGRATION_ENABLED false
#endif

#ifndef STEAM_ENABLED
#define STEAM_ENABLED false
#endif

#ifndef SELF_PATCH_ENABLED
#if STEAM_ENABLED
#define SELF_PATCH_ENABLED false
#else
#ifdef PLATFORMOSX
#define SELF_PATCH_ENABLED false
#else
#define SELF_PATCH_ENABLED true
#endif
#endif
#endif

#ifndef DISCORD_GUEST_ENABLED
#define DISCORD_GUEST_ENABLED false
#endif

#ifndef EDITOR_DEPRECATED_ENABLED
#define EDITOR_DEPRECATED_ENABLED false
#endif
