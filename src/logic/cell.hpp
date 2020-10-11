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


class Cell
{
private:
	// Cell is a 16-bit index into the area of Spaces or Squares that can
	// magically transform into a Position because the grid dimensions are
	// stored alongside it. For all cells except undefined() the array index
	// ix() is guaranteed to be between 0 and (rows() * cols()) inclusive, so
	// there is no need to check for edge() in that case. However pos() is only
	// well defined for non-edge Cells.
	uint8_t _rows;
	uint8_t _cols;
	uint16_t _ix;

	friend class Board;
	friend class Level;
	friend class Area;
	friend class AreaSanityTest;

	// TODO cleanup (#948, #1191)
	friend class Square;

	Cell() = default;

	static Cell create(int rr, int cc, int i)
	{
		Cell that;
		that._rows = rr;
		that._cols = cc;
		that._ix = i;
		return that;
	}

	void assign(int i)
	{
		_ix = i;
	}

	constexpr int rows() const
	{
		return _rows;
	}

	constexpr int cols() const
	{
		return _cols;
	}

public:
	constexpr int ix() const
	{
		return _ix;
	}

	constexpr bool valid() const
	{
		return (ix() < rows() * cols());
	}

	constexpr bool edge() const
	{
		return !valid();
	}

	// Precondition: valid()
	Position pos() const
	{
		return Position(ix() / cols(), ix() % cols());
	}

	// Cell can act as a const_iterator over cells with indices from 0 inclusive
	// to ix() exclusive, with Cell as value_type.

	Cell& operator++()
	{
		++_ix;
		return *this;
	}

	Cell operator++(int /**/)
	{
		Cell iter = *this;
		++_ix;
		return iter;
	}

	Cell operator*() const
	{
		return *this;
	}

	const Cell* operator->() const
	{
		return this;
	}

	friend bool operator==(Cell a, Cell b)
	{
		return (a._ix == b._ix);
	}

	friend bool operator!=(Cell a, Cell b)
	{
		return (a._ix != b._ix);
	}

	friend bool operator<(Cell a, Cell b)
	{
		return (a._ix < b._ix);
	}

	Cell begin() const
	{
		Cell iter = *this;
		iter.assign(0);
		return iter;
	}

	Cell end() const
	{
		Cell iter = *this;
		return iter;
	}

	// Note that an uninitialized uint8_t is at most 0xFF, so valid() will fail
	// for undefined() as desired because 0xFFFF >= 0xFE01 == 0xFF * 0xFF.
	static Cell undefined()
	{
		Cell that;
		//that._rows = __uninitialized__;
		//that._cols = __uninitialized__;
		that._ix = 0xFFFF;
		return that;
	}

	inline Cell eswn(const Move& move) const
	{
		Cell that = *this;
		int rr = rows();
		int cc = cols();
		int i = ix();
		int r = (i / cc) + (move == Move::S) - (move == Move::N);
		int c = (i % cc) + (move == Move::E) - (move == Move::W);
		if (r >= 0 && c >= 0 && r < rr && c < cc)
		{
			that.assign(r * cc + c);
		}
		else
		{
			that.assign(rr * cc);
		}
		return that;
	}

	friend Cell operator+(Cell cell, const Move& move)
	{
		return cell.eswn(move);
	}

	friend std::ostream& operator<<(std::ostream& os, Cell index);
};
