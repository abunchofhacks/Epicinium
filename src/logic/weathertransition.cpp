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
#include "weathertransition.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "board.hpp"
#include "changeset.hpp"
#include "cycle.hpp"
#include "randomizer.hpp"


WeatherTransition::WeatherTransition(const Bible& bible, Board& board,
		ChangeSet& changeset,
		const Season& season) :
	_bible(bible),
	_board(board),
	_changeset(changeset),
	_season(season),
	_totalchaos(0),
	_emissionbased(bible.emissionDivisor() > 0
		&& !_bible.counterBasedWeather()),
	_emissions(0),
	_gain(_board.end().ix(), 0),
	_loss(_board.end().ix(), 0)
{
	for (Cell index : _board)
	{
		_totalchaos += _board.chaos(index);
	}

	// Randomly pick a number of spaces to receive aridification.
	if (_bible.quantitativeChaos() && _board.mass() > 0
		&& _bible.randomizedAridification())
	{
		Randomizer<Cell> humidspaces;
		Randomizer<Cell> otherspaces;

		for (Cell index : _board)
		{
			if (_board.humidity(index) > 0)
			{
				if (_bible.planeBasedAridification())
				{
					if (_bible.tilePlane(_board.tile(index).type))
					{
						humidspaces.push(index);
					}
					else
					{
						otherspaces.push(index);
					}
				}
				else
				{
					humidspaces.push(index);
				}
			}
		}

		if (_bible.chaosMinAridification(_season) > 0)
		{
			int count = _bible.aridificationCount() * _board.mass()
				* (_totalchaos
					/ (_bible.chaosMinAridification(_season)
						* _board.mass()));

			for (int i = 0; i < count; i++)
			{
				if (humidspaces)
				{
					_randomizedAridification.emplace_back(humidspaces.pop());
				}
				else if (otherspaces)
				{
					_randomizedAridification.emplace_back(otherspaces.pop());
				}
				else break;
			}
		}
	}
}

void WeatherTransition::execute()
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

// Some effects might cause a space to gain humidity; the highest gain is used.
// Others might cause a space to lose humidity; the deepest loss is used.
void WeatherTransition::map(Cell index)
{
	// If the chaos is high enough,
	// aridification lowers the humidity.
	bool aridification = false;
	if (_bible.randomizedAridification())
	{
		aridification = (_bible.chaosMinAridification(_season) >= 0
			&& std::find(_randomizedAridification.begin(),
					_randomizedAridification.end(), index)
				!= _randomizedAridification.end());
	}
	else if (_bible.counterBasedWeather())
	{
		aridification = (_board.chaos(index)
			>= _bible.chaosMinAridification(_season));
	}
	else
	{
		aridification = (_board.temperature(index)
			>= _bible.temperatureMinAridification(_season));
	}

	if (aridification)
	{
		// Aridification lowers the humidity of an entire region.
		for (Cell to : _board.area(index, 0, _bible.aridificationRange()))
		{
			// The drop depends on whether the humidity is
			// already below the degradation threshold.
			uint8_t loss;
			if (_board.humidity(to)
				<= _bible.humidityMaxDegradation(_season))
			{
				loss = _bible.aridificationAmountDegraded();
			}
			else
			{
				loss = _bible.aridificationAmountHumid();
			}
			_loss[to.ix()] = std::max(_loss[to.ix()], loss);
		}
	}

	const TileToken& tiletoken = _board.tile(index);

	// Industry tiles cause global warming through emissions (old style).
	if (_emissionbased)
	{
		_emissions += _bible.tileEmission(tiletoken.type);
	}

	// Industry tiles cause ground pollution (i.e. local humidity loss).
	int pollution = _bible.tilePollutionAmount(tiletoken.type);
	if (pollution &&
		(_season == Season::AUTUMN || !_bible.groundPollutionOnlyInAutumn()))
	{
		// To simulate ground pollution, industry tiles reduce the humidity
		// around them.
		int radius = _bible.tilePollutionRadius(tiletoken.type);
		for (Cell to : _board.area(index, 0, radius))
		{
			// Ground pollution causes humidity to drop just below the threshold
			// where grass turns to dirt, but not enough to turn dirt to desert.
			int tohum = _board.humidity(to);
			int target = _bible.humidityMaxDegradation(_season);
			if (!_bible.counterBasedWeather())
			{
				target = target - 5 + (tohum % 5);
			}
			if (tohum > target)
			{
				uint8_t loss = std::min(tohum - target, pollution);
				_loss[to.ix()] = std::max(_loss[to.ix()], loss);
			}
		}
	}
}

void WeatherTransition::reduce(Cell index)
{
	// Calculate the humidity change.
	int8_t humgain = _gain[index.ix()] - _loss[index.ix()];

	// Make sure the humidity does not go out of bounds.
	int8_t targethum = std::max((int) _bible.humidityMin(),
		std::min((int) _board.humidity(index) + humgain,
			(int) _bible.humidityMax()));
	humgain = targethum - _board.humidity(index);

	// Change the humidity unless trivial.
	if (humgain)
	{
		Change change(Change::Type::HUMIDITY,
			Descriptor::cell(index.pos()));
		change.xHumidity(humgain);
		_board.enact(change);
		_changeset.push(change, _board.vision(index));
	}

	if (_bible.counterBasedWeather()) return;

	// Temperature gain is a combination of seasonal swing and global warming.
	int tempgain = _bible.seasonTemperatureSwing(_season);

	// New chaos-based global warming.
	if (_board.chaos(index))
	{
		int chaos = _board.chaos(index);
		tempgain += chaos * _bible.seasonGlobalWarmingFactor(_season);
	}

	// Old emission-based global warming.
	if (_emissionbased)
	{
		int globalwarming = _emissions / _bible.emissionDivisor();
		tempgain += globalwarming * _bible.seasonGlobalWarmingFactor(_season);
	}

	// Make sure the temperature does not go out of bounds.
	int8_t targettemp = std::max((int) _bible.temperatureMin(),
		std::min((int) _board.temperature(index) + tempgain,
			(int) _bible.temperatureMax()));
	tempgain =  targettemp - _board.temperature(index);

	// Change the temperature unless trivial.
	if (tempgain)
	{
		Change change(Change::Type::TEMPERATURE,
			Descriptor::cell(index.pos()));
		change.xTemperature(tempgain);
		_board.enact(change);
		_changeset.push(change, _board.vision(index));
	}
}
