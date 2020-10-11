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
#include "visiontransition.hpp"
#include "source.hpp"

#include "position.hpp"
#include "bible.hpp"
#include "board.hpp"
#include "playerinfo.hpp"
#include "changeset.hpp"
#include "tiletoken.hpp"
#include "unittoken.hpp"


template <class This>
VisionTMRI<This>::VisionTMRI(const Bible& bible, Board& board,
			PlayerInfo& info,
			ChangeSet& changeset) :
	_bible(bible),
	_board(board),
	_info(info),
	_changeset(changeset),
	_results(_board.end().ix())
{}

template <class This>
void VisionTMRI<This>::execute()
{
	// Map.
	for (Cell index : _board)
	{
		map(index);
	}

	// Reduce.
	for (Cell index : _board)
	{
		reduce(index);
	}
}

template <class This>
void VisionTMRI<This>::executeAround(Cell near, int range)
{
	// We have a range R and a vision radius V. We want to calculate a value M
	// such that d(a, b) <= M iff exists c with d(a, c) <= R and d(c, b) <= V
	// for all a and b. Because we use square Euclidean distance,
	// we can't use the triangle inequality and just have M = R + V.
	// Instead we must calculate a different integer upper bound for M.
	// If R = x^2, V = y^2 and M = z^2 then basic algebra states
	// R + V = x^2 + y^2 <= (x + y)^2. But we don't want to calculate x and y.
	// Instead we approximate
	// (x + y)^2 = x^2 + 2xy + y^2 = x^2 + 2 * max(x^2, y^2) + y^2
	// which gives M = R + V + 2 * max(R, V).
	// Also see Automaton::processMove for more algebra.
	const int vmax = _bible.unitVisionMax();
	const int emrange = range + vmax + 2 * std::max(range, vmax);

	// Map.
	for (Cell index : _board.area(near, 0, emrange))
	{
		map(index);
	}

	// Reduce.
	for (Cell index : _board.area(near, 0, range))
	{
		reduce(index);
	}
}

template <class This>
void VisionTMRI<This>::map(Cell index)
{
	// The tile and units in this space may provide vision.
	provider(index, _board.tile(index));
	provider(index, _board.ground(index));
	provider(index, _board.air(index));
	provider(index, _board.bypass(index));
}

template <class This>
void VisionTMRI<This>::provider(Cell from, const TileToken& tile)
{
	// Is there a tile?
	if (!tile) return;

	// Uncontrolled tiles do not give vision.
	if (tile.owner == Player::NONE) return;

	// The tile provides vision in an area based on its vision radius.
	int range = _bible.tileVision(tile.type);
	for (Cell to : _board.area(from, 0, range))
	{
		_results[to.ix()].add(tile.owner);
	}
}

template <class This>
void VisionTMRI<This>::provider(Cell from, const UnitToken& unit)
{
	// Is there a unit?
	if (!unit) return;

	// Uncontrolled units do not give vision.
	if (unit.owner == Player::NONE) return;

	// The unit provides vision in an area based on its vision radius.
	int range = _bible.unitVision(unit.type);
	for (Cell to : _board.area(from, 0, range))
	{
		_results[to.ix()].add(unit.owner);
	}
}

void VisionTransition::reduce(Cell index)
{
	// We will determine which players have vision of this space.
	Vision sees = Vision::none();

	// Get the players that have vision of this space.
	sees.add(_results[index.ix()]);

	// We reveal all tiles to all visionaries.
	for (const Player& visionary : _info._visionaries)
	{
		sees.add(visionary);
	}

	// Did the vision of the space change?
	if (sees == _board.vision(index)) return;

	// Get a description of this cell.
	Descriptor desc = Descriptor::cell(index.pos());

	// Some players might have lost vision of this cell.
	Vision unsees = _board.vision(index).minus(sees);
	if (!unsees.empty())
	{
		// When vision is lost, the tile is still visible but no longer up-to-date.
		_changeset.push(Change(Change::Type::OBSCURE, desc), unsees);

		// All units are forgotten because units tend to move a lot.
		for (Descriptor::Type slot :
				{Descriptor::Type::GROUND, Descriptor::Type::AIR, Descriptor::Type::BYPASS})
		{
			// Is there a unit here?
			const UnitToken& unit = _board.unit(index, slot);
			if (!unit) continue;

			// If so, the players that have just lost vision should forget about it.
			Descriptor unitdesc(slot, index.pos());
			_changeset.push(Change(Change::Type::EXITED, unitdesc), unsees);
		}
	}

	// Some players might have gained vision of this cell.
	Vision newsees = sees.minus(_board.vision(index));
	if (!newsees.empty())
	{
		// Because the Automaton does not remember exactly what a player currently sees,
		// we have to update all the contents of the cell: the tile...
		// ... all of the markers and counters...
		_changeset.push(Change(Change::Type::REVEAL, desc,
			_board.tile(index), _board.snow(index),
			_board.frostbite(index), _board.firestorm(index),
			_board.bonedrought(index), _board.death(index),
			_board.gas(index), _board.radiation(index),
			_board.temperature(index), _board.humidity(index),
			_board.chaos(index)),
			newsees);
		// ... and all of the units.
		for (Descriptor::Type slot :
				{Descriptor::Type::GROUND, Descriptor::Type::AIR, Descriptor::Type::BYPASS})
		{
			// Is there a unit here?
			const UnitToken& unit = _board.unit(index, slot);
			if (!unit) continue;

			// If so, introduce it to the players that have just gained vision.
			Descriptor unitdesc(slot, index.pos());
			_changeset.push(Change(Change::Type::ENTERED, unitdesc, unit), newsees);
		}
	}

	// Actually update the vision of the space.
	Change update(Change::Type::VISION, desc, sees);
	_board.enact(update);
	// No one sees this meta-change and it is not recorded.
}

