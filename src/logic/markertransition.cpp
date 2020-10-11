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
#include "markertransition.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "board.hpp"
#include "changeset.hpp"
#include "randomizer.hpp"


namespace MarkerFlag
{
	enum
	{
		SNOW = 0x01,
		FROSTBITE = 0x02,
		FIRESTORM = 0x04,
		BONEDROUGHT = 0x08,
		DEATH = 0x10,
	};
}

MarkerTransition::MarkerTransition(const Bible& bible, Board& board,
		ChangeSet& changeset, const Season& season) :
	_bible(bible),
	_board(board),
	_changeset(changeset),
	_season(season),
	_totalchaos(0),
	_results(_board.end().ix(), 0)
{
	for (Cell index : _board)
	{
		_totalchaos += _board.chaos(index);
	}

	// Randomly pick a number of spaces to receive firestorm and death.
	if (_bible.quantitativeChaos() && _board.mass() > 0
		&& (_bible.randomizedFirestorm() || _bible.cumulativeDeath()))
	{
		Randomizer<Cell> flammables;
		Randomizer<Cell> nonflammables;
		Randomizer<Cell> controllables;
		Randomizer<Cell> uncontrollables;

		for (Cell index : _board)
		{
			if (_bible.tileControllable(_board.tile(index).type))
			{
				controllables.push(index);
			}
			else
			{
				uncontrollables.push(index);
			}
		}

		if (_bible.flammableBasedFirestorm())
		{
			for (Cell index : _board)
			{
				if (_bible.tileFlammable(_board.tile(index).type))
				{
					flammables.push(index);
				}
				else
				{
					nonflammables.push(index);
				}
			}
		}
		else
		{
			flammables = uncontrollables;
			nonflammables = controllables;
		}

		if (_bible.chaosMinFirestorm(_season) > 0)
		{
			int count = _bible.firestormCount() * _board.mass()
				* (_totalchaos
					/ (_bible.chaosMinFirestorm(_season) * _board.mass()));

			for (int i = 0; i < count; i++)
			{
				if (flammables)
				{
					_randomizedFirestorm.emplace_back(flammables.pop());
				}
				else if (nonflammables)
				{
					_randomizedFirestorm.emplace_back(nonflammables.pop());
				}
				else break;
			}
		}

		if (_bible.chaosMinDeath(_season) > 0)
		{
			int count = _bible.deathCount() * _board.mass()
				* (_totalchaos
					/ (_bible.chaosMinDeath(_season) * _board.mass()));

			for (int i = 0; i < count; i++)
			{
				if (controllables)
				{
					_randomizedDeath.emplace_back(controllables.pop());
				}
				else if (uncontrollables)
				{
					_randomizedDeath.emplace_back(uncontrollables.pop());
				}
				else break;
			}
		}
	}
}

void MarkerTransition::execute()
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

