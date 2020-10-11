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
#include "visiontype.hpp"
#include "source.hpp"

#include "parseerror.hpp"


VisionType parseVisionType(const std::string& str)
{
	if      (str == "none")     return VisionType::NONE;
	if      (str == "normal")   return VisionType::NORMAL;
	else if (str == "global")   return VisionType::GLOBAL;
	else throw ParseError("Unknown VisionType '" + str + "'");
}

const char* stringify(const VisionType& type)
{
	switch (type)
	{
		case VisionType::NONE:     return "none";
		case VisionType::NORMAL:   return "normal";
		case VisionType::GLOBAL:   return "global";
	}
	return "error";
}

std::ostream& operator<<(std::ostream& os, const VisionType& type)
{
	return os << stringify(type);
}
