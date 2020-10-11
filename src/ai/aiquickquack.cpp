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
#include "aiquickquack.hpp"
#include "source.hpp"

#include "pathingfloodfill.hpp"
#include "difficulty.hpp"


std::string AIQuickQuack::ainame() const
{
	return "QuickQuack";
}

std::string AIQuickQuack::authors() const
{
	return "Daan Mulder";
}

bool AIQuickQuack::cityOccupied(Cell index) const
{
	if (!_bible.tileBinding(_board.tile(index).type)) return false;
	if (_board.ground(index).type == UnitType::NONE) return false;
	if (_board.tile(index).owner == _board.ground(index).owner) return false;
	return true;
}

AIQuickQuack::AIQuickQuack(const Player& player, const Difficulty& difficulty,
		const std::string& rulesetname, char character) :
	AICommander(player, difficulty, rulesetname, character)
{
	if (_difficulty == Difficulty::NONE)
	{
		LOGW << "AI difficulty not set";
		DEBUG_ASSERT(false);
	}

	for (TileType tiletype : _bible.tiletypes())
	{
		if (_bible.tileBinding(tiletype))
		{
			_citytypes.emplace_back(tiletype);
		}
	}
}

void AIQuickQuack::process()
{
	// first priority: capture anything!
	for (Cell index : _board)
	{
		if (_newOrders.size() >= _bible.newOrderLimit()) break;
		if (!_bible.unitCanCapture(_board.ground(index).type)) continue;
		if (_board.ground(index).owner != _player) continue;
		if (!_bible.tileOwnable(_board.tile(index).type)) continue;
		if (_board.tile(index).owner == _player) continue;
		Descriptor subject = Descriptor::ground(index.pos());
		if (hasNew(subject).type != Order::Type::NONE) continue;
		if (hasUnfinished(subject).type == Order::Type::CAPTURE) continue;
		_newOrders.emplace_back(Order::Type::CAPTURE, subject);
	}

	std::vector<Cell> myMilitaryPositions;
	for (Cell index : _board)
	{
		if (!_bible.unitCanAttack(_board.ground(index).type)) continue;
		if (_board.ground(index).owner != _player) continue;
		if (cityOccupied(index)) continue;
		Descriptor subject = Descriptor::ground(index.pos());
		if (hasNew(subject).type != Order::Type::NONE) continue;
		const Order& unfinished = hasUnfinished(subject);
		if (unfinished.type != Order::Type::NONE
			&& (unfinished.type != Order::Type::MOVE
			|| !cityOccupied(_board.cell(unfinished.target.position)))) continue;
		myMilitaryPositions.emplace_back(index);
	}

	// second priority: move to cities!
	TileFloodfill cities(_bible, _board);
	cities.include(_citytypes);
	cities.exclude({_player});
	cities.excludeOccupied();
	cities.execute();
	std::random_shuffle(myMilitaryPositions.begin(), myMilitaryPositions.end());
	std::sort(myMilitaryPositions.begin(), myMilitaryPositions.end(),
		[&](Cell lhs, Cell rhs) {

			return cities.steps(lhs) < cities.steps(rhs);
		});
	for (Cell index : myMilitaryPositions)
	{
		if (!cities.reached(index)) continue;
		if (_newOrders.size() + 1 >= _bible.newOrderLimit()) break;
		std::vector<Move> moves;
		Move current;
		Cell destination = index;
		while ((current = cities.step(destination)) != Move::X)
		{
			moves.emplace_back(current);
			destination = destination + current;
		}
		_newOrders.emplace_back(Order::Type::MOVE,
			Descriptor::ground(index.pos()),
			Descriptor::cell(destination.pos()),
			moves);
	}

	// third priority: make more military!
	std::vector<Cell> myCityPositions;
	for (Cell index : _board)
	{
		if (!_bible.tileControllable(_board.tile(index).type)) continue;
		if (_board.tile(index).owner != _player) continue;
		myCityPositions.emplace_back(index);
	}
	std::random_shuffle(myCityPositions.begin(), myCityPositions.end());
	int militaryCount = 0;
	for (Cell index : _board)
	{
		if (_bible.unitCanAttack(_board.ground(index).type)
			&& _board.ground(index).owner == _player)
		{
			militaryCount++;
		}
	}
	for (Cell index : myCityPositions)
	{
		const TileToken& tile = _board.tile(index);
		if (_newOrders.size() >= _bible.newOrderLimit()) break;
		int power = _board.tile(index).power;
		if (militaryCount < 2)
		{
			if (power < _bible.tilePowerMax(tile.type) - 3) continue;
		}
		else
		{
			if (power < _bible.tilePowerMax(tile.type) - 1) continue;
		}
		uint16_t cost = uint16_t(-1);
		UnitType unittype;
		for (const Bible::UnitBuild& build : _bible.tileProduces(tile.type))
		{
			if (!_bible.unitCanAttack(build.type)) continue;
			cost = build.cost;
			unittype = build.type;
			break;
		}
		if (_money < cost) continue;
		Descriptor subject = Descriptor::tile(index.pos());
		if (hasNew(subject).type != Order::Type::NONE) continue;
		if (hasUnfinished(subject).type != Order::Type::NONE) continue;
		_newOrders.emplace_back(Order::Type::PRODUCE, subject,
			Descriptor::cell(subject.position),
			unittype);
		militaryCount++;
	}

	if (_difficulty == Difficulty::EASY)
	{
		// Throw away all but one order
		std::random_shuffle(_newOrders.begin(), _newOrders.end());
		if (_newOrders.size() > 1) _newOrders = {_newOrders[0]};
	}
	else if (_difficulty == Difficulty::MEDIUM)
	{
		// Throw away all but three orders
		std::random_shuffle(_newOrders.begin(), _newOrders.end());
		if (_newOrders.size() > 3)
		{
			_newOrders = {_newOrders[0], _newOrders[1], _newOrders[2]};
		}
	}
}
