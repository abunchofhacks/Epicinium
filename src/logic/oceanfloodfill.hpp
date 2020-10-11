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

#include "floodfill.hpp"

class Bible;
class Board;
class Cell;


class OceanFloodfill : public Floodfill<OceanFloodfill>
{
public:
	OceanFloodfill(const Bible& bible, Board& board);

private:
	const int8_t _maxrow;
	const int8_t _maxcol;

	void map(Cell index);
	void flood(Cell index);
	void reduce(Cell /**/) {}

	friend Floodfill;

public:
	bool ocean(Cell at) const;
};
