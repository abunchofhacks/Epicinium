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
#include "chaostransition.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "board.hpp"
#include "changeset.hpp"


ChaosTransition::ChaosTransition(const Bible& bible, Board& board,
		ChangeSet& changeset) :
	_bible(bible),
	_board(board),
	_changeset(changeset),
	_remainder(0),
	_randompositions(board.rows() * board.cols())
{
	fillRandomPositions();
}

void ChaosTransition::fillRandomPositions()
{
	for (Cell index : _board)
	{
		if (_board.chaos(index) < _bible.chaosMax())
		{
			_randompositions.push(index);
		}
	}
}

void ChaosTransition::execute()
{
	int oldtotal = 0;
	for (Cell index : _board)
	{
		oldtotal += _board.chaos(index);
	}

	for (Cell index : _board)
	{
		if (!_randompositions)
		{
			fillRandomPositions();
			if (!_randompositions) break;
		}

		map(index);
	}

	int newtotal = 0;
	for (Cell index : _board)
	{
		newtotal += _board.chaos(index);
	}

	if (_bible.quantitativeChaos() && _board.mass() > 0
		&& _bible.chaosThreshold() > 0)
	{
		int oldlevel = oldtotal / (_board.mass() * _bible.chaosThreshold());
		int newlevel = newtotal / (_board.mass() * _bible.chaosThreshold());
		if (newlevel > oldlevel)
		{
			// Announce that a new chaos threshold has been crossed.
			Change report(Change::Type::CHAOSREPORT);
			report.xLevel(newlevel);
			_changeset.push(report, Vision::all(_board.players()));
		}
	}
}

void ChaosTransition::map(Cell index)
{
	// Get the tiletoken.
	const TileToken& tiletoken = _board.tile(index);

	// All powered tiles cause global warming through chaos.
	if (_bible.emissionDivisor() <= 1)
	{
		int chaosgain = _bible.tileEmission(tiletoken.type);
		distribute(chaosgain);
	}
	else
	{
		int value = _bible.tileEmission(tiletoken.type) + _remainder;
		int chaosgain = value / _bible.emissionDivisor();
		_remainder = value % _bible.emissionDivisor();
		distribute(chaosgain);
	}
}

void ChaosTransition::distribute(int chaosgain)
{
	for (int i = 0; i < chaosgain; i++)
	{
		// Get a random space on the board.
		if (!_randompositions) fillRandomPositions();
		if (!_randompositions) return;
		Cell target = _randompositions.pop();

		// Forest tiles are protected from the random chaos.
		if (_bible.tileChaosProtection(_board.tile(target).type)
			&& (_bible.forestChaosProtectionPermanent()
				|| _board.chaos(target) <= 0))
		{
			// This instance of random chaos is negated.
		}
		else if (_bible.quantitativeChaos())
		{
			// Pick a random space until we find a space without chaos,
			// or until we run out of spaces.
			do
			{
				if (!_randompositions) fillRandomPositions();
				if (!_randompositions) return;
				target = _randompositions.pop();
			}
			while (_board.chaos(target) >= _bible.chaosMax());

			// Increase the chaos of the tile we picked.
			enact(target);
		}
		else
		{
			// Does any neighbour have strictly lower chaos?
			Randomizer<Cell> randomizer(8);
			for (Cell other : _board.area(target, 1, 2))
			{
				if (_board.chaos(other) < _board.chaos(target))
				{
					randomizer.push(other);
				}
			}

			// Increase the chaos of either itself or its neighbour.
			if (randomizer) enact(randomizer.pop());
			else enact(target);
		}
	}
}

void ChaosTransition::enact(Cell index)
{
	// Make sure the chaos does not go out of bounds.
	int8_t targetchaos = std::max((int) _bible.chaosMin(),
			std::min((int) _board.chaos(index) + 1,
				(int) _bible.chaosMax()));
	int8_t chaosgain = targetchaos - _board.chaos(index);

	// Change the chaos unless trivial.
	if (chaosgain)
	{
		Change change(Change::Type::CHAOS, Descriptor::cell(index.pos()));
		change.xChaos(chaosgain);
		_board.enact(change);
		_changeset.push(change, _board.vision(index));
	}
}
