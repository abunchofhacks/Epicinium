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

#include "board.hpp"
#include "cell.hpp"

class Bible;
class ChangeSet;


// XyzFloodfill can be used to perform a floodfill search for all cells that
// have property Xyz. It does it by first "mapping" each cell, which allows it
// to mark cells and surrounding cells, then repeatedly "flooding" marked cells,
// and lastly "reducing" all cells to handle their mark.

template <class This>
class Floodfill
{
public:
	Floodfill(const Bible& bible, Board& board) :
		_bible(bible),
		_board(board),
		_marks(_board.end().ix() + 1, 0),
		_queue(_marks.size(), Cell::undefined())
	{
		// We want _board.end() to be marked with a near-infinite mark.
		_marks[_board.end().ix()] = uint16_t(-2);
	}

protected:
	const Bible& _bible;
	Board& _board;

private:
	std::vector<uint16_t> _marks;
	std::vector<Cell> _queue;
	size_t _head = 0;
	size_t _size = 0;

public:
	void reset()
	{
		std::fill(_marks.begin(), _marks.end(), 0);
		// Everything in _queue past _size is considered garbage.
		_head = 0;
		_size = 0;
	}

protected:
	void put(Cell from, Cell to)
	{
		// Inside _marks, 0 means unmarked.
		// If it is already marked, do not add it to the queue.
		if (_marks[to.ix()] != 0) return;
		// If "from" is marked with X, we want "to" to be marked with X+1.
		// Because there are rows * cols <= 0xFF * 0xFF < 0xFFFF spaces on any
		// given board, no overflow can occur during flooding.
		_marks[to.ix()] = _marks[from.ix()] + 1;
		// Only add "to" to the queue if it was previously unmarked.
		_queue[_size++] = to;
	}

	uint16_t get(Cell at) const
	{
		// Inside _marks, 0 means unmarked so any marks are 1 too high.
		// The underflow from 0 (unmarked) to uint16_t(-1) here is intentional.
		return uint16_t(_marks[at.ix()] - 1);
	}

	void put(Cell at)
	{
		// Inside _marks, 0 means unmarked.
		// If it is already marked, do not add it to the queue.
		if (_marks[at.ix()] != 0) return;
		// We want to store 0, but we must add 1 to mark it.
		_marks[at.ix()] = 0 + 1;
		// Only add "at" to the queue if it was previously unmarked.
		_queue[_size++] = at;
	}

	// void flood(Cell index) = 0;
	// void map(Cell index) = 0;
	// void reduce(Cell index) = 0;

public:
	void execute()
	{
		for (Cell index : _board)
		{
			static_cast<This*>(this)->map(index);
		}
		fill();
		for (Cell index : _board)
		{
			static_cast<This*>(this)->reduce(index);
		}
	}

protected:
	void fill()
	{
		while (_head < _size)
		{
			Cell index = _queue[_head];
			static_cast<This*>(this)->flood(index);
			_head++;
		}
	}
};

