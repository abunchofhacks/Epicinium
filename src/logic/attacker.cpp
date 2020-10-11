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
#include "attacker.hpp"
#include "source.hpp"


Attacker::Attacker(const TypeNamer& namer, const Json::Value& json) :
	unittype(::parseUnitType(namer, json["unittype"].asString())),
	position(json["position"])
{}

Bombarder::Bombarder(const TypeNamer& namer, const Json::Value& json) :
	unittype(::parseUnitType(namer, json["unittype"].asString()))
{}

std::ostream& operator<<(std::ostream& os, const Attacker& attacker)
{
	return os << "{"
		"\"unittype\":\"" << attacker.unittype << "\""
		","
		"\"position\":" << attacker.position << ""
		"}";
}

std::ostream& operator<<(std::ostream& os, const Bombarder& bombarder)
{
	return os << "{"
		"\"unittype\":\"" << bombarder.unittype << "\""
		"}";
}
