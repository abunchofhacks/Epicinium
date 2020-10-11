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
#include "target.hpp"
#include "source.hpp"

#include "parseerror.hpp"


Target parseTarget(const std::string& str)
{
	if      (str == "none")    return Target::NONE;
	if      (str == "general") return Target::GENERAL;
	else if (str == "lobby")   return Target::LOBBY;
	else throw ParseError("Unknown target '" + str + "'");
}

const char* stringify(const Target& target)
{
	switch (target)
	{
		case Target::NONE:    return "none";
		case Target::GENERAL: return "general";
		case Target::LOBBY:   return "lobby";
	}
	return "error";
}

std::ostream& operator<<(std::ostream& os, const Target& target)
{
	return os << stringify(target);
}
