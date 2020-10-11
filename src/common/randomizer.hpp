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


template <class T>
class Randomizer
{
public:
	explicit Randomizer(size_t size = 0) :
		shuffle(false)
	{
		options.reserve(size);
	}

	void push(T t)
	{
		options.push_back(t);
		shuffle = true;
	}

	void select(size_t index)
	{
		if (index + 1 < options.size())
		{
			std::rotate(options.begin() + index,
				options.begin() + index + 1,
				options.end());
		}
		shuffle = false;
	}

	const T& inspect(size_t index) const
	{
		return options[index];
	}

	const T& peek() const
	{
		return options.back();
	}

	T pop()
	{
		if (shuffle)
		{
			std::random_shuffle(options.begin(), options.end());
			shuffle = false;
		}
		T t = options.back();
		options.pop_back();
		return t;
	}

	void clear()
	{
		options.clear();
	}

	size_t count() const
	{
		return options.size();
	}

	bool empty() const
	{
		return options.empty();
	}

	explicit operator bool() const
	{
		return !empty();
	}

private:
	bool shuffle;
	std::vector<T> options;
};
