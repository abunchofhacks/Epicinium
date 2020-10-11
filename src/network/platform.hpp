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


enum class Platform : uint8_t
{
	UNKNOWN = 0,
	DEBIAN32,
	DEBIAN64,
	WINDOWS32,
	WINDOWS64,
	OSX32,
	OSX64,
};

constexpr Platform platform()
{
#ifdef PLATFORMDEBIAN32
	return Platform::DEBIAN32;
#endif
#ifdef PLATFORMDEBIAN64
	return Platform::DEBIAN64;
#endif
#ifdef PLATFORMWINDOWS32
	return Platform::WINDOWS32;
#endif
#ifdef PLATFORMWINDOWS64
	return Platform::WINDOWS64;
#endif
#ifdef PLATFORMOSX32
	return Platform::OSX32;
#endif
#ifdef PLATFORMOSX64
	return Platform::OSX64;
#endif
}

Platform parsePlatform(const std::string& str);
const char* stringify(const Platform& Platform);

std::ostream& operator<<(std::ostream& os, const Platform& platform);
