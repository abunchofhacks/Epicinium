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
#include "player.hpp"
#include "source.hpp"

#include "parseerror.hpp"


std::vector<Player> getPlayers(size_t n)
{
	if (n > PLAYER_MAX) throw std::runtime_error("More than eight players not supported");

	std::vector<Player> players;
	for (size_t i = 1; i <= n; i++)
	{
		players.push_back(Player(i));
	}
	return players;
}

Player parsePlayer(const std::string& str)
{
	if      (str == "none")     return Player::NONE;
	else if (str == "nature")   return Player::NONE;
	else if (str == "red")      return Player::RED;
	else if (str == "blue")     return Player::BLUE;
	else if (str == "teal")     return Player::TEAL;
	else if (str == "yellow")   return Player::YELLOW;
	else if (str == "black")    return Player::BLACK;
	else if (str == "pink")     return Player::PINK;
	else if (str == "indigo")   return Player::INDIGO;
	else if (str == "purple")   return Player::PURPLE;
	else if (str == "alice")    return Player::RED;
	else if (str == "bob")      return Player::BLUE;
	else if (str == "carol")    return Player::TEAL;
	else if (str == "dave")     return Player::YELLOW;
	else if (str == "emma")     return Player::PINK;
	else if (str == "frank")    return Player::BLACK;
	else if (str == "blind")    return Player::BLIND;
	else if (str == "observer") return Player::OBSERVER;
	else if (str == "self")     return Player::SELF;
	else throw ParseError("Unknown player '" + str + "'");
}

const char* stringify(const Player& player)
{
	switch (player)
	{
		case Player::NONE:     return "none";
		case Player::RED:      return "red";
		case Player::BLUE:     return "blue";
		case Player::YELLOW:   return "yellow";
		case Player::TEAL:     return "teal";
		case Player::BLACK:    return "black";
		case Player::PINK:     return "pink";
		case Player::INDIGO:   return "indigo";
		case Player::PURPLE:   return "purple";
		case Player::BLIND:    return "blind";
		case Player::OBSERVER: return "observer";
		case Player::SELF:     return "self";
	}
	return "error";
}

const char* colorPlayerName(const Player& player)
{
	switch (player)
	{
		case Player::RED:      return _("Red Player");
		case Player::BLUE:     return _("Blue Player");
		case Player::YELLOW:   return _("Yellow Player");
		case Player::TEAL:     return _("Teal Player");
		case Player::BLACK:    return _("Black Player");
		case Player::PINK:     return _("Pink Player");
		case Player::INDIGO:   return _("Indigo Player");
		case Player::PURPLE:   return _("Purple Player");

		case Player::BLIND:
		case Player::OBSERVER:
		case Player::SELF:
		case Player::NONE:
		break;
	}
	return "error";
}

std::ostream& operator<<(std::ostream& os, const Player& player)
{
	return os << stringify(player);
}
