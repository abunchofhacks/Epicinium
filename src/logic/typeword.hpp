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


struct TypeWord
{
	char data[16] = {};

	bool operator==(const TypeWord& other) const;
	bool operator!=(const TypeWord& other) const;
	bool operator==(const char* other) const;
	bool operator!=(const char* other) const;
};

TypeWord forceTypeWord(const char* str);

TypeWord parseTypeWord(const std::string& str);
const char* stringify(const TypeWord& word);

std::ostream& operator<<(std::ostream& os, const TypeWord& word);
