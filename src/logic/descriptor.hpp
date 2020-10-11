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

#include "position.hpp"


struct Descriptor
{
public:
	enum class Type : uint8_t
	{
		NONE = 0,
		CELL,
		TILE,
		GROUND,
		AIR,
		BYPASS,
	};

	static constexpr size_t TYPE_SIZE = ((size_t) Type::BYPASS) + 1;

	static Descriptor cell(const Position& position)
	{
		return Descriptor(Type::CELL, position);
	}

	static Descriptor tile(const Position& position)
	{
		return Descriptor(Type::TILE, position);
	}

	static Descriptor ground(const Position& position)
	{
		return Descriptor(Type::GROUND, position);
	}

	static Descriptor air(const Position& position)
	{
		return Descriptor(Type::AIR, position);
	}

	static Descriptor bypass(const Position& position)
	{
		return Descriptor(Type::BYPASS, position);
	}

	Descriptor() : type(Type::NONE) {}

	Descriptor(Type type, const Position& position) :
		type(type),
		position(position)
	{}

	Type type;
	Position position;

	explicit operator bool() const
	{
		return (type != Type::NONE);
	}

	bool operator==(const Descriptor& other) const
	{
		return (type == other.type && position == other.position);
	}

	bool operator!=(const Descriptor& other) const
	{
		return !(*this == other);
	}

	bool operator<(const Descriptor& other) const
	{
		return std::tie(type, position) < std::tie(other.type, other.position);
	}

	static Descriptor::Type parseType(const std::string& str);
	static const char* stringify(const Descriptor::Type& str);

	explicit Descriptor(const Json::Value& json);

	friend std::ostream& operator<<(std::ostream& os, const Descriptor& desc);
	friend std::ostream& operator<<(std::ostream& os, const Descriptor::Type& type);
};
