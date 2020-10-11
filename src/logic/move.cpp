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
#include "move.hpp"
#include "source.hpp"

#include "parseerror.hpp"


Move parseMove(const std::string& str)
{
	if      (str == "self")  return Move::X;
	else if (str == "east")  return Move::E;
	else if (str == "south") return Move::S;
	else if (str == "west")  return Move::W;
	else if (str == "north") return Move::N;
	else throw ParseError("Unknown move '" + str + "'");
}

const char* stringify(const Move& move)
{
	switch (move)
	{
		case Move::X: return "self";
		case Move::E: return "east";
		case Move::S: return "south";
		case Move::W: return "west";
		case Move::N: return "north";
	}
	return "error";
}

std::ostream& operator<<(std::ostream& os, const Move& move)
{
	return os << stringify(move);
}