void InitialVisionTransition::reduce(Cell index)
{
	// We will determine which players have vision of this space.
	Vision sees = Vision::none();

	// Get the players that have vision of this space.
	sees.add(_results[index.ix()]);

	// We reveal all tiles to all visionaries.
	for (const Player& visionary : _info._visionaries)
	{
		sees.add(visionary);
	}

	// Get a description of this cell.
	Descriptor desc = Descriptor::cell(index.pos());

	// Some players might have vision of this cell.
	Vision newsees = sees;
	if (!newsees.empty())
	{
		// We have to reveal all the contents of the cell: the tile...
		// ... all of the markers and counters...
		_changeset.push(Change(Change::Type::REVEAL, desc,
			_board.tile(index), _board.snow(index),
			_board.frostbite(index), _board.firestorm(index),
			_board.bonedrought(index), _board.death(index),
			_board.gas(index), _board.radiation(index),
			_board.temperature(index), _board.humidity(index),
			_board.chaos(index)),
			newsees);
		// ... and all of the units.
		for (Descriptor::Type slot :
				{Descriptor::Type::GROUND, Descriptor::Type::AIR, Descriptor::Type::BYPASS})
		{
			// Is there a unit here?
			const UnitToken& unit = _board.unit(index, slot);
			if (!unit) continue;

			// If so, introduce it to the players that have just gained vision.
			Descriptor unitdesc(slot, index.pos());
			_changeset.push(Change(Change::Type::ENTERED, unitdesc, unit), newsees);
		}
	}

	// Some players might not have vision of this cell, but still need to know
	// the starting tile and the starting values of markers and counters.
	Vision unsees = Vision::all(_info._players).minus(sees);
	if (!unsees.empty())
	{
		// We have to reveal some of the contents of the cell: the tile...
		// ... all of the markers and counters...
		_changeset.push(Change(Change::Type::REVEAL, desc,
			_board.tile(index), _board.snow(index),
			_board.frostbite(index), _board.firestorm(index),
			_board.bonedrought(index), _board.death(index),
			_board.gas(index), _board.radiation(index),
			_board.temperature(index), _board.humidity(index),
			_board.chaos(index)),
			unsees);
		// ... but none of the units.

		// The tile is still visible but no longer up-to-date.
		_changeset.push(Change(Change::Type::OBSCURE, desc), unsees);
	}

	// Actually update the vision of the space.
	Change update(Change::Type::VISION, desc, sees);
	_board.enact(update);
	// No one sees this meta-change and it is not recorded.
}

void RejoinVisionTransition::reduce(Cell index)
{
	// We will determine which players have vision of this space.
	Vision sees = Vision::none();

	// Get the players that have vision of this space.
	sees.add(_results[index.ix()]);

	// We reveal all tiles to all visionaries.
	for (const Player& visionary : _info._visionaries)
	{
		sees.add(visionary);
	}

	// Get a description of this cell.
	Descriptor desc = Descriptor::cell(index.pos());

	// Some players might have vision of this cell.
	Vision newsees = sees;
	if (!newsees.empty())
	{
		// We have to reveal all the contents of the cell: the tile...
		// ... all of the markers and counters...
		_changeset.push(Change(Change::Type::REVEAL, desc,
			_board.tile(index), _board.snow(index),
			_board.frostbite(index), _board.firestorm(index),
			_board.bonedrought(index), _board.death(index),
			_board.gas(index), _board.radiation(index),
			_board.temperature(index), _board.humidity(index),
			_board.chaos(index)),
			newsees);
		// ... and all of the units.
		for (Descriptor::Type slot :
				{Descriptor::Type::GROUND, Descriptor::Type::AIR, Descriptor::Type::BYPASS})
		{
			// Is there a unit here?
			const UnitToken& unit = _board.unit(index, slot);
			if (!unit) continue;

			// If so, introduce it to the players that have just gained vision.
			Descriptor unitdesc(slot, index.pos());
			_changeset.push(Change(Change::Type::ENTERED, unitdesc, unit), newsees);
		}
	}

	// Some players might not have vision of this cell, but still need to know
	// the starting tile and the starting values of markers and counters.
	Vision unsees = Vision::all(_info._players).minus(sees);
	if (!unsees.empty())
	{
		// We have to reveal some of the contents of the cell: the tile...
		// ... all of the markers and counters...
		_changeset.push(Change(Change::Type::REVEAL, desc,
			_board.tile(index), _board.snow(index),
			_board.frostbite(index), _board.firestorm(index),
			_board.bonedrought(index), _board.death(index),
			_board.gas(index), _board.radiation(index),
			_board.temperature(index), _board.humidity(index),
			_board.chaos(index)),
			unsees);
		// ... but none of the units.

		// The tile is still visible but no longer up-to-date.
		_changeset.push(Change(Change::Type::OBSCURE, desc), unsees);
	}

	// This is a rejoin; do not update the vision of the space.
}

template class VisionTMRI<VisionTransition>;
template class VisionTMRI<InitialVisionTransition>;
template class VisionTMRI<RejoinVisionTransition>;
