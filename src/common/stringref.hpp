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

#include <cstring>


struct stringref
{
private:
	const char* const _data;
	const size_t _size;

public:
	template <size_t N>
	constexpr stringref(const char (&lit)[N]) :
		_data(lit),
		_size(N)
	{}

	explicit stringref(const char* raw) :
		_data(raw),
		_size(strlen(raw) + 1)
	{}

	stringref(const std::string& str) :
		_data(str.c_str()),
		_size(str.size() + 1)
	{}

	stringref(const stringref&) = default;
	stringref(stringref&&) = default;
	stringref& operator=(const stringref&) = delete;
	stringref& operator=(stringref&&) = delete;
	~stringref() = default;

	friend bool operator==(const stringref& a, const stringref& b)
	{
		return (a._size == b._size && strcmp(a._data, b._data) == 0);
	}

	friend bool operator!=(const stringref& a, const stringref& b)
	{
		return (a._size != b._size || strcmp(a._data, b._data) != 0);
	}

	friend bool strncmp(stringref a, stringref b, size_t n)
	{
		return strncmp(a._data, b._data, n);
	}

	std::string str() const
	{
		return std::string(_data);
	}

	friend std::ostream& operator<<(std::ostream& os, stringref str);
};
