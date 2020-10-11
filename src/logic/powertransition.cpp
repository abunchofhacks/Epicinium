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
#include "powertransition.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "board.hpp"
#include "changeset.hpp"
#include "cell.hpp"


constexpr PowerTransition::Stage PowerTransition::stages[];

namespace PowerFlag
{
	enum
	{
		EVALUATED = 0x01,
		POWER = 0x02,
		STACK = 0x04,
		DEPOWER = 0x08,
	};
}

PowerTransition::PowerTransition(const Bible& bible, Board& board,
		ChangeSet& changeset, Stage stage) :
	_bible(bible),
	_board(board),
	_changeset(changeset),
	_stage(stage),
	_results(_board.end().ix(), 0)
{}

void PowerTransition::execute()
{
	for (Cell index : _board)
	{
		map(index);
	}

	for (Cell index : _board)
	{
		reduce(index);
	}
}

// NOTE: Changes to this code should be copied to Commander::fillFocusPositions().
// TODO is there a way to automate that?
void PowerTransition::map(Cell index)
{
	const TileToken& tiletoken = _board.tile(index);
	const TileType tiletype = tiletoken.type;

	// Natural tiles do not grow.
	if (!_bible.tilePowered(tiletype)) return;

	// Neutral tiles do not grow.
	if (tiletoken.owner == Player::NONE) return;

	// Tiles that are occupied by an enemy unit do not grow.
	bool occupied = ((_board.ground(index)
				&& _board.ground(index).owner != tiletoken.owner
				&& _bible.unitCanOccupy(_board.ground(index).type))
		|| (_board.air(index)
				&& _board.air(index).owner != tiletoken.owner
				&& _bible.unitCanOccupy(_board.air(index).type)));
	if (occupied && !_bible.powerCanDrop())
	{
		return;
	}

	// Fully powered tiles do not grow.
	bool atmax = (tiletoken.power >= _bible.tilePowerMax(tiletype));
	if (atmax && !_bible.powerCanDrop())
	{
		return;
	}

	// Certain types of tiles grow in separate stages.
	switch (_stage)
	{
		case Stage::NICENESS:
		{
			if (!_bible.tileNeedsNiceness(tiletype)) return;
		}
		break;
		case Stage::LABOR:
		{
			if (!_bible.tileNeedsLabor(tiletype)) return;
		}
		break;
		case Stage::ENERGY:
		{
			if (!_bible.tileNeedsEnergy(tiletype)) return;
		}
		break;
		case Stage::ALL:
		break;
	}

	// Are all of the stacks powered?
	bool full = (tiletoken.power >= tiletoken.stacks);

	// Not all tiles gain a stack if all their stacks are powered.
	bool cangrow = (_bible.tileGrowthMax(tiletype) <= 0
		|| tiletoken.stacks < _bible.tileGrowthMax(tiletype));
	if (full && !cangrow && !_bible.powerCanDrop())
	{
		return;
	}

	// Evaluate the "niceness" of the surrounding tiles.
	int niceness = 0;
	switch (_stage)
	{
		case Stage::NICENESS:
		{
			niceness = evaluateNiceness(index);
		}
		break;

		case Stage::LABOR:
		{
			niceness = evaluateLabor(index);
		}
		break;

		case Stage::ENERGY:
		{
			niceness = evaluateEnergy(index);
		}
		break;

		case Stage::ALL:
		{
			if (_bible.tileNeedsNiceness(tiletype))
			{
				niceness = evaluateNiceness(index);
			}
			else if (_bible.tileNeedsLabor(tiletype))
			{
				niceness = evaluateLabor(index);
			}
			else if (_bible.tileNeedsEnergy(tiletype))
			{
				niceness = evaluateEnergy(index);
			}
			else if (_bible.tileNeedsTime(tiletype))
			{
				niceness = 1000;
			}
		}
		break;
	}

	// The niceness needs to be higher than the tile's current power for growth
	// to occur.
	bool powerup = !occupied && !atmax && (!full || cangrow)
		&& (niceness > tiletoken.power);

	// If the tile wants to grow but is full, it will create an extra stack.
	bool stackup = powerup && full;

	// Buildings depower if niceness drops.
	bool depower = _bible.powerCanDrop() && (niceness < tiletoken.power);

	// The power cannot drop, we also show tiles that fail to power up.
	if (_bible.powerCanDrop() && !powerup && !stackup && !depower) return;

	// Encode the bools as bits.
	uint8_t bits = PowerFlag::EVALUATED;
	if (powerup) bits |= PowerFlag::POWER;
	if (stackup) bits |= PowerFlag::STACK;
	if (depower) bits |= PowerFlag::DEPOWER;

	// We create a growth event, even if the niceness is not enough to support actual growth.
	_results[index.ix()] = bits;
}

