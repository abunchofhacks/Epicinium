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

#include <bitset>

#include "player.hpp"


class Vision
{
public:
	class Iterator
	{
	private:
		const Vision& _vision;
		size_t _offset;

		void settle()
		{
			while (_offset < PLAYER_SIZE && !_vision._data[_offset])
			{
				++_offset;
			}
		}

	public:
		Iterator(const Vision& vision, size_t offset) :
			_vision(vision),
			_offset(offset)
		{
			settle();
		}

		~Iterator() = default;
		Iterator(const Iterator&) = default;
		Iterator(Iterator&&) = default;
		Iterator& operator=(const Iterator&) = delete;
		Iterator& operator=(Iterator&&) = delete;

		Player operator*() const
		{
			return (Player) _offset;
		}

		Iterator& operator++()
		{
			++_offset;
			settle();
			return *this;
		}

		bool operator!=(const Iterator& other) const
		{
			return _offset != other._offset;
		}
	};

private:
	std::bitset<PLAYER_SIZE> _data;

public:
	static Vision none()
	{
		Vision vision;
		vision.add(Player::OBSERVER);
		return vision;
	}

	static Vision all(const std::vector<Player>& players)
	{
		Vision vision;
		for (const Player& player : players)
		{
			vision.add(player);
		}
		vision.add(Player::OBSERVER);
		vision.add(Player::BLIND);
		return vision;
	}

	static Vision only(const Player& player)
	{
		Vision vision;
		vision.add(player);
		vision.add(Player::OBSERVER);
		return vision;
	}

	Vision() = default;

	explicit Vision(const Json::Value& json);

	/**/ATTRIBUTE_WARN_UNUSED_RESULT/**/
	Vision plus(const Vision& other) const
	{
		Vision result;
		result._data = _data | other._data;
		return result;
	}

	/**/ATTRIBUTE_WARN_UNUSED_RESULT/**/
	Vision minus(const Vision& other) const
	{
		Vision result;
		result._data = _data & ~(other._data);
		return result;
	}

	void add(const Vision& other)
	{
		_data |= other._data;
	}

	void remove(const Vision& other)
	{
		_data &= ~(other._data);
	}

	void add(const Player& player)
	{
		_data[(size_t) player] = true;
	}

	void remove(const Player& player)
	{
		_data[(size_t) player] = false;
	}

	bool contains(const Player& player) const
	{
		return _data[(size_t) player];
	}

	bool empty() const
	{
		return _data.none();
	}

	bool operator==(const Vision& other) const
	{
		return _data == other._data;
	}

	bool operator!=(const Vision& other) const
	{
		return _data != other._data;
	}

	Iterator begin() const
	{
		return Iterator(*this, 0);
	}

	Iterator end() const
	{
		return Iterator(*this, PLAYER_SIZE);
	}
};

std::ostream& operator<<(std::ostream& os, const Vision& vision);
