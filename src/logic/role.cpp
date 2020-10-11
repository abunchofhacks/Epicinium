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
#include "role.hpp"
#include "source.hpp"

#include "parseerror.hpp"
#include "player.hpp"


Role parseRole(const std::string& str)
{
	if      (str == "none")     return Role::NONE;
	if      (str == "player")   return Role::PLAYER;
	else if (str == "observer") return Role::OBSERVER;
	else throw ParseError("Unknown role '" + str + "'");
}

const char* stringify(const Role& role)
{
	switch (role)
	{
		case Role::NONE:     return "none";
		case Role::PLAYER:   return "player";
		case Role::OBSERVER: return "observer";
	}
	return "error";
}

Player getVisionLevel(const Role& role)
{
	switch (role)
	{
		case Role::OBSERVER: return Player::OBSERVER;
		default:             return Player::BLIND;
	}
}

std::ostream& operator<<(std::ostream& os, const Role& role)
{
	return os << stringify(role);
}