int PowerTransition::evaluateNiceness(Cell index)
{
	// For city tiles, each surrounding grassy tile with stacks (i.e. forest)
	// provides a full point of niceness, each non-snowed grassy tile without
	// stacks (i.e. grass) provides a full point of niceness, and each other
	// non-snowed natural tile (water, crops) provides half a point.
	int niceness = 0;
	for (Cell neighbor : _board.area(index, 1, 2))
	{
		const TileToken& tile = _board.tile(neighbor);
		if (_board.snow(neighbor))
		{
			if (_bible.snowCoversNiceness())
			{
				if (_bible.tileGrassy(tile.type) && tile.stacks > 0)
				{
					niceness += 2;
				}
				// else nothing
			}
			else
			{
				// Old: each snowed tile provides half a point of niceness.
				niceness += 1;
			}
		}
		else if (_bible.tileGrassy(tile.type))
		{
			niceness += 2;
		}
		else if (_bible.tileNatural(tile.type))
		{
			niceness += 1;
		}
	}
	return (niceness / 2);
}

int PowerTransition::evaluateLabor(Cell index)
{
	// For industry tiles, each surrounding city owned by the same player
	// provides 1 niceness per powered stack.
	int niceness = 0;
	for (Cell neighbor : _board.area(index, 1, 2))
	{
		const TileToken& tile = _board.tile(neighbor);
		if (_bible.tileLaboring(tile.type)
			&& tile.owner == _board.tile(index).owner)
		{
			if (_bible.industryNicenessQuantitative())
			{
				// New style: each city and settlement only counts for 1 niceness if powered.
				niceness += (tile.power > 0) ? 1 : 0;
			}
			else
			{
				// Old style: the amount of power matters.
				niceness += tile.power;
			}
		}
	}
	return (niceness);
}

int PowerTransition::evaluateEnergy(Cell index)
{
	// For reactor tiles, each surrounding industry owned by the same player
	// provides 1 niceness per powered stack.
	int niceness = 0;
	for (Cell neighbor : _board.area(index, 1, 2))
	{
		const TileToken& tile = _board.tile(neighbor);
		if (_bible.tileEnergizing(tile.type)
			&& tile.owner == _board.tile(index).owner)
		{
			if (_bible.reactorNicenessQuantitative())
			{
				// New style: each industry only counts for 1 niceness if powered.
				niceness += (tile.power > 0) ? 1 : 0;
			}
			else
			{
				// Old style: the amount of power matters.
				niceness += tile.power;
			}
		}
	}
	return (niceness);
}

void PowerTransition::reduce(Cell index)
{
	uint8_t result = _results[index.ix()];

	// We only put a result if niceness was evaluated,
	// but also if no growth occurred.
	if (result)
	{
		bool powerup = result & PowerFlag::POWER;
		bool stackup = result & PowerFlag::STACK;
		bool depower = result & PowerFlag::DEPOWER;
		Descriptor desc = Descriptor::tile(index.pos());
		Change change(Change::Type::GROWS, desc, stackup, powerup - depower);
		_board.enact(change);
		_changeset.push(change, _board.vision(index));
	}
}
