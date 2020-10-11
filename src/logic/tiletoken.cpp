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
#include "tiletoken.hpp"
#include "source.hpp"


TileToken::TileToken(const TypeNamer& namer, const Json::Value& json) :
	TileToken()
{
	if (!json["type"].isNull())
	{
		type = parseTileType(namer, json["type"].asString());
	}
	if (!json["owner"].isNull())
	{
		owner = parsePlayer(json["owner"].asString());
	}
	if (!json["stacks"].isNull())
	{
		stacks = json["stacks"].asInt();
	}
	if (!json["power"].isNull())
	{
		power = json["power"].asInt();
	}
}

std::ostream& operator<<(std::ostream& os, const TileToken& token)
{
	os << "{";
	bool empty = true;
	if (token.type != TileType::NONE)
	{
		if (empty) empty = false;
		else os << ",";
		os << "\"type\":\"" << token.type << "\"";
	}
	if (token.owner != Player::NONE)
	{
		if (empty) empty = false;
		else os << ",";
		os << "\"owner\":\"" << token.owner << "\"";
	}
	if (token.stacks != 0)
	{
		if (empty) empty = false;
		else os << ",";
		os << "\"stacks\":" << ((int) token.stacks);
	}
	if (token.power != 0)
	{
		if (empty) empty = false;
		else os << ",";
		os << "\"power\":" << ((int) token.power);
	}
	return os << "}";
}
