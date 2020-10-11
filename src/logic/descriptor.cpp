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
#include "descriptor.hpp"
#include "source.hpp"

#include "parseerror.hpp"


Descriptor::Type Descriptor::parseType(const std::string& str)
{
	if      (str == "cell")   return Descriptor::Type::CELL;
	else if (str == "tile")   return Descriptor::Type::TILE;
	else if (str == "ground") return Descriptor::Type::GROUND;
	else if (str == "air")    return Descriptor::Type::AIR;
	else if (str == "bypass") return Descriptor::Type::BYPASS;
	else if (str == "none")   return Descriptor::Type::NONE;
	else throw ParseError("Unknown descriptor type '" + str + "'");
}

const char* Descriptor::stringify(const Descriptor::Type& type)
{
	switch (type)
	{
		case Descriptor::Type::CELL:   return "cell";
		case Descriptor::Type::TILE:   return "tile";
		case Descriptor::Type::GROUND: return "ground";
		case Descriptor::Type::AIR:    return "air";
		case Descriptor::Type::BYPASS: return "bypass";
		case Descriptor::Type::NONE:   return "none";
	}
	return "error";
}

Descriptor::Descriptor(const Json::Value& json) :
	type(parseType(json["type"].asString())),
	position(json)
{}

std::ostream& operator<<(std::ostream& os, const Descriptor& desc)
{
	return os << "{"
		"\"type\":\"" << Descriptor::stringify(desc.type) << "\""
		","
		"\"row\":" << ((int) desc.position.row) << ""
		","
		"\"col\":" << ((int) desc.position.col) << ""
		"}";
}

std::ostream& operator<<(std::ostream& os, const Descriptor::Type& type)
{
	return os << Descriptor::stringify(type);
}
