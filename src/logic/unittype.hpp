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

class TypeNamer;


enum class UnitType : uint8_t
{
	NONE = 0,
};

#if NEURALNEWT_20x13_ENABLED
static constexpr size_t UNITTYPE_SIZE = 8;
#else
static constexpr size_t UNITTYPE_SIZE = 32;
#endif

UnitType parseUnitType(const TypeNamer& namer, const std::string& str);

std::ostream& operator<<(std::ostream& os, const UnitType& type);
