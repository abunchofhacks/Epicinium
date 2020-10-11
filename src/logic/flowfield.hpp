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

#include <map>
#include <queue>

#include "board.hpp"
#include "cell.hpp"
#include "move.hpp"

class Bible;
class ChangeSet;


// Flowfield can be used to generate the shortest paths using a floodfill
// search. It does it by first "mapping" each cell, which allows it to mark
// cells, then repeatedly "flowing" outward from marked cells and remembering
// the Move, and lastly "reducing" all cells to handle their mark if necessary.
// In order to generate a full flowfield, cells that the flow cannot go through
// can be given a low priority so they are handled after all accessible cells.
// Like current, the flow flows inward towards the sources.

template <class This>
class Flowfield
{
public:
	Flowfield(const Bible& bible, Board& board) :
		_bible(bible),
		_board(board)
	{}

protected:
	const Bible& _bible;
	Board& _board;

private:
	std::map<Cell, Move> _flows;
	std::queue<Cell> _queue;
	std::queue<Cell> _forcedqueue;
	bool _forced = false;

protected:
	bool forced() { return _forced; }

	void force(Cell index, const Move& move)
	{
		if (_flows.count(index)) return;
		_flows[index] = move;
		_forcedqueue.push(index);
	}

	void put(Cell index, const Move& move)
	{
		if (_flows.count(index)) return;
		_flows[index] = move;
		_queue.push(index);
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
		while (!_queue.empty())
		{
			Cell index = _queue.front();
			_queue.pop();
			static_cast<This*>(this)->flood(index);
		}
		_forced = true;
		while (!_forcedqueue.empty())
		{
			Cell index = _forcedqueue.front();
			_forcedqueue.pop();
			static_cast<This*>(this)->flood(index);
		}
	}

public:
	Move step(Cell index)
	{
		return _flows.count(index) ? _flows.at(index) : Move::X;
	}
};

