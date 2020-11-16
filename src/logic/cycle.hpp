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


enum class Season : uint8_t
{
	SPRING,
	SUMMER,
	AUTUMN,
	WINTER,
};

constexpr size_t SEASON_SIZE = 4;

enum class Daytime : uint8_t
{
	LATE,
	EARLY,
};

constexpr size_t DAYTIME_SIZE = 2;

enum class Phase : uint8_t
{
	GROWTH,
	RESTING,
	PLANNING,
	STAGING,
	ACTION,
	DECAY,
};

constexpr size_t PHASE_SIZE = 6;

Season  parseSeason (const std::string& str);
Daytime parseDaytime(const std::string& str);
Phase   parsePhase  (const std::string& str);

const char* stringify(const Season& season);
const char* stringify(const Daytime& daytime);
const char* stringify(const Phase& phase);

std::ostream& operator<<(std::ostream& os, const Season& value);
std::ostream& operator<<(std::ostream& os, const Daytime& value);
std::ostream& operator<<(std::ostream& os, const Phase& value);

std::string phaseDescription(const Daytime& daytime, const Phase& phase);
