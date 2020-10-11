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
#include "pathingflowfield.hpp"
#include "source.hpp"

#include "bible.hpp"


void PathingFlowfield::flood(Cell from)
{
	std::vector<Move> moves = {Move::E, Move::S, Move::W, Move::N};
	std::random_shuffle(moves.begin(), moves.end());
	for (const Move& move : moves)
	{
		Cell to = from + move;
		TileType tiletype = _board.tile(to).type;

		if (tiletype == TileType::NONE) continue;

		if (!forced() && _bible.tileAccessible(tiletype)
			&& (_air || _bible.tileWalkable(tiletype)))
		{
			put(to, ::flip(move));
		}
		else
		{
			force(to, ::flip(move));
		}
	}
}
