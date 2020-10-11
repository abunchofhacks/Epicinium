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

#include "randomizer.hpp"
#include "cell.hpp"

class Bible;
class Board;
class ChangeSet;
enum class Season : uint8_t;


class ChaosTransition
{
public:
	ChaosTransition(const Bible& bible, Board& board, ChangeSet& changeset);

private:
	const Bible& _bible;
	Board& _board;
	ChangeSet& _changeset;

	int _remainder;

	Randomizer<Cell> _randompositions;

	void map(Cell index);
	void enact(Cell index);

	void distribute(int amount);

	void fillRandomPositions();

public:
	void execute();
};
