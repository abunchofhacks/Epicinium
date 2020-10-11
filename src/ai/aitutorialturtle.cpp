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
#include "aitutorialturtle.hpp"
#include "source.hpp"

#include "difficulty.hpp"


std::string AITutorialTurtle::ainame() const
{
	return "TutorialTurtle";
}

std::string AITutorialTurtle::authors() const
{
	return "Sander in 't Veld";
}

int AITutorialTurtle::maxOrders()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 2;
		case Difficulty::MEDIUM: return 5;
		case Difficulty::HARD: return 5;
	}
	return 0;
}

int AITutorialTurtle::maxMilitaryOrders()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 1;
		case Difficulty::MEDIUM: return 3;
		case Difficulty::HARD: return 4;
	}
	return 0;
}

int AITutorialTurtle::maxMilitaryUnits()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 1;
		case Difficulty::MEDIUM: return 6;
		case Difficulty::HARD: return 10;
	}
	return 0;
}

int AITutorialTurtle::maxDefenseUnits()
{
	// The tutorialturtle never declares attackers.
	return 1000;
}

int AITutorialTurtle::maxCaptures()
{
	// The tutorialturtle never captures.
	return 0;
}

int AITutorialTurtle::maxBombardments()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 0;
		case Difficulty::MEDIUM: return 1;
		case Difficulty::HARD: return 1;
	}
	return 0;
}
