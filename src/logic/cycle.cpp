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
#include "cycle.hpp"
#include "source.hpp"

#include "parseerror.hpp"


Season parseSeason(const std::string& str)
{
	if      (str == "spring") return Season::SPRING;
	else if (str == "summer") return Season::SUMMER;
	else if (str == "autumn") return Season::AUTUMN;
	else if (str == "winter") return Season::WINTER;
	else throw ParseError("Unknown season '" + str + "'");
}

Daytime parseDaytime(const std::string& str)
{
	if      (str == "early") return Daytime::EARLY;
	else if (str == "late")  return Daytime::LATE;
	else throw ParseError("Unknown daytime '" + str + "'");
}

Phase parsePhase(const std::string& str)
{
	if      (str == "growth")   return Phase::GROWTH;
	else if (str == "resting")  return Phase::RESTING;
	else if (str == "planning") return Phase::PLANNING;
	else if (str == "staging")  return Phase::STAGING;
	else if (str == "action")   return Phase::ACTION;
	else if (str == "decay")    return Phase::DECAY;
	else throw ParseError("Unknown phase '" + str + "'");
}

const char* stringify(const Season& value)
{
	switch (value)
	{
		case Season::SPRING: return "spring";
		case Season::SUMMER: return "summer";
		case Season::AUTUMN: return "autumn";
		case Season::WINTER: return "winter";
	}
	return "error";
}

const char* stringify(const Daytime& value)
{
	switch (value)
	{
		case Daytime::EARLY: return "early";
		case Daytime::LATE:  return "late";
	}
	return "error";
}

const char* stringify(const Phase& value)
{
	switch (value)
	{
		case Phase::GROWTH:   return "growth";
		case Phase::RESTING:  return "resting";
		case Phase::PLANNING: return "planning";
		case Phase::STAGING:  return "staging";
		case Phase::ACTION:   return "action";
		case Phase::DECAY:    return "decay";
	}
	return "error";
}

std::ostream& operator<<(std::ostream& os, const Season& value)  { return os << stringify(value); }
std::ostream& operator<<(std::ostream& os, const Daytime& value) { return os << stringify(value); }
std::ostream& operator<<(std::ostream& os, const Phase& value)   { return os << stringify(value); }

std::string phaseDescription(const Daytime& daytime, const Phase& phase)
{
	switch (phase)
	{
		case Phase::GROWTH:
		{
			switch (daytime)
			{
				case Daytime::LATE:
				{
					return _(""
					"In the Weather phase,"
					" the season changes and crops can grow."
					" In Spring, trees can grow."
					" In Autumn, global warming worsens."
					"");
				}
				break;

				case Daytime::EARLY:
				{
					return _(""
					"In the Night phase,"
					" building tiles gain power depending on their surroundings"
					" and generate income."
					"");
				}
				break;
			}
		}
		break;

		case Phase::PLANNING:
		{
			return _(""
			"In the Planning phase,"
			" players decide which orders to give to their units and tiles."
			" Orders are executed in the Action phase."
			"");
		}
		break;

		case Phase::ACTION:
		{
			return _(""
			"In the Action phase,"
			" players take turns automatically executing their orders."
			"");
		}
		break;

		case Phase::DECAY:
		{
			return _(""
			"In the Decay phase,"
			" any Frostbite, Firestorm or Gas present on the map deals damage."
			" Gas spreads out or dissipates."
			"");
		}
		break;

		case Phase::RESTING:
		case Phase::STAGING:
		break;
	}

	return "";
}

