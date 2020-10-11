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
#include "aim.hpp"
#include "source.hpp"


Move Aim::direction() const
{
	if      (coldiff > 0) return Move::E;
	else if (coldiff < 0) return Move::W;
	else if (rowdiff > 0) return Move::S;
	else if (rowdiff < 0) return Move::N;
	else                  return Move::X;
}

std::vector<Move> Aim::deconstruct() const
{
	std::vector<Move> moves;
	bool horizontalfirst = ((coldiff > 0 && rowdiff >= 0) || (coldiff < 0 && rowdiff <= 0));
	int8_t r = 0;
	int8_t c = 0;
	for (int step = 0; step < abs(rowdiff) + abs(coldiff); step++)
	{
		int cc = (coldiff - c) * (coldiff - c);
		int rr = (rowdiff - r) * (rowdiff - r);
		bool horizontal = (cc > rr || (horizontalfirst && cc == rr));
		if      ( horizontal && coldiff > 0)
		{
			c += 1; moves.push_back(Move::E);
		}
		else if ( horizontal && coldiff < 0)
		{
			c -= 1; moves.push_back(Move::W);
		}
		else if (!horizontal && rowdiff > 0)
		{
			r += 1; moves.push_back(Move::S);
		}
		else if (!horizontal && rowdiff < 0)
		{
			r -= 1; moves.push_back(Move::N);
		}
		horizontalfirst = horizontal;
	}
	return moves;
}

Aim::Aim(const Json::Value& json) :
	rowdiff(json["rowdiff"].asInt()),
	coldiff(json["coldiff"].asInt())
{}

std::ostream& operator<<(std::ostream& os, const Aim& aim)
{
	return os << "{"
		"\"rowdiff\":" << ((int) aim.rowdiff) << ""
		","
		"\"coldiff\":" << ((int) aim.coldiff) << ""
		"}";
}
