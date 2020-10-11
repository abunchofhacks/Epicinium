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
#include "gastransition.hpp"
#include "source.hpp"

#include "board.hpp"
#include "bible.hpp"
#include "changeset.hpp"
#include "cell.hpp"
#include "cycle.hpp"


GasTransition::GasTransition(const Bible& bible, Board& board,
		ChangeSet& changeset) :
	_bible(bible),
	_board(board),
	_changeset(changeset),
	_results(_board.end().ix(), 0)
{}

void GasTransition::execute()
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

void GasTransition::map(Cell index)
{
	int8_t level = _board.gas(index);

	if (!level) return;

	put(index, level - 1);
	if (level >= 2)
	{
		// We do not use Area here because we want to put() gas on edges.
		for (Cell neighbor : {
				index + Move::E,
				index + Move::E + Move::S,
				index + Move::S,
				index + Move::S + Move::W,
				index + Move::W,
				index + Move::W + Move::N,
				index + Move::N,
				index + Move::N + Move::E,
			})
		{
			put(neighbor, level - 1);
		}
	}
}

void GasTransition::reduce(Cell index)
{
	if (!_results[index.ix()]) return;

	// Inside _results, levels are increased by 1 so that 0 means undefined.
	int8_t level = _results[index.ix()] - 1;

	int8_t diff = level - _board.gas(index);
	if (diff)
	{
		Change change(Change::Type::GAS,
			Descriptor::cell(index.pos()));
		change.xGas(diff);
		_board.enact(change);
		_changeset.push(change, _board.vision(index));
	}
	else
	{
		// Gas stayed the same, so humidity still drops.
	}

	int8_t hum = _board.humidity(index);

	// Gas causes humidity to drop just below the threshold where grass
	// turns to dirt, but not enough to turn dirt to desert.
	int8_t target = _bible.humidityMaxDegradation(Season::SPRING);
	if (!_bible.counterBasedWeather())
	{
		target = target - 5 + (hum % 5);
	}
	if (_board.gas(index) && hum > target)
	{
		// Note that the maximum of negative values is the least extreme value.
		int8_t humdiff = std::max(target - hum,
			- _bible.gasPollutionAmount());

		// Make sure the humidity does not go out of bounds.
		int8_t targethum = std::max((int) _bible.humidityMin(),
			std::min((int) hum + humdiff, (int) _bible.humidityMax()));
		humdiff = targethum - hum;

		// Change the humidity unless trivial.
		if (humdiff)
		{
			Change change(Change::Type::HUMIDITY,
				Descriptor::cell(index.pos()));
			change.xHumidity(humdiff);
			_board.enact(change);
			_changeset.push(change, _board.vision(index));
		}
	}
}

void GasTransition::put(Cell index, int8_t level)
{
	DEBUG_ASSERT(level >= 0);

	// Inside _results, levels are increased by 1 so that 0 means undefined.
	uint8_t result = level + 1;
	_results[index.ix()] = std::max(_results[index.ix()], result);
}
