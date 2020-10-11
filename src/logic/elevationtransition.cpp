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
#include "elevationtransition.hpp"
#include "source.hpp"

#include "aim.hpp"
#include "bible.hpp"
#include "board.hpp"
#include "changeset.hpp"
#include "cell.hpp"


ElevationTransition::ElevationTransition(const Bible& bible, Board& board,
		ChangeSet& changeset) :
	_bible(bible),
	_board(board),
	_changeset(changeset),
	_oceans(bible, board),
	_elev(_board.end().ix(), 0),
	_coast(_board.end().ix(), 0)
{
	_oceans.execute();
}

void ElevationTransition::execute()
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

void ElevationTransition::map(Cell index)
{
	int range = _bible.tempGenGainRange();
	if (range < 0) return;

	if (_oceans.ocean(index))
	{
		for (Cell to : _board.area(index, 0, range))
		{
			size_t dist = Aim(index.pos(), to.pos()).sumofsquares();
			uint8_t gain = _bible.tempGenOceanGain(dist);
			_coast[index.ix()] = std::max(_coast[index.ix()], gain);
		}
	}
	else if (_bible.tileMountain(_board.tile(index).type))
	{
		for (Cell to : _board.area(index, 0, range))
		{
			size_t dist = Aim(index.pos(), to.pos()).sumofsquares();
			uint8_t gain = _bible.tempGenMountainGain(dist);
			_elev[index.ix()] = std::max(_elev[index.ix()], gain);
		}
	}
}

void ElevationTransition::reduce(Cell index)
{
	int8_t elev = _elev[index.ix()];
	int8_t coast = _coast[index.ix()];

	int target = (int) _bible.tempGenDefault() + elev + coast + rand() % 4 - 1;
	int8_t temperature = std::max((int) _bible.temperatureMin(),
		std::min(target, (int) _bible.temperatureMax()));

	int8_t diff = temperature - _board.temperature(index);
	if (diff)
	{
		Change change(Change::Type::TEMPERATURE, Descriptor::cell(index.pos()));
		change.xTemperature(diff);
		_board.enact(change);
		_changeset.push(change, _board.vision(index));
	}
}
