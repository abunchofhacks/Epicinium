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
#include "order.hpp"

class Bible;
class Board;
class Sprite;
class Guide;


class TargetCursor
{
public:
	TargetCursor(const Bible& bible, const Board& board, Cell from,
		const Order& order,
		const char* stampspritename);
	~TargetCursor();

	void update();

	void set(Cell to);

	bool valid() const;
	Order resolve() const;

private:
	const Bible& _bible;
	const Board& _board;
	Cell _origin;
	Cell _target;
	Order _order;
	int _minrange;
	int _maxrange;
	bool _valid;
	std::shared_ptr<Sprite> _hoversprite; // (unique ownership)
	std::shared_ptr<Sprite> _stampsprite; // (unique ownership)
	std::shared_ptr<Sprite> _invalidsprite; // (unique ownership)
	std::shared_ptr<Sprite> _outofboundssprite; // (unique ownership)
	std::vector<Cell> _foci;
	std::vector<Guide> _targetguides;

	void determineRanges();

	bool checkValidity() const;
	bool checkValidity(Cell target) const;

public:
	const std::vector<Cell>& foci() const { return _foci; }
};
