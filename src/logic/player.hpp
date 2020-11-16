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


enum class Player : uint8_t
{
	// No player.
	NONE = 0,
	// Player colors.
	RED,
	BLUE,
	YELLOW,
	TEAL,
	BLACK,
	PINK,
	INDIGO,
	PURPLE,
	// Non-player vision types used by the Automaton.
	BLIND,
	OBSERVER,
	// Non-player vision type used by the Board/Level to keep track of its
	// owner's vision.
	SELF,
};

constexpr size_t PLAYER_MAX = 8;
constexpr size_t PLAYER_SIZE = ((size_t) Player::SELF) + 1;

constexpr bool isPlayer(const Player& player)
{
	return (player != Player::NONE && ((size_t) player) <= PLAYER_MAX);
}

std::vector<Player> getPlayers(size_t n);
Player parsePlayer(const std::string& str);
const char* stringify(const Player& player);
std::string colorPlayerName(const Player& player);

std::ostream& operator<<(std::ostream& os, const Player& player);
