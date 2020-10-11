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


struct Position
{
	// The board size must be strictly smaller than 128x128 because we use
	// int8_t to store _rows in Cell, so it can have values 1 to 127 inclusive.
	// We use a much smaller maximum value to help train neural networks.
	static constexpr int MAX_COLS = 32;
	static constexpr int MAX_ROWS = MAX_COLS;

	int8_t row;
	int8_t col;

	Position() :
		Position(0, 0) {}

	Position(int8_t r, int8_t c) :
		row(r),
		col(c) {}

	bool operator==(const Position& other) const
	{
		return (row == other.row && col == other.col);
	}

	bool operator!=(const Position& other) const
	{
		return !(*this == other);
	}

	bool operator<(const Position& other) const
	{
		return std::tie(row, col) < std::tie(other.row, other.col);
	}

	explicit Position(const Json::Value& json);

	friend std::ostream& operator<<(std::ostream& os, const Position& pos);
};
