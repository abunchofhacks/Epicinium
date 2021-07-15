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

#include "point.hpp"

enum class Player : uint8_t;
class CameraFocus;


struct AnimationGroup
{
public:
	float delay = 0;
	uint16_t ticks = 0;
	int16_t money = 0;
	bool morale = false;
	bool attacker = false;
	bool multiculti = false;
	bool coldfeet = false;
	Player player;
	int8_t unitslot = 0;
	int8_t fromrow = 0;
	int8_t fromcol = 0;
	int8_t torow = 0;
	int8_t tocol = 0;
	std::weak_ptr<CameraFocus> camerafocus;
	std::vector<uint8_t> indices = {};
	std::vector<Point> placements = {};
	std::vector<std::pair<float, float>> hitstops = {};
	bool skiplighting = false;
};
