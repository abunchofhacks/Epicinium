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

#include "animator.hpp"

class Commander;
class Settings;
enum class UnitType : uint8_t;
enum class TileType : uint8_t;


class Tutorial : public Animator
{
private:
	enum class State;

public:
	Tutorial(Commander& commander, const Settings& settings);

private:
	Commander& _commander;

	const Settings& _settings;

	const UnitType _settlertype;
	const TileType _industrytype;
	const TileType _barrackstype;

	State _state;
	int _clickCount;

	bool isEnglish() const;
	void prompt(State newstate);

public:
	void load();
	void update();
};