void MarkerTransition::map(Cell index)
{
	bool snow;
	bool frostbite;
	bool firestorm;
	bool bonedrought;
	bool death;

	if (_bible.counterBasedWeather())
	{
		TileType tiletype = _board.tile(index).type;
		int hum = _board.humidity(index);
		int chaos = _board.chaos(index);
		int stacks = _board.tile(index).stacks;
		if (_bible.quantitativeChaos() && _board.mass() > 0)
		{
			chaos = _totalchaos / _board.mass();
		}

		snow = (_bible.chaosMinSnow(_season) >= 0
			&& hum >= _bible.humidityMinSnow(_season)
			&& chaos >= _bible.chaosMinSnow(_season));
		frostbite = (_bible.chaosMinFrostbite(_season) >= 0
			&& hum >= _bible.humidityMinFrostbite(_season)
			&& chaos >= _bible.chaosMinFrostbite(_season)
			&& ((_bible.planeBasedFrostbite())
				? (_bible.tilePlane(tiletype))
				: (_bible.emptyBasedFrostbite())
				? (stacks == 0)
				: (_bible.stackBasedFrostbite())
				? ((stacks + 1) * _bible.chaosMinFrostbite(_season) <= chaos)
				: true));
		firestorm = (_bible.chaosMinFirestorm(_season) >= 0
			&& !_bible.randomizedFirestorm()
			&& hum <= _bible.humidityMaxFirestorm(_season)
			&& chaos >= _bible.chaosMinFirestorm(_season));
		bonedrought = (_bible.tileDesert(tiletype)
			&& _bible.chaosMinBonedrought(_season) >= 0
			&& hum <= _bible.humidityMaxBonedrought(_season)
			&& chaos >= _bible.chaosMinBonedrought(_season));
		bonedrought |= (_bible.tileStone(tiletype)
			&& _bible.chaosMinStonedrought(_season) >= 0
			&& hum <= _bible.humidityMaxStonedrought(_season)
			&& chaos >= _bible.chaosMinStonedrought(_season));
		death = (_bible.chaosMinDeath(_season) >= 0
			&& !_bible.cumulativeDeath()
			&& chaos >= _bible.chaosMinDeath(_season));

		if (_bible.cumulativeDeath()
			&& (_board.death(index)
				|| std::find(_randomizedDeath.begin(),
						_randomizedDeath.end(), index)
					!= _randomizedDeath.end()))
		{
			death = true;
		}

		if (_bible.randomizedFirestorm()
			&& std::find(_randomizedFirestorm.begin(),
					_randomizedFirestorm.end(), index)
				!= _randomizedFirestorm.end())
		{
			firestorm = true;
		}
	}
	else
	{
		int temp = _board.temperature(index);
		int hum = _board.humidity(index);
		int rad = _board.radiation(index);

		// Snow can fall if it freezes and the humidity is high enough.
		snow = hum >= _bible.humidityMinWet(_season)
			&& temp <= _bible.temperatureMaxSnow(_season);
		// Frostbite occurs if the temperature is critically low.
		frostbite = temp <= _bible.temperatureMaxFrostbite(_season);
		// Firestorm occurs if the temperature is critically high.
		firestorm = temp >= _bible.temperatureMinFirestorm(_season);
		// Bonedrought occurs if the humidity is critically low.
		bonedrought = hum <= _bible.humidityMaxBonedrought(_season);
		// Death occurs at extreme temperatures, at extremely low humidity and at extreme radiation.
		death = hum <= _bible.humidityMaxDeath(_season)
			|| temp <= _bible.temperatureMaxColdDeath(_season)
			|| temp >= _bible.temperatureMinHotDeath(_season)
			|| rad >= _bible.radiationThresholdDeath();
	}

	uint8_t result = 0;
	if (snow) result |= MarkerFlag::SNOW;
	if (frostbite) result |= MarkerFlag::FROSTBITE;
	if (firestorm) result |= MarkerFlag::FIRESTORM;
	if (bonedrought) result |= MarkerFlag::BONEDROUGHT;
	if (death) result |= MarkerFlag::DEATH;
	_results[index.ix()] = result;
}

void MarkerTransition::reduce(Cell index)
{
	uint8_t result = _results[index.ix()];

	{
		bool snow = (result & MarkerFlag::SNOW);
		if (snow != _board.snow(index))
		{
			Change change(Change::Type::SNOW,
				Descriptor::cell(index.pos()));
			change.xSnow(snow);
			_board.enact(change);
			_changeset.push(change, _board.vision(index));
		}
	}

	{
		bool frostbite = (result & MarkerFlag::FROSTBITE);
		if (frostbite != _board.frostbite(index))
		{
			Change change(Change::Type::FROSTBITE,
				Descriptor::cell(index.pos()));
			change.xFrostbite(frostbite);
			_board.enact(change);
			_changeset.push(change, _board.vision(index));
		}
	}

	{
		bool firestorm = (result & MarkerFlag::FIRESTORM);
		if (firestorm != _board.firestorm(index))
		{
			Change change(Change::Type::FIRESTORM,
				Descriptor::cell(index.pos()));
			change.xFirestorm(firestorm);
			_board.enact(change);
			_changeset.push(change, _board.vision(index));
		}
	}

	{
		bool bonedrought = (result & MarkerFlag::BONEDROUGHT);
		if (bonedrought != _board.bonedrought(index))
		{
			Change change(Change::Type::BONEDROUGHT,
				Descriptor::cell(index.pos()));
			change.xBonedrought(bonedrought);
			_board.enact(change);
			_changeset.push(change, _board.vision(index));
		}
	}

	{
		bool death = (result & MarkerFlag::DEATH);
		if (death != _board.death(index))
		{
			Change change(Change::Type::DEATH,
				Descriptor::cell(index.pos()));
			change.xDeath(death);
			_board.enact(change);
			_changeset.push(change, _board.vision(index));
		}
	}
}
