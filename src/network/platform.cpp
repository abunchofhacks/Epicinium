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
#include "platform.hpp"
#include "source.hpp"

#include "parseerror.hpp"


Platform parsePlatform(const std::string& str)
{
	if      (str == "unknown")             return Platform::UNKNOWN;
	else if (str == "debian32")            return Platform::DEBIAN32;
	else if (str == "debian64")            return Platform::DEBIAN64;
	else if (str == "windows32")           return Platform::WINDOWS32;
	else if (str == "windows64")           return Platform::WINDOWS64;
	else if (str == "osx32")               return Platform::OSX32;
	else if (str == "osx64")               return Platform::OSX64;
	else throw ParseError("Unknown platform '" + str + "'");
}

const char* stringify(const Platform& platform)
{
	switch (platform)
	{
		case Platform::UNKNOWN:            return "unknown";
		case Platform::DEBIAN32:           return "debian32";
		case Platform::DEBIAN64:           return "debian64";
		case Platform::WINDOWS32:          return "windows32";
		case Platform::WINDOWS64:          return "windows64";
		case Platform::OSX32:              return "osx32";
		case Platform::OSX64:              return "osx64";
	}
	return "error";
}

std::ostream& operator<<(std::ostream& os, const Platform& platform)
{
	return os << stringify(platform);
}
