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

enum class Move : uint8_t;
class Sprite;
class Cell;


class Guide
{
private:
	Guide(Cell index,
		std::shared_ptr<Sprite> hoversprite, // (unique ownership)
		std::shared_ptr<Sprite> stampsprite, // (unique ownership)
		bool shine, bool pulse);

public:
	static Guide move(Cell index, const Move& move);
	static Guide slow(Cell index, const Move& move);
	static Guide postmove(Cell index, const Move& move);
	static Guide order(Cell index, const char* stampspritename);
	static Guide halt(Cell index);
	static Guide validtarget(Cell index);
	static Guide invalidtarget(Cell index);
	static Guide skiptarget(Cell index);
	static Guide frostbite(Cell index);
	static Guide firestorm(Cell index);
	static Guide bonedrought(Cell index);
	static Guide death(Cell index);
	static Guide gas(Cell index);
	static Guide radiation(Cell index);

	Guide();
	Guide(const Guide&) = delete;
	Guide(Guide&& that);
	Guide& operator=(const Guide&) = delete;
	Guide& operator=(Guide&&) = delete;
	~Guide();

	const Point _hoverorigin;
	const Point _stamporigin;
	std::shared_ptr<Sprite> _hoversprite; // (unique ownership)
	std::shared_ptr<Sprite> _stampsprite; // (unique ownership)
	bool _pulsecolors;

	static const char* spritenameMove(const Move& move);

public:
	void update();

	void setVisible(bool visible);
};
