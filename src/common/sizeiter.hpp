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

class SizeIter
{
public:
	typedef SizeIter self_type;
	typedef size_t value_type;
	typedef size_t& reference;
	typedef size_t* pointer;
	typedef std::forward_iterator_tag iterator_category;
	typedef size_t difference_type;

	explicit SizeIter(size_t i) : _i(i) {}

private:
	size_t _i;

public:
	SizeIter& operator++() { ++_i; return *this; }
	SizeIter operator++(int /**/) { return SizeIter(_i++); }
	size_t operator*() const { return _i; }
	const size_t* operator->() const { return &_i; }
	friend void swap(SizeIter& a, SizeIter& b) { std::swap(a._i, b._i); }
	friend bool operator==(const SizeIter& a, const SizeIter& b) { return a._i == b._i; }
	friend bool operator!=(const SizeIter& a, const SizeIter& b) { return a._i != b._i; }
};
