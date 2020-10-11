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

#include <map>

#include "order.hpp"

enum class Player : uint8_t;


class PlayerInfo
{
public:
	explicit PlayerInfo(size_t playercount) :
		_playercount(playercount)
	{}

	~PlayerInfo() = default;
	PlayerInfo(const PlayerInfo&) = delete;
	PlayerInfo(PlayerInfo&&) = delete;

	PlayerInfo& operator=(const PlayerInfo&) = delete;
	PlayerInfo& operator=(PlayerInfo&&) = delete;

	size_t _playercount;
	std::vector<Player> _players;
	std::map<Player, int16_t> _money;
	std::map<Player, int8_t> _initiative;
	std::map<Player, std::vector<Order>> _orderlists;
	std::map<Player, bool> _citybound;
	std::map<Player, bool> _defeated;
	std::map<Player, int> _score;
	std::map<Player, int> _award;
	std::vector<Player> _visionaries;
};
