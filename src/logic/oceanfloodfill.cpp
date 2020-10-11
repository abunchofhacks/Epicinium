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
#include "oceanfloodfill.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "board.hpp"
#include "tiletoken.hpp"
#include "position.hpp"
#include "move.hpp"



OceanFloodfill::OceanFloodfill(const Bible& bible, Board& board) :
	Floodfill(bible, board),
	_maxrow(board.rows() - 1),
	_maxcol(board.cols() - 1)
{}

void OceanFloodfill::map(Cell index)
{
	Position pos = index.pos();
	if (pos.row == 0 || pos.row == _maxrow || pos.col == 0 || pos.col == _maxcol)
	{
		if (_bible.tileWater(_board.tile(index).type))
		{
			put(index);
		}
	}
}

void OceanFloodfill::flood(Cell from)
{
	for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
	{
		Cell to = from + move;
		if (_bible.tileWater(_board.tile(to).type))
		{
			put(from, to);
		}
	}
}

bool OceanFloodfill::ocean(Cell at) const
{
	return get(at) != uint16_t(-1);
}
