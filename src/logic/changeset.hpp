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

#include "change.hpp"
#include "vision.hpp"

enum class Player : uint8_t;
class TypeNamer;


class ChangeSet
{
private:
	std::vector<std::pair<Change, Vision>> _data;

public:
	ChangeSet() = default;

	ChangeSet(const TypeNamer& namer, const Json::Value& json);

	void push(const Change& change, const Vision& vision)
	{
		_data.emplace_back(std::make_pair(change, vision));
	}

	std::vector<Change> get(const Player& player) const
	{
		std::vector<Change> results;
		for (const auto& kv : _data)
		{
			if (kv.second.contains(player))
			{
				results.push_back(kv.first);
			}
		}
		return results;
	}

	friend std::ostream& operator<<(std::ostream& os,
		const ChangeSet& changeset);

	static bool equal(const TypeNamer& namer,
		const ChangeSet& a, const ChangeSet& b);

	bool any() const
	{
		return (!_data.empty());
	}

	explicit operator bool() const
	{
		return any();
	}
};
