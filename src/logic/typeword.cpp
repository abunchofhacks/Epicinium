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
#include "typeword.hpp"
#include "source.hpp"

#include "parseerror.hpp"


bool TypeWord::operator==(const TypeWord& other) const
{
	return (strcmp(data, other.data) == 0);
}

bool TypeWord::operator!=(const TypeWord& other) const
{
	return (strcmp(data, other.data) != 0);
}

bool TypeWord::operator==(const char* str) const
{
	return (strcmp(data, str) == 0);
}

bool TypeWord::operator!=(const char* str) const
{
	return (strcmp(data, str) != 0);
}

TypeWord forceTypeWord(const char* str)
{
	TypeWord word;

	DEBUG_ASSERT(strlen(str) + 1 <= sizeof(TypeWord));

	strcpy(word.data, str);

	return word;
}

TypeWord parseTypeWord(const std::string& str)
{
	TypeWord word;

	DEBUG_ASSERT(str.size() + 1 <= sizeof(TypeWord));
	if (str.size() + 1 > sizeof(TypeWord))
	{
		throw ParseError("Invalid typeword '" + str + "': too long");
	}

	std::copy(str.begin(), str.end(), word.data);

	return word;
}

const char* stringify(const TypeWord& word)
{
	return word.data;
}

std::ostream& operator<<(std::ostream& os, const TypeWord& word)
{
	return os << word.data;
}
