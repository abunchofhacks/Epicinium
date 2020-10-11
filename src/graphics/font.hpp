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


enum class Font : uint8_t
{
	DEFAULT = 0,
	// Can have nameless variant values 1 through 255 returned by Graphics.
};

struct FontStyle
{
	int size;
	int outline;
	bool bold;
	bool italic;

	FontStyle(int sz) :
		FontStyle(sz, 0, false, false)
	{}

	FontStyle(int sz, int ol, bool b, bool i) :
		size(sz),
		outline(ol),
		bold(b),
		italic(i)
	{}

	bool operator==(const FontStyle& other)
	{
		return (size == other.size
			&& outline == other.outline
			&& bold == other.bold
			&& italic == other.italic);
	}

	bool operator!=(const FontStyle& other)
	{
		return (size != other.size
			|| outline != other.outline
			|| bold != other.bold
			|| italic != other.italic);
	}
};
