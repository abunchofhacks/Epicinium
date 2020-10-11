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

#include "cell.hpp"

class Board;
class TypeNamer;
class Sprite;


class Cursor
{
public:
	enum class State : uint8_t
	{
		ACTIVE,
		BUSY,
		WAITING,
		READING,
	};

	Cursor(const Board& board, const TypeNamer& typenamer);
	~Cursor();

	void update();

	void set(Cell cell)
	{
		_cell = cell;
	}

	void setState(State state)
	{
		_state = state;
	}

	State state() const
	{
		return _state;
	}

	bool showRealCursor()
	{
		return _state == State::ACTIVE;
	}

private:
	const Board& _board;
	const TypeNamer& _typenamer;
	Cell _cell;
	State _state;
	std::shared_ptr<Sprite> _hoversprite; // (unique ownership)
	std::shared_ptr<Sprite> _busysprite; // (unique ownership)
	std::shared_ptr<Sprite> _waitingsprite; // (unique ownership)
	std::shared_ptr<Sprite> _readingsprite; // (unique ownership)
	std::shared_ptr<Sprite> _outofboundssprite; // (unique ownership)
};
