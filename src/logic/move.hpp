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


enum class Move : uint8_t
{
	X, E, S, W, N
};

static constexpr size_t MOVE_SIZE = ((size_t) Move::N) + 1;

inline Move flip(const Move& move)
{
	switch (move)
	{
		case Move::E: return Move::W;
		case Move::S: return Move::N;
		case Move::W: return Move::E;
		case Move::N: return Move::S;
		default: return Move::X;
	}
}

Move parseMove(const std::string& str);
const char* stringify(const Move& move);

std::ostream& operator<<(std::ostream& os, const Move& move);
