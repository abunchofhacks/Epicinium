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
#include "treetype.hpp"
#include "source.hpp"

#include "stringref.hpp"


const char* stringify(TreeType treetype)
{
	switch (treetype)
	{
		case TreeType::UNSET:  return "none";
		case TreeType::OAK:    return "oak";
		case TreeType::BIRCH:  return "birch";
		case TreeType::POPLAR: return "poplar";
		case TreeType::SPRUCE: return "spruce";
		case TreeType::PALM:   return "palm";
	}
	return "";
}

TreeType parseTreeType(stringref str)
{
	if (str == "none")   return TreeType::UNSET;
	if (str == "unset")  return TreeType::UNSET;
	if (str == "oak")    return TreeType::OAK;
	if (str == "birch")  return TreeType::BIRCH;
	if (str == "poplar") return TreeType::POPLAR;
	if (str == "spruce") return TreeType::SPRUCE;
	if (str == "palm")   return TreeType::PALM;

	LOGE << "Unknown TreeType '" << str << "'";
	DEBUG_ASSERT(false);
	return TreeType::UNSET;
}

std::ostream& operator<<(std::ostream& os, const TreeType& type)
{
	return os << ::stringify(type);
}
