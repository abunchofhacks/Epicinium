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

#include "position.hpp"
#include "move.hpp"


struct Aim
{
	int8_t rowdiff;
	int8_t coldiff;

	bool operator==(const Aim& other) const
	{
		return rowdiff == other.rowdiff && coldiff == other.coldiff;
	}

	bool operator!=(const Aim& other) const
	{
		return !(*this == other);
	}

	Aim(int8_t r, int8_t c) : rowdiff(r), coldiff(c) {}

	explicit Aim(const Move& move) :
		Aim((move == Move::S) - (move == Move::N),
			(move == Move::E) - (move == Move::W))
	{}

	explicit Aim(const Position& from, const Position& to) :
		Aim(to.row - from.row, to.col - from.col)
	{}

	int length()
	{
		return abs((int) rowdiff) + abs((int) coldiff);
	}

	int sumofsquares()
	{
		return rowdiff * rowdiff + coldiff * coldiff;
	}

	Move direction() const;

	std::vector<Move> deconstruct() const;

	explicit Aim(const Json::Value& json);

	friend std::ostream& operator<<(std::ostream& os, const Aim& aim);
};

inline Position operator+(Position pos, const Aim& aim)
{
	pos.row += aim.rowdiff;
	pos.col += aim.coldiff;
	return pos;
}
