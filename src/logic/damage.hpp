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

#include "descriptor.hpp"

struct UnitToken;
struct TileToken;
class Bible;
class Board;
class Cell;


class Damage
{
public:
	struct Shot
	{
		Shot(const Descriptor& desc, const UnitToken* token,
			int8_t fig, int8_t damage);

		Descriptor desc;
		const UnitToken* unit;
		int8_t figure;
		int8_t damage;
	};

	struct Body
	{
		Body(const Descriptor& desc, const UnitToken* token,
			int8_t fig, int8_t hp);
		Body(const Descriptor& desc, const TileToken* token,
			int8_t fig, int8_t hp,
			bool killable, bool depowerable);

		Descriptor desc;
		const UnitToken* unit;
		const TileToken* tile;
		int8_t figure;
		bool killable;
		bool depowerable;
		int8_t hitpoints;
		int8_t taken;
	};

	struct Hit
	{
		Hit(const Shot&, const Body& body, bool killing, bool depowering);

		const Shot& shot;
		const Body& body;
		bool killing;
		bool depowering;
	};

	Damage(const Bible& bible, const Board& board) :
		_bible(bible),
		_board(board)
	{}

private:
	const Bible& _bible;
	const Board& _board;
	std::vector<uint8_t> _bullets;
	std::vector<Shot> _shots;
	std::vector<uint8_t> _targets;
	std::vector<Body> _bodies;
	std::vector<Hit> _hits;

	int hitpoints(Cell index, const TileToken& tile);
	int hitpoints(Cell index, const UnitToken& unit);

public:
	void addAttacker(const Descriptor& desc, const UnitToken& unit);
	void addTrampler(const Descriptor& desc, const UnitToken& unit);
	void addCaster(const Descriptor& desc, const UnitToken& unit);
	void addEffect(const Descriptor& desc, int8_t shots, int8_t damage);
	void addTaker(const Descriptor& desc, const UnitToken& unit);
	void addTaker(const Descriptor& desc, const TileToken& tile);
	void addBackground(const Descriptor& desc, const UnitToken& unit);
	void addBackground(const Descriptor& desc, const TileToken& tile);
	void addTrenches(const Descriptor& desc, const TileToken& tile);
	void addClouds(const Descriptor& desc);

	size_t shots() const;
	size_t bodies() const;
	void resolve();

	std::vector<Hit>::const_iterator begin() const
	{
		return _hits.begin();
	}

	std::vector<Hit>::const_iterator end() const
	{
		return _hits.end();
	}
};
