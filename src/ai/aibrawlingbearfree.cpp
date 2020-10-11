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
#include "aibrawlingbearfree.hpp"
#include "source.hpp"

#include "difficulty.hpp"


std::string AIBrawlingBearFree::ainame() const
{
	return "BrawlingBear";
}

std::string AIBrawlingBearFree::authors() const
{
	return "Sander in 't Veld";
}

int AIBrawlingBearFree::maxOrders()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 2;
		case Difficulty::MEDIUM: return 3;
		case Difficulty::HARD: return 5;
	}
	return 0;
}

int AIBrawlingBearFree::maxMilitaryOrders()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 2;
		case Difficulty::MEDIUM: return 3;
		case Difficulty::HARD: return 5;
	}
	return 0;
}

int AIBrawlingBearFree::maxMilitaryUnits()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 5;
		case Difficulty::MEDIUM: return 15;
		case Difficulty::HARD: return 15;
	}
	return 0;
}

int AIBrawlingBearFree::maxDefenseUnits()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 1000;
		case Difficulty::EASY: return 4;
		case Difficulty::MEDIUM: return 4;
		case Difficulty::HARD: return 4;
	}
	return 0;
}

int AIBrawlingBearFree::maxBombardments()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 1;
		case Difficulty::MEDIUM: return 1;
		case Difficulty::HARD: return 1;
	}
	return 0;
}

int AIBrawlingBearFree::minDefenseMoney()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return 0;
		case Difficulty::EASY:   return 0;
		case Difficulty::MEDIUM: return 0;
		case Difficulty::HARD:   return 0;
	}
	return 0;
}

int AIBrawlingBearFree::minSettlersMoney()
{
	switch (_difficulty)
	{
		// It is 1 > 0 because we do not want it to produce settlers.
		case Difficulty::NONE:   return 0;
		case Difficulty::EASY:   return 1;
		case Difficulty::MEDIUM: return 1;
		case Difficulty::HARD:   return 1;
	}
	return 0;
}

int AIBrawlingBearFree::minBarracksMoney()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return 0;
		case Difficulty::EASY:   return 0;
		case Difficulty::MEDIUM: return 0;
		case Difficulty::HARD:   return 0;
	}
	return 0;
}

int AIBrawlingBearFree::minDefenseUpgradeMoney()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return 0;
		case Difficulty::EASY:   return 0;
		case Difficulty::MEDIUM: return 0;
		case Difficulty::HARD:   return 0;
	}
	return 0;
}

bool AIBrawlingBearFree::canUseCombatAbilities()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return false;
		case Difficulty::EASY:   return true;
		case Difficulty::MEDIUM: return true;
		case Difficulty::HARD:   return true;
	}
	return false;
}

bool AIBrawlingBearFree::canBuildBarracks()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return false;
		case Difficulty::EASY:   return true;
		case Difficulty::MEDIUM: return true;
		case Difficulty::HARD:   return true;
	}
	return false;
}

bool AIBrawlingBearFree::canBuildCities()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return false;
		case Difficulty::EASY:   return false;
		case Difficulty::MEDIUM: return false;
		case Difficulty::HARD:   return false;
	}
	return false;
}
