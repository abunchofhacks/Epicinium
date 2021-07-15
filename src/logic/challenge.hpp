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

enum class Player : uint8_t;
enum class Notice : uint8_t;
class Bible;
class Board;
class PlayerInfo;
class RoundInfo;


class Challenge
{
public:
	enum Id : uint16_t
	{
		CUSTOM,
		SHOWCASE,
		ELIMINATION,
		EVERYTHINGISFREE,
		TRAMPLE,
		HIGHSPEED,
		INVESTMENT,
		MORALE,
		ACT1A, ACT1B, ACT1C, ACT1D, ACT1E, ACT1F,
		ACT2A, ACT2B, ACT2C, ACT2D, ACT2E,
	};

	static constexpr size_t ID_SIZE = ((size_t) Id::MORALE) + 1;

	Challenge(Id id) :
		_id(id)
	{}

	static const std::vector<Id>& pool();
	static const std::vector<Id>& campaign();

protected:
	const Id _id;

public:
	Notice check(const Bible&, const Board& board,
		const RoundInfo& rinfo, PlayerInfo& pinfo,
		std::vector<Player>& defeats);

	void score(const Bible& bible, const Board& board,
		PlayerInfo& info);
	void award(const Bible& bible, const Board& board,
		PlayerInfo& info);

	static Notice check(const Id& id, const Bible&, const Board& board,
		const RoundInfo& rinfo, PlayerInfo& pinfo,
		std::vector<Player>& defeats);

	static void score(const Id& id, const Bible& bible, const Board& board,
		PlayerInfo& info);
	static void award(const Id& id, const Bible& bible, const Board& board,
		PlayerInfo& info);
};
