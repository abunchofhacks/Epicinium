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

#include "unittype.hpp"
#include "player.hpp"

class TypeNamer;


struct UnitToken
{
	UnitType type;
	Player owner;
	int8_t stacks;

	UnitToken() :
		type(UnitType::NONE),
		owner(Player::NONE),
		stacks(0)
	{}

	UnitToken(const TypeNamer& namer, const Json::Value& json);

	bool operator==(const UnitToken& other) const = delete;

	bool operator!=(const UnitToken& other) const
	{
		return (type  != other.type
			|| owner  != other.owner
			|| stacks != other.stacks);
	}

	explicit operator bool() const
	{
		return (type != UnitType::NONE);
	}
};

class UnitTokenWithId : public UnitToken
{
public:
	using UnitToken::UnitToken;
	UnitTokenWithId() = default;

	UnitTokenWithId(UnitToken that) :
		UnitToken(that)
	{}

private:
	uint32_t _id = 0;

public:
	void assignId(uint32_t assigned)
	{
		_id = assigned;
	}

	void resetId()
	{
		_id = 0;
	}

	uint32_t id() const
	{
		return _id;
	}
};

std::ostream& operator<<(std::ostream& os, const UnitToken& token);
