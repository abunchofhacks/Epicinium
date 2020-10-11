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


class Loop
{
protected:
	static float _rawTempo;
	static float _tempo;
	static float _delta;
	static float _theta;

	static float _tempo_multiplier;
	static float _hitstop;
	static float _hitstop_amount;

public:
	static float rawTempo() { return _rawTempo; }
	static float tempo() { return _tempo; }
	static float delta() { return _delta; }
	static float theta() { return _theta; }

	static void changeTempo(float multiplier)
	{
		_tempo_multiplier = multiplier;
	}

	static void hitstop(float amount)
	{
		_hitstop = std::max(_hitstop, amount);
	}

	static float hitstopAmount()
	{
		return _hitstop_amount;
	}
};
